#ifndef P_SCREEN_H
#define P_SCREEN_H

// COLOR CODES
#define P_COLOR_DEFAULT 0 
#define P_COLOR_RED 31
#define P_COLOR_GREEN 32
#define P_COLOR_BLUE 34

/**
 * @brief  Clear screen
 */
void p_screen_clear(void);


/**
 * @brief  Move cursor to specified x and y
 *
 * @param  x x(COL) coordinate 
 * @param  y y(ROW) coordinate
 */
void p_cursor_move(int x, int y);

/**
 * @brief  Hide terminal cursor 
 */
void p_cursor_hide(void);

/**
 * @brief  Show terminal cursor
 */
void p_cursor_show(void);

/**
 * @brief  Change color of characters
 *
 * @param  color_code Color code difined in p_screen.h
 */
void p_fcolor_set(int color_code);

#endif

