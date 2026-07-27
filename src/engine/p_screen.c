#include <stdio.h>
#include "pocket.h"
#include "p_screen_internal.h"

struct p_cell {
	char c;
	int color;
};

static struct p_cell front_buffer[P_DEFAULT_SHEIGHT][P_DEFAULT_SWIDTH];
static struct p_cell back_buffer[P_DEFAULT_SHEIGHT][P_DEFAULT_SWIDTH];

int p_draw_char(int x, int y, int color, char c)
{
	if (x < 0 || x >= P_DEFAULT_SWIDTH || y < 0 || y >= P_DEFAULT_SHEIGHT)
		return -1;
	back_buffer[y][x].c = c;
	back_buffer[y][x].color = color;

	return 0;
}

int p_draw_str(int x, int y, int color, const char *str)
{
	if (x < 0 || x >= P_DEFAULT_SWIDTH || y < 0 || y >= P_DEFAULT_SHEIGHT)
		return -1;

	int i = 0;
	while (str[i] != '\0') {
		p_draw_char(x + i, y, color, str[i]);
		i++;
	}
	
	return 0;
}

void __p_clear_screen(void) 
{
	printf("\x1b[2J"); // ANSI Escape sequence to clear screen 
	fflush(stdout); // force flushing buffer stored by printf
}

// Move cursor using ANSI escape sequence (ANSI coordinates starts with 1)
void __p_move_cursor(int x, int y) 
{
	printf("\x1b[%d;%dH", y + 1, x + 1); 
}

void __p_set_fcolor(int color)  
{
	printf("\x1b[%dm", color);
}

void __p_hide_cursor(void) 
{
	printf("\x1b[?25l"); // ANSI escape sequence to hide cursor
	fflush(stdout);
}

void __p_show_cursor(void) 
{
	printf("\x1b[?25h"); // ANSI escape sequence to show cursor
	fflush(stdout);
}

void __p_update_screen(void)
{
	for (int y = 0; y < P_DEFAULT_SHEIGHT; y++) {
		for (int x = 0; x < P_DEFAULT_SWIDTH; x++){
			if (back_buffer[y][x].c != front_buffer[y][x].c 
					|| back_buffer[y][x].color != front_buffer[y][x].color) {
				__p_move_cursor(x, y);
				__p_set_fcolor(back_buffer[y][x].color);
				printf("%c", back_buffer[y][x].c);

				front_buffer[y][x] = back_buffer[y][x];
			}	

			back_buffer[y][x].c = ' ';
			back_buffer[y][x].color = P_COLOR_DEFAULT;
		}
	}

	fflush(stdout);
}
