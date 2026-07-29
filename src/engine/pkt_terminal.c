#include <stdio.h>
#include <termios.h>
#include <unistd.h>	//STDIN_FILENO 
#include <stdlib.h>     //calloc, free
#include <sys/ioctl.h>      // ioctl, TIOCGWINSZ
#include "pocket.h"
#include "pkt_terminal_internal.h"

struct pkt_cell {
	char c;
	int color;
};

static void __pkt_terminal_clear(void);
static void __pkt_terminal_hidecurs(void);
static void __pkt_terminal_showcurs(void);
static void __pkt_terminal_mvcurs(int x, int y);
static void __pkt_terminal_setfc(int color);
static int __pkt_terminal_init_buffers(void);
static void __pkt_terminal_start_altbuff(void);
static void __pkt_terminal_stop_altbuff(void);
static void __pkt_terminal_load_config(struct pkt_config *config);
static void __pkt_terminal_resize(int row, int col);

static struct termios original_term;
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
	for (int y = 0; y < row; y++) {
		for (int x = 0; x < col; x++){
			if (back_buffer[y * col + x].c != front_buffer[y * col + x].c 
					|| back_buffer[y * col + x].color != front_buffer[y * col + x].color) {
				__pkt_terminal_mvcurs(x, y);
				__pkt_terminal_setfc(back_buffer[y * col + x].color);
				printf("%c", back_buffer[y * col + x].c);
				front_buffer[y * col + x] = back_buffer[y * col + x];
			}	

			back_buffer[y * col + x].c = ' ';
			back_buffer[y * col + x].color = PKT_COLOR_DEFAULT;
		}
	}

	fflush(stdout);
	return 0;
}

int __pkt_terminal_putch(int x, int y, int color, char c)
{
	if (x < 0 || x >= col || y < 0 || y >= row)
		return -1;
	back_buffer[y * col + x].c = c;
	back_buffer[y * col + x].color = color;

	return 0;
}

int __pkt_terminal_putstr(int x, int y, int color, const char *str)
{
	if (x < 0 || x >= col || y < 0 || y >= row)
		return -1;

	int i = 0;
	while (str[i] != '\0') {
		__pkt_terminal_putch(x + i, y, color, str[i]);
		i++;
	}
	
	return 0;
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

// Set front color of terminal
static void __pkt_terminal_setfc(int color)  
{
	printf("\x1b[%dm", color);
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
