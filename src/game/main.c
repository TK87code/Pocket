#include <stdio.h>
#include "../engine/p_screen.h"
#include "../engine/p_input.h"

int main(void) {
	p_input_init();
	p_screen_clear();
	p_cursor_hide();
	
	int player_input = 0;
	int p_x = 1, p_y = 1;

	while (1) {
		p_screen_clear();
		p_cursor_move(p_x, p_y);

		p_fcolor_set(P_COLOR_RED);
		printf("@");
		p_fcolor_set(P_COLOR_DEFAULT);
		fflush(stdout);

		player_input = p_input_read();

		if (player_input == 'q') break;

		if (player_input == 'h') {
			if (p_x - 1 >= 1) p_x -= 1; // left top corner is 1,1 in ANSI escape sequece
		}
		else if (player_input == 'l') {
			if (p_x + 1 <= 30) p_x += 1;
		}
		else if (player_input == 'j') {
			if (p_y + 1 <= 30) p_y += 1;
		}
		else if (player_input == 'k') {
			if (p_y - 1 >= 1) p_y -= 1;
		}

	}

	p_screen_clear();
	p_cursor_move(1,1);
	p_cursor_show();
	p_fcolor_set(P_COLOR_DEFAULT);

	p_input_restore();
	return 0;
}
