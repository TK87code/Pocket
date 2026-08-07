#define _XOPEN_SOURCE 500 	// X/Open System Interfaces Extension (XSI) Issue 5
#include <stdio.h>		// puts, putc(
#include <termios.h>		// tcgetattr, struct terminos
#include <unistd.h>		// STDIN_FILENO 
#include <stdlib.h>     	// calloc, free, mbtowc
#include <sys/ioctl.h>      	// ioctl, TIOCGWINSZ
#include <signal.h>		// sig_atomic_t, struct sigaction
#include <stdint.h>		// uintxx_t
#include <stdarg.h> 		// va_list, va_start, vfprintf, va_end
#include <locale.h>		// setlocale
#include <wchar.h>		// mbtowc, wchar_t
#include "pocket.h"
#include "pkt_terminal_internal.h"

#define PKT_CELL_FLAG_ORIGINAL_COLOR 1

struct pkt_cell { // 8 bytes
	uint32_t ch;
	uint8_t fcolor;
	uint8_t bcolor;
	uint8_t attr;
	uint8_t flags;
};

static int __pkt_terminal_setup_sig(void);
static void __pkt_terminal_hidecurs(void);
static void __pkt_terminal_showcurs(void);
static void __pkt_terminal_mvcurs(int x, int y);
static void __pkt_terminal_set_color(int fcolor, int bcolor);
static void __pkt_terminal_set_attr(unsigned int attr);
static void __pkt_terminal_clear_attr(void);
static int __pkt_terminal_init_buffers(void);
static void __pkt_terminal_start_altbuff(void);
static void __pkt_terminal_stop_altbuff(void);
static void __pkt_terminal_load_config(struct pkt_config *config);
static void __pkt_terminal_flag_sigwinch(int sig);
static void __pkt_terminal_flag_sigint(int sig);
static int __pkt_terminal_set_termsize(int *out_cols, int *out_rows);
static int __pkt_terminal_pack_utf8(const char *str, uint32_t *out_char);
static int __pkt_terminal_unpack_utf8(char *buf, uint32_t ch, size_t buf_size);
static void __pkt_terminal_write_cell(int x, int y, unsigned int fcolor, unsigned int bcolor, unsigned int attr, 
		unsigned int flag, uint32_t ch);

static struct termios original_term;

static struct pkt_cell *front_buffer;
static struct pkt_cell *back_buffer;

static volatile sig_atomic_t pending_resize_event = 0;
static volatile sig_atomic_t pending_quit_event = 0;

// Global variables to store game player's current terminal sizes
static int terminal_rows = 0;
static int terminal_cols = 0;

static int game_rows = 0;
static int game_cols = 0;

static enum pkt_color user_default_fcolor = PKT_COLOR_WHITE;
static enum pkt_color user_default_bcolor = PKT_COLOR_BLACK;

/*
 * Load config to set data needed. Get terminal size.
 * Store terminal attributes to restore later, and 
 * modify some attributes and set it to the terminal immediately.
 * Register a callback when user resize the terminal so that engine can issue the event.
 * Start alternate buffer and hide cursor. Then initialize screen buffers.
 */
int __pkt_terminal_init(struct pkt_config *config) 
{
	__pkt_terminal_load_config(config);	

	setlocale(LC_ALL, "");

	__pkt_terminal_set_termsize(&terminal_cols, &terminal_rows);

	tcgetattr(STDIN_FILENO, &original_term); 
	struct termios new_term = original_term;
	new_term.c_lflag &= ~(ECHO | ICANON); 
	new_term.c_cc[VMIN] = 0; 
	new_term.c_cc[VTIME] = 0;  
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term); 
	
	__pkt_terminal_setup_sig();
	__pkt_terminal_start_altbuff();
	__pkt_terminal_hidecurs();
	fflush(stdout);
	
	if (__pkt_terminal_init_buffers() < 0)
		return -1;

	return 0;
}

/* 
 * Recover terminal attribute before app stareted. 
 * Stop alternative buffer, show cursor, restore the original terminal size and color
 * If window resized ever, place cursor all the way down so that it correctly restore the looks.
 */
int __pkt_terminal_restore(void) 
{
	__pkt_terminal_stop_altbuff();
	__pkt_terminal_showcurs();
	__pkt_terminal_clear_attr();
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
		
		if (c == '\r' || c == '\n') 
			return PKT_KEY_ENTER;

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
	int fcpen = -1;
	int bcpen = -1;
	int attrpen = -1;

	for (int y = 0; y < terminal_rows; y++) {
		for (int x = 0; x < terminal_cols; x++){
			struct pkt_cell bcell = back_buffer[y * terminal_cols + x];
			struct pkt_cell fcell = front_buffer[y * terminal_cols + x];

			int is_logical = (game_cols <= 0 && game_rows <= 0) || (x < game_cols && y < game_rows);
			unsigned int reset_fc = is_logical ? (unsigned int)user_default_fcolor : 0;
			unsigned int reset_bc = is_logical ? (unsigned int)user_default_bcolor : 0;
			unsigned int reset_flag = is_logical ? 0 : PKT_CELL_FLAG_ORIGINAL_COLOR;
			
			// if ch is 1, it means its fullwidth character's last half
			if (bcell.ch == 1) {
				front_buffer[y * terminal_cols + x] = bcell;
				__pkt_terminal_write_cell(x, y, reset_fc, reset_bc, 
						PKT_ATTR_NONE, reset_flag, ' ');
				continue;
			}

			if (bcell.ch != fcell.ch || bcell.fcolor != fcell.fcolor || bcell.bcolor != fcell.bcolor 
					|| bcell.attr != fcell.attr || bcell.flags != fcell.flags) {
				__pkt_terminal_mvcurs(x, y);

				int target_fc = (bcell.flags & PKT_CELL_FLAG_ORIGINAL_COLOR) ? -2 : bcell.fcolor; 
				int target_bc = (bcell.flags & PKT_CELL_FLAG_ORIGINAL_COLOR) ? -2 : bcell.bcolor; 

				if (fcpen != target_fc || bcpen != target_bc || attrpen != bcell.attr) {
					__pkt_terminal_clear_attr();
					__pkt_terminal_set_color(target_fc, target_bc);
					__pkt_terminal_set_attr(bcell.attr);
					fcpen = target_fc;
					bcpen = target_bc;
					attrpen = bcell.attr;
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
				front_buffer[y * terminal_cols + x] = bcell;
			}	
			
			__pkt_terminal_write_cell(x, y, reset_fc, reset_bc, PKT_ATTR_NONE, reset_flag,' ');
		}
	}

	fflush(stdout);
	return 0;
}

int __pkt_terminal_putc(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, char c)
{
	int max_cols = (game_cols > 0 && game_cols < terminal_cols) ? game_cols : terminal_cols;
	int max_rows = (game_rows > 0 && game_rows < terminal_rows) ? game_rows : terminal_rows;
	
	if (x < 0 || x >= max_cols || y < 0 || y >= max_rows)
		return -1;

	unsigned int fc = (fcolor == PKT_COLOR_DEFAULT) ? (unsigned int)user_default_fcolor : (unsigned int)fcolor;
	unsigned int bc = (bcolor == PKT_COLOR_DEFAULT) ? (unsigned int)user_default_bcolor : (unsigned int)bcolor;

	__pkt_terminal_write_cell(x, y, fc, bc, attr, 0, c);
	return 0;
}

int __pkt_terminal_puts(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *str)
{
	int max_cols = (game_cols > 0 && game_cols < terminal_cols) ? game_cols : terminal_cols;
	int max_rows = (game_rows > 0 && game_rows < terminal_rows) ? game_rows : terminal_rows;

	if (y < 0 || y >= max_rows)
		return -1;

	unsigned int fc = (fcolor == PKT_COLOR_DEFAULT) ? (unsigned int)user_default_fcolor : (unsigned int)fcolor;
	unsigned int bc = (bcolor == PKT_COLOR_DEFAULT) ? (unsigned int)user_default_bcolor : (unsigned int)bcolor;

	int i = 0;
	int current_x = x;

	while (str[i] != '\0') {
		if (current_x < 0 || current_x >= max_cols)
			break;

		uint32_t packed_ch = 0;
		int bytes = __pkt_terminal_pack_utf8(&str[i], &packed_ch);

		wchar_t wc = 0;
		mbtowc(&wc, &str[i], bytes);
		int width = wcwidth(wc);

		__pkt_terminal_write_cell(current_x, y, fc, bc, attr, 0, packed_ch);
		// last half of fullwidth character. set 1 in ch as a mark so thay terminal_update() knows 
		if (width == 2 && current_x + 1 < max_cols) {
			__pkt_terminal_write_cell(current_x + 1, y, fc, bc, attr, 0, 1);
		}

		current_x += width;
		i += bytes;
	}
	return 0;
}

int __pkt_terminal_check_resize(int *out_cols, int *out_rows)
{
	if (pending_resize_event) {
		pending_resize_event = 0;
		
		int new_cols = 0;
		int new_rows = 0;
		__pkt_terminal_set_termsize(&new_cols, &new_rows);
		
		if (new_cols != terminal_cols || new_rows != terminal_rows) {
			terminal_cols = new_cols;
			terminal_rows = new_rows;

			__pkt_terminal_init_buffers();

			fputs("\x1b[0m\x1b[2J", stdout); // reset color and attr, then clear screen
			fflush(stdout);

			*out_cols = terminal_cols;
			*out_rows = terminal_rows;

			return 1;
		}
	}
	return 0;
}

int __pkt_terminal_check_quit(void)
{
	if (pending_quit_event) {
		pending_quit_event = 0;
		__pkt_terminal_restore();
		return 1;
	}

	return 0;
}

int __pkt_terminal_get_termsize(int *out_cols, int *out_rows)
{
	if (out_cols)
		*out_cols = terminal_cols;
	if (out_rows)
		*out_rows = terminal_rows;
	return 0;
}

/*
 * Setup resize signal and force quit(ctrl + c) signal.
 * Each signal handlers just flag that events occur.
 * Main loop check if there is a pending event, then each checking function do their job.
 */
static int __pkt_terminal_setup_sig(void)
{
	struct sigaction sw;
	sw.sa_handler = __pkt_terminal_flag_sigwinch;
	sigemptyset(&sw.sa_mask);
	sw.sa_flags = SA_RESTART;

	if (sigaction(SIGWINCH, &sw, NULL) < 0) {
		return -1;
	}

	struct sigaction si;
	si.sa_handler = __pkt_terminal_flag_sigint;
	sigemptyset(&si.sa_mask);
	si.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &si, NULL) < 0) {
		return -2;
	}

	return 0;
}

static int __pkt_terminal_set_termsize(int *out_cols, int *out_rows) 
{
	if (!out_cols || !out_rows)
		return -1;

	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
		*out_cols = w.ws_col;
		*out_rows = w.ws_row; 
	}

	return 0;
}

/*
 * Read bytes of UTF-8, pack the character in 32bits box(out_char)
 * This returns the bytes of the character read
 * [REF] https://ja.wikipedia.org/wiki/UTF-8
 */
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

static void __pkt_terminal_hidecurs(void) 
{
	fputs("\x1b[?25l", stdout); 
}

static void __pkt_terminal_showcurs(void) 
{
	fputs("\x1b[?25h", stdout); 
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
// If fcolor or bcolor is -2, reset fcolor and bcolor to terminal original.
static void __pkt_terminal_set_color(int fcolor, int bcolor)  
{
	if (fcolor == -2 || bcolor == -2) 
		fputs("\x1b[39;49m", stdout);
	else
		printf("\x1b[38;5;%d;48;5;%dm", (uint8_t)fcolor, (uint8_t)bcolor); 
}

static void __pkt_terminal_set_attr(unsigned int attr)
{
	if (attr & PKT_ATTR_BOLD) {
		fputs("\x1b[1m", stdout);
	}

	if (attr & PKT_ATTR_UNDERLINE) {
		fputs("\x1b[4m", stdout);
	}

	if (attr & PKT_ATTR_REVERSE) {
		fputs("\x1b[7m", stdout);
	}

	if (attr & PKT_ATTR_BLINK) {
		fputs("\x1b[5m", stdout);
	}

	if (attr & PKT_ATTR_FASTBLINK) {
		fputs("\x1b[6m", stdout);
	}
}

static void __pkt_terminal_clear_attr()
{
	fputs("\x1b[0m", stdout);
}

static int __pkt_terminal_init_buffers(void)
{
	free(front_buffer);
	free(back_buffer);

	size_t cells = (size_t)(terminal_cols * terminal_rows);

	front_buffer = (struct pkt_cell *)calloc(cells, sizeof(struct pkt_cell));
	if (!front_buffer)
		return -1;

	back_buffer = (struct pkt_cell *)calloc(cells, sizeof(struct pkt_cell));
	if (!back_buffer) {
		free(front_buffer);
		return -1;
	}

	return 0;
}

static void __pkt_terminal_load_config(struct pkt_config *c)
{
	if (c->default_fcolor < 0 || c->default_fcolor > 255)
		user_default_fcolor = PKT_COLOR_WHITE;
	else
		user_default_fcolor = c->default_fcolor;

	if (c->default_bcolor < 0 || c->default_bcolor > 255)
		user_default_bcolor = PKT_COLOR_BLACK;
	else
		user_default_bcolor = c->default_bcolor;

	game_cols = c->game_cols;
	game_rows = c->game_rows;
}

// Just flag if window resized by OS
static void __pkt_terminal_flag_sigwinch(int sig)
{
	(void)sig;
	pending_resize_event = 1;
}

static void __pkt_terminal_flag_sigint(int sig)
{
	(void)sig;
	pending_quit_event = 1;	
}

static void __pkt_terminal_write_cell(int x, int y, unsigned int fcolor, unsigned int bcolor, 
		unsigned int attr, unsigned int flags, uint32_t ch)
{
	struct pkt_cell *bb = &back_buffer[y * terminal_cols + x];	
	bb->ch = ch;
	bb->fcolor = (uint8_t)fcolor;
	bb->bcolor = (uint8_t)bcolor;
	bb->attr = (uint8_t)attr;
	bb->flags = (uint8_t)flags;
}
