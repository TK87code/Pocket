#ifndef P_SCREEN_INTERNAL_H
#define P_SCREEN_INTERNAL_H

void __p_screen_clear(void);

void __p_screen_update(void);

void __p_screen_hidecurs(void);

void __p_screen_showcurs(void);

void __p_screen_mvcurs(int x, int y);

void __p_screen_setfc(int color);

#endif

