#include <stdio.h>
#include <termios.h>
#include <unistd.h>	//STDIN_FILENO 
#include <stdlib.h>     //calloc, free
#include <sys/ioctl.h>      // ioctl, TIOCGWINSZ
#include <signal.h>	// SIGWINCH, sig_atomic_t
#include <string.h>	// memset
#include <stdint.h> 
#include "pocket.h"
#include "pkt_terminal_internal.h"

struct pkt_cell { // 6 bytes
	uint32_t ch;
	uint8_t fcolor;
	uint8_t bcolor;
};

static void __pkt_terminal_clear(void);
static void __pkt_terminal_hidecurs(void);
static void __pkt_terminal_showcurs(void);
static void __pkt_terminal_mvcurs(int x, int y);
static void __pkt_terminal_set_color(int fcolor, int bcolor);
static int __pkt_terminal_init_buffers(void);
static void __pkt_terminal_start_altbuff(void);
static void __pkt_terminal_stop_altbuff(void);
static void __pkt_terminal_load_config(struct pkt_config *config);
static void __pkt_terminal_resize(int row, int col);
static void __pkt_terminal_flag_sigwinch(int sig);
static void __pkt_terminal_handle_sigwinch(void);
static int __pkt_terminal_pack_utf8(const char *str, uint32_t *out_char);
static void __pkt_terminal_unpack_utf8(char *buf, uint32_t ch);

static struct termios original_term;
static volatile sig_atomic_t is_window_resized = 0; // Force compiler to see this variable

static struct pkt_cell *front_buffer;
static struct pkt_cell *back_buffer;

static int original_col = 0;
static int original_row = 0;
static int col = PKT_DEFAULT_SCOL;
static int row = PKT_DEFAULT_SROW;

int __pkt_terminal_init(struct pkt_config *config) 
{
	__pkt_terminal_load_config(config);	
	
	// Get original window size to restore later
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
		original_row = w.ws_row;
		original_col = w.ws_col;
	}

	tcgetattr(STDIN_FILENO, &original_term); // store user terminal attribute to restore later.
	struct termios new_term = original_term;
	new_term.c_lflag &= ~(ECHO | ICANON); // Disable echo and ICANON(waiting untill user hit enter)
	new_term.c_cc[VMIN] = 0; // wait at least 0 characters
	new_term.c_cc[VTIME] = 0; // get realtime input 
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term); // Set terminal attribute immediately(TCSANOW)
	
	signal(SIGWINCH, __pkt_terminal_flag_sigwinch); // Register window resize(by game player) signal handler to OS

	__pkt_terminal_start_altbuff();
	__pkt_terminal_hidecurs();
	__pkt_terminal_clear();
	__pkt_terminal_resize(row, col);
	fflush(stdout);
	
	if (__pkt_terminal_init_buffers() < 0)
		return -1;

	return 0;
}

// Recover terminal attribute before app stareted. 
// Move cursor to top-left, show cursor, and clear screen
int __pkt_terminal_restore(void) 
{
	__pkt_terminal_clear();
	__pkt_terminal_showcurs();
	__pkt_terminal_mvcurs(0, 0);
	__pkt_terminal_stop_altbuff();
	__pkt_terminal_resize(original_row, original_col);

	tcsetattr(STDIN_FILENO, TCSANOW, &original_term); // Restore original attribute 
	fflush(stdout);

	free(front_buffer);
	free(back_buffer);

	return 0;
}

int __pkt_terminal_getch(void) 
{
	unsigned char c;
	// order OS to read 1 byte directly from file descriptor
	if (read(STDIN_FILENO, &c, 1) == 1)
		return c; // Key has pressed 

	return -1; // Key has not pressed 
}

int __pkt_terminal_update(void)
{
	__pkt_terminal_handle_sigwinch();

	for (int y = 0; y < row; y++) {
		for (int x = 0; x < col; x++){
			struct pkt_cell bcell = back_buffer[y * col + x];
			struct pkt_cell fcell = front_buffer[y * col + x];
			
			// if ch is 1, it means its fullwidth character's last half
			if (bcell.ch == 1) {
				front_buffer[y * col + x] = bcell;

				back_buffer[y * col + x].ch = ' ';
				back_buffer[y * col + x].fcolor = PKT_COLOR_DEFAULT;
				back_buffer[y * col + x].bcolor = PKT_COLOR_DEFAULT;
				continue;
			}

			if (bcell.ch != fcell.ch || bcell.fcolor != fcell.fcolor || bcell.bcolor != fcell.bcolor) {
				__pkt_terminal_mvcurs(x, y);
				__pkt_terminal_set_color(bcell.fcolor, bcell.bcolor);
				
				if (bcell.ch == 0 || bcell.ch == ' ') {
					printf(" ");
				} else {
					char buf[5] = {0};
					__pkt_terminal_unpack_utf8(buf, bcell.ch);
					printf("%s", buf);	
				}
				front_buffer[y * col + x] = bcell;
			}	

			back_buffer[y * col + x].ch = ' ';
			back_buffer[y * col + x].fcolor = PKT_COLOR_DEFAULT;
			back_buffer[y * col + x].bcolor = PKT_COLOR_DEFAULT;
		}
	}

	fflush(stdout);
	return 0;
}

int __pkt_terminal_putch(int x, int y, int fcolor, int bcolor, char c)
{
	if (x < 0 || x >= col || y < 0 || y >= row)
		return -1;
	back_buffer[y * col + x].ch = c;
	back_buffer[y * col + x].fcolor = fcolor;
	back_buffer[y * col + x].bcolor = bcolor;
	return 0;
}

int __pkt_terminal_putstr(int x, int y, int fcolor, int bcolor, const char *str)
{
	if (y < 0 || y >= row)
		return -1;
	int i = 0;
	int current_x = x;

	while (str[i] != '\0') {
		if (current_x < 0 || current_x >= col)
			break;

		uint32_t packed_ch = 0;
		int bytes = __pkt_terminal_pack_utf8(&str[i], &packed_ch);

		int width = (bytes >= 3) ? 2 : 1; // fullwidth character

		back_buffer[y * col + current_x].ch = packed_ch;
		back_buffer[y * col + current_x].fcolor = fcolor;
		back_buffer[y * col + current_x].bcolor = bcolor;
		
		// last half of fullwidth character. set 1 in ch as a mark so thay terminal_update() knows 
		if (width == 2 && current_x + 1 < col) {
			back_buffer[y * col + current_x + 1].ch = 1;
			back_buffer[y * col + current_x + 1].fcolor = fcolor;
			back_buffer[y * col + current_x + 1].bcolor = bcolor;
		}

		current_x += width;
		i += bytes;
	}

	return 0;
}

// Read bytes of UTF-8, pack the character in 32bits box(out_char)
// This returns the bytes of the character read
// [REF] https://ja.wikipedia.org/wiki/UTF-8
static int __pkt_terminal_pack_utf8(const char *str, uint32_t *out_char)
{
	unsigned char c1 = (unsigned char)str[0];
	if (c1 == '\0')
		return 0;
	
	if ((c1 & 0x80) == 0x00) { // 1 byte (ASCII)
		*out_char = (uint32_t)(unsigned char)c1;
		return 1; 
	} else if ((c1 & 0xE0) == 0xC0) {
		*out_char = ((uint32_t)(unsigned char)c1 << 8) | (uint32_t)(unsigned char)str[1];
		return 2;
	} else if ((c1 & 0xF0) == 0xE0) {
		*out_char = ((uint32_t)(unsigned char)c1 << 16) | ((uint32_t)(unsigned char)str[1] << 8) 
			| (uint32_t)(unsigned char)str[2];
		return 3;
	} else if ((c1 & 0xF8) == 0xF0) {
		*out_char = ((uint32_t)(unsigned char)c1 << 24) | ((uint32_t)(unsigned char)str[1] << 16) 
			| ((uint32_t)(unsigned char)str[2] << 8) | (uint32_t)(unsigned char)str[3];
		return 4;
	}

	*out_char = (uint32_t)c1;
	return 1;
}

// [Warning] Must pass 5 bytes or bigger buffer must be passed utf8(1-4 bytes) + \0
static void __pkt_terminal_unpack_utf8(char *buf, uint32_t ch)
{
	int i = 0;
	if (ch >= 0x1000000) { // boundary from 3 bytes to 4 bytes
		buf[i] = (ch >> 24) & 0xFF;
		i++;	
	}

	if (ch >= 0x10000) {
		buf[i] = (ch >> 16) & 0xFF;
		i++;
	}

	if (ch >= 0x100) {
		buf[i] = (ch >> 8) & 0xFF;
		i++;
	}

	buf[i] = ch & 0xFF;
	buf[i + 1] = '\0';
}

static void __pkt_terminal_clear(void) 
{
	printf("\x1b[2J"); // ANSI Escape sequence to clear screen 
}

static void __pkt_terminal_hidecurs(void) 
{
	printf("\x1b[?25l"); // ANSI escape sequence to hide cursor
}

static void __pkt_terminal_showcurs(void) 
{
	printf("\x1b[?25h"); // ANSI escape sequence to show cursor
}

// Move cursor using ANSI escape sequence (ANSI coordinates starts with 1)
static void __pkt_terminal_mvcurs(int x, int y) 
{
	printf("\x1b[%d;%dH", y + 1, x + 1); 
}

static void __pkt_terminal_start_altbuff(void)
{
	printf("\x1b[?1049h");
}

static void __pkt_terminal_stop_altbuff(void)
{
	printf("\x1b[?1049l");
}

static void __pkt_terminal_set_color(int fcolor, int bcolor)  
{
	if (fcolor == PKT_COLOR_DEFAULT)
		fcolor = 39;

	if (bcolor == PKT_COLOR_DEFAULT)
		bcolor = 49;
	else
		bcolor += 10; // fcolor + 10 is background color in ANSI
	
	printf("\x1b[%d;%dm", fcolor, bcolor); 
}

static int __pkt_terminal_init_buffers(void)
{
	front_buffer = (struct pkt_cell *)calloc((size_t)(col * row), sizeof(struct pkt_cell));
	if (!front_buffer)
		return -1;
	back_buffer = (struct pkt_cell *)calloc((size_t)(col * row), sizeof(struct pkt_cell));
	if (!back_buffer) {
		free(front_buffer);
		return -1;
	}

	return 0;
}

static void __pkt_terminal_load_config(struct pkt_config *c)
{
	if (c->screen_col < 1 || c->screen_col > 500)
		col = PKT_DEFAULT_SCOL;
	else
		col = c->screen_col;

	if (c->screen_row < 1 || c->screen_row > 500)
		row = PKT_DEFAULT_SROW;
	else
		row = c->screen_row;
}

static void __pkt_terminal_resize(int row, int col)
{
	printf("\x1b[8;%d;%dt", row, col);
}

// Just flag if window resized by OS
static void __pkt_terminal_flag_sigwinch(int sig)
{
	(void)sig;
	is_window_resized = 1;
}

// Bring back the terminal size to configured game size. Clear screen, then zero clear front buffer
// so that every cells will be re-drawn in next frame.
static void __pkt_terminal_handle_sigwinch(void)
{
	if (is_window_resized) {
		is_window_resized = 0;

		__pkt_terminal_resize(row, col);
		__pkt_terminal_clear();
		memset(front_buffer, 0, sizeof(struct pkt_cell) * col * row);

	}
}
