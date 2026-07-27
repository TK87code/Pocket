#ifndef P_SCREEN_INTERNAL_H
#define P_SCREEN_INTERNAL_H

void __p_clear_screen(void);

void __p_hide_cursor(void);

void __p_show_cursor(void);

void __p_update_screen(void);

void __p_set_fcolor(int color);

void __p_move_cursor(int x, int y);

#endif

