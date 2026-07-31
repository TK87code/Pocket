#include <stdio.h>
#include <termios.h>
#include <unistd.h>	//STDIN_FILENO 
#include <stdlib.h>     //calloc, free
#include <sys/ioctl.h>      // ioctl, TIOCGWINSZ
#include <signal.h>	// SIGWINCH, sig_atomic_t
#include <string.h>	// memset
#include <stdint.h> 
#include <stdarg.h> // va_list, va_start, vfprintf, va_end
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
static void __pkt_terminal_set_color(enum pkt_color fcolor, enum pkt_color bcolor);
static int __pkt_terminal_init_buffers(void);
static void __pkt_terminal_start_altbuff(void);
static void __pkt_terminal_stop_altbuff(void);
static void __pkt_terminal_load_config(struct pkt_config *config);
static void __pkt_terminal_resize(int row, int col);
static void __pkt_terminal_flag_sigwinch(int sig);
static void __pkt_terminal_handle_sigwinch(void);
static int __pkt_terminal_pack_utf8(const char *str, uint32_t *out_char);
static int __pkt_terminal_unpack_utf8(char *buf, uint32_t ch, size_t buf_size);

static struct termios original_term;
static volatile sig_atomic_t is_window_resized = 0; // Force compiler to see this variable
static int has_resized_ever = 0;

static struct pkt_cell *front_buffer;
static struct pkt_cell *back_buffer;

static int original_col = 0;
static int original_row = 0;
static int col = PKT_DEFAULT_SCOL;
static int row = PKT_DEFAULT_SROW;
static enum pkt_color user_default_fcolor = PKT_COLOR_WHITE;
static enum pkt_color user_default_bcolor = PKT_COLOR_BLACK;

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
// Stop alternative buffer, show cursor, restore the original terminal size and color
// If window resized ever, place cursor all the way down so that it correctly restore the looks.
int __pkt_terminal_restore(void) 
{
	__pkt_terminal_resize(original_row, original_col);
	__pkt_terminal_stop_altbuff();
	__pkt_terminal_showcurs();
	fputs("\x1b[0m", stdout);
	if (has_resized_ever == 1)	
		fputs("\x1b[999;1H", stdout);
	fflush(stdout);

	tcsetattr(STDIN_FILENO, TCSANOW, &original_term); // Restore original attribute 

	free(front_buffer);
	free(back_buffer);

	return 0;
}

/*
 * Order OS to read 1 byte directly from file descriptor. If first byte was 'ESC',
 * read 2 bytes followed, and detect special characters.Also this function detect enter key which
 * differs among OSs.
 * Return the character if input is detected, otherwise -1.
 */
int __pkt_terminal_read_input(void) 
{
	unsigned char c;
	// order OS to read 1 byte directly from file descriptor
	if (read(STDIN_FILENO, &c, 1) == 1) {
		if (c == '\x1b') {
			unsigned char seq[2];

			if (read(STDIN_FILENO, &seq[0], 1) != 1)
				return PKT_KEY_ESCAPE; 
			if (read(STDIN_FILENO, &seq[1], 1) != 1)
				return PKT_KEY_ESCAPE;

			if (seq[0] == '[') {
				switch (seq[1]) {
				case 'A': 
					return PKT_KEY_UP;
				case 'B':
					return PKT_KEY_DOWN;
				case 'C':
					return PKT_KEY_RIGHT;
				case 'D':
					return PKT_KEY_LEFT;
				}
			}

			return PKT_KEY_ESCAPE;
		}	
		
		if (c == '\r' || c == '\n') {
			return PKT_KEY_ENTER;
		}

		return c;  
	}

	return -1;  
}

/* Compare back buffer and front buffer, then update screen only if there is some changes between the buffers.
* set_color() function will be calld only when current terminal color setting is different from the color that
* is about to be drawn.
*/ 
int __pkt_terminal_update(void)
{
	__pkt_terminal_handle_sigwinch();

	int fcpen = -1;
	int bcpen = -1;

	for (int y = 0; y < row; y++) {
		for (int x = 0; x < col; x++){
			struct pkt_cell bcell = back_buffer[y * col + x];
			struct pkt_cell fcell = front_buffer[y * col + x];
			
			// if ch is 1, it means its fullwidth character's last half
			if (bcell.ch == 1) {
				front_buffer[y * col + x] = bcell;

				back_buffer[y * col + x].ch = ' ';
				back_buffer[y * col + x].fcolor = user_default_fcolor;
				back_buffer[y * col + x].bcolor = user_default_bcolor;
				continue;
			}

			if (bcell.ch != fcell.ch || bcell.fcolor != fcell.fcolor || bcell.bcolor != fcell.bcolor) {
				__pkt_terminal_mvcurs(x, y);
				if (fcpen != bcell.fcolor || bcpen != bcell.bcolor) {
					__pkt_terminal_set_color(bcell.fcolor, bcell.bcolor);
					fcpen = bcell.fcolor;
					bcpen = bcell.bcolor;
				}
				
				if (bcell.ch == 0 || bcell.ch == ' ') {
					fputc(' ', stdout);	
				} else if (bcell.ch < 0x80) { // 1 byte character 
					fputc((char)bcell.ch, stdout);			
				} else {
					char buf[5] = {0};
					__pkt_terminal_unpack_utf8(buf, bcell.ch, sizeof(buf));
					fputs(buf, stdout);
				}
				front_buffer[y * col + x] = bcell;
			}	

			back_buffer[y * col + x].ch = ' ';
			back_buffer[y * col + x].fcolor = user_default_fcolor;
			back_buffer[y * col + x].bcolor = user_default_bcolor;
		}
	}

	fflush(stdout);
	return 0;
}

int __pkt_terminal_putc(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, char c)
{
	if (x < 0 || x >= col || y < 0 || y >= row)
		return -1;
	back_buffer[y * col + x].ch = c;
	back_buffer[y * col + x].fcolor = fcolor;
	back_buffer[y * col + x].bcolor = bcolor;
	return 0;
}

int __pkt_terminal_puts(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, const char *str)
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
static int __pkt_terminal_unpack_utf8(char *buf, uint32_t ch, size_t buf_size)
{
	if (buf_size < 5)
		return -1;
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

	return 0;
}

static void __pkt_terminal_clear(void) 
{
	fputs("\x1b[2J", stdout); // ANSI Escape sequence to clear screen 
}

static void __pkt_terminal_hidecurs(void) 
{
	fputs("\x1b[?25l", stdout); // ANSI escape sequence to hide cursor
}

static void __pkt_terminal_showcurs(void) 
{
	fputs("\x1b[?25h", stdout); // ANSI escape sequence to show cursor
}

// Move cursor using ANSI escape sequence (ANSI coordinates starts with 1)
static void __pkt_terminal_mvcurs(int x, int y) 
{
	printf("\x1b[%d;%dH", y + 1, x + 1); 
}

static void __pkt_terminal_start_altbuff(void)
{
	fputs("\x1b[?1049h", stdout);
}

static void __pkt_terminal_stop_altbuff(void)
{
	fputs("\x1b[?1049l", stdout);
}

// Set font and background color. bcolor is fcolor + 10 in ANSI escape sequence.
static void __pkt_terminal_set_color(enum pkt_color fcolor, enum pkt_color bcolor)  
{
	printf("\x1b[%d;%dm", fcolor, bcolor + 10); 
}

static void __pkt_terminal_resize(int row, int col)
{
	printf("\x1b[8;%d;%dt", row, col);
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

	if (c->default_fcolor < 30 || c->default_fcolor > 37)
		user_default_fcolor = PKT_COLOR_WHITE;
	else
		user_default_fcolor = c->default_fcolor;

	if (c->default_bcolor < 30 || c->default_bcolor > 37)
		user_default_bcolor = PKT_COLOR_BLACK;
	else
		user_default_bcolor = c->default_bcolor;
}

// Just flag if window resized by OS
static void __pkt_terminal_flag_sigwinch(int sig)
{
	(void)sig;
	is_window_resized = 1;
	has_resized_ever = 1;
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
