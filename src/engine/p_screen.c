#include <stdio.h>
#include "p_screen.h"

void p_screen_clear(void) {
	printf("\x1b[2J"); // ANSI Escape sequence to clear screen 
	fflush(stdout); // force flushing buffer stored by printf
}

void p_cursor_move(int x, int y) {
	printf("\x1b[%d;%dH", y, x); // ANSI escape sequence to move cursor to specified location 
	fflush(stdout);
}

void p_cursor_hide(void) {
	printf("\x1b[?25l"); // ANSI escape sequence to hide cursor
	fflush(stdout);
}

void p_cursor_show(void) {
	printf("\x1b[?25h"); // ANSI escape sequence to show cursor
	fflush(stdout);
}

void p_fcolor_set(int color_code) {
	printf("\x1b[%dm", color_code); // ANSI escape sequence to change front color
	fflush(stdout);
}

