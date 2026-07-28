#include <stdio.h>
#include <termios.h>
#include <unistd.h>	//STDIN_FILENO 
#include "pocket.h"
#include "p_terminal_internal.h"

struct p_cell {
	char c;
	int color;
};

static void __p_terminal_clear(void);
static void __p_terminal_hidecurs(void);
static void __p_terminal_showcurs(void);
static void __p_terminal_mvcurs(int x, int y);
static void __p_terminal_setfc(int color);

static struct termios original_term;
static struct p_cell front_buffer[P_DEFAULT_SHEIGHT][P_DEFAULT_SWIDTH];
static struct p_cell back_buffer[P_DEFAULT_SHEIGHT][P_DEFAULT_SWIDTH];

int p_terminal_putch(int x, int y, int color, char c)
{
	if (x < 0 || x >= P_DEFAULT_SWIDTH || y < 0 || y >= P_DEFAULT_SHEIGHT)
		return -1;
	back_buffer[y][x].c = c;
	back_buffer[y][x].color = color;

	return 0;
}

int p_terminal_putstr(int x, int y, int color, const char *str)
{
	if (x < 0 || x >= P_DEFAULT_SWIDTH || y < 0 || y >= P_DEFAULT_SHEIGHT)
		return -1;

	int i = 0;
	while (str[i] != '\0') {
		p_terminal_putch(x + i, y, color, str[i]);
		i++;
	}
	
	return 0;
}

void __p_terminal_init(void) 
{
	tcgetattr(STDIN_FILENO, &original_term); // store user terminal attribute to restore later.
	struct termios new_term = original_term;
	new_term.c_lflag &= ~(ECHO | ICANON); // Disable echo and ICANON(waiting untill user hit enter)
	new_term.c_cc[VMIN] = 0; // wait at least 0 characters
	new_term.c_cc[VTIME] = 0; // get realtime input 
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term); // Set terminal attribute immediately(TCSANOW)
	
	__p_terminal_clear();
	__p_terminal_hidecurs();
}

void __p_terminal_restore(void) 
{
	tcsetattr(STDIN_FILENO, TCSANOW, &original_term); // Restore original attribute 
	__p_terminal_mvcurs(0, 0);
	__p_terminal_showcurs();
	__p_terminal_clear();
}

int __p_terminal_getch(void) 
{
	unsigned char c;
	// order OS to read 1 byte directly from file descriptor
	if (read(STDIN_FILENO, &c, 1) == 1)
		return c; // Key has pressed 

	return -1; // Key has not pressed 
}

void __p_terminal_update(void)
{
	for (int y = 0; y < P_DEFAULT_SHEIGHT; y++) {
		for (int x = 0; x < P_DEFAULT_SWIDTH; x++){
			if (back_buffer[y][x].c != front_buffer[y][x].c 
					|| back_buffer[y][x].color != front_buffer[y][x].color) {
				__p_terminal_mvcurs(x, y);
				__p_terminal_setfc(back_buffer[y][x].color);
				printf("%c", back_buffer[y][x].c);

				front_buffer[y][x] = back_buffer[y][x];
			}	

			back_buffer[y][x].c = ' ';
			back_buffer[y][x].color = P_COLOR_DEFAULT;
		}
	}

	fflush(stdout);
}

static void __p_terminal_clear(void) 
{
	printf("\x1b[2J"); // ANSI Escape sequence to clear screen 
	fflush(stdout); // force flushing buffer stored by printf
}

static void __p_terminal_hidecurs(void) 
{
	printf("\x1b[?25l"); // ANSI escape sequence to hide cursor
	fflush(stdout);
}

static void __p_terminal_showcurs(void) 
{
	printf("\x1b[?25h"); // ANSI escape sequence to show cursor
	fflush(stdout);
}

// Move cursor using ANSI escape sequence (ANSI coordinates starts with 1)
static void __p_terminal_mvcurs(int x, int y) 
{
	printf("\x1b[%d;%dH", y + 1, x + 1); 
}

// Set front color of terminal
static void __p_terminal_setfc(int color)  
{
	printf("\x1b[%dm", color);
}
