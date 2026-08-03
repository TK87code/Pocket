#ifndef PKT_WIN_H
#define PKT_WIN_H

#include <stdint.h>
#include "../pocket.h"

struct pkt_window { // 8 bytes
	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
};

/**
 * @brief  Create subwindow structure.
 *
 * @param  x x coordinate where subwindow starts.
 * @param  y y coordinate where subwindow starts.
 * @param  width subwindow width(columns).
 * @param  height subwindow height(rows).
 *
 * @return A pkt_window structure.
 */
struct pkt_window pkt_win_create(int x, int y, int width, int height);

/**
 * @brief  Put a 1 byte character on the subwindow at local x and y coordinates.
 *
 * @param  win A subwindow structure to put character.
 * @param  x Local x position(columns).
 * @param  y Local y position(rows).
 * @param  c A character to put on subwindow.
 *
 * @return 0 on success. -1 when invalid x, y passed.
 */
int pkt_win_putc(const struct pkt_window *win, int x, int y, char c);

/**
 * @brief  Put a 1 byte character on subwindow at specified local x and y coordinates with colors and attributes.
 *
 * @param  win A subwindow structure to put character.
 * @param  x Local x position(columns).
 * @param  y Local y position(rows).
 * @param  fcolor Font color (e.g., PKT_COLOR_RED). You also can pass ANSI 256 color codes here (e.g., 13).
 * @param  bcolor Background color
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  c A character to put on subwindow.
 *
 * @return 0 on success -1 when invalid x, y, or color code passed. 
 */
int pkt_win_putc_color(const struct pkt_window *win, int x, int y, 
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, char c);

/**
 * @brief  Put a string or multi-byte character(utf-8) on a subwindow at specified local x and y coodinates. 
 *
 * @param  win A subwindow structure.
 * @param  x Local x position(columns).
 * @param  y Local y position(rows).
 * @param  str A string to put on subwindow.
 *
 * @return 0 on success, -1 when invalid x or y passed.

 */
int pkt_win_puts(const struct pkt_window *win, int x, int y, const char *str);

/**
 * @brief  Put a string or multi-byte character(UTF-8) on subwindow 
 * 	   at specified x and y coordinates with color and attributes.
 *
 * @param  win A subwindow structure.
 * @param  x Local x position(colmuns).
 * @param  y Local y position(rows).
 * @param  fcolor Font color (e.g., PKT_COLOR_RED). You also can pass ANSI 256 color code here.
 * @param  bcolor Background color.
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  str A string to put on subwindow.
 *
 * @return 0 on success, -1 when invalid parameters passed.
 */
int pkt_win_puts_color(const struct pkt_window *win, int x, int y,
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *str);

/**
 * @brief  Print a formatted string on subwindow at local x and y coordinates.
 *
 * @param  win A subwindow structure.
 * @param  x Local x position(columns).
 * @param  y Local y position(rows).
 * @param  fmt A formatted string to print.
 *
 * @return 0 on success, -1 when invalid x or y passed.
 */
int pkt_win_printf(const struct pkt_window *win, int x, int y, const char *fmt, ...);

/**
 * @brief  Print a formatted string on subwindow at x and y coordinates with color and attributes.
 *
 * @param  win A subwindow structure.
 * @param  x Local x position(columns).
 * @param  y Local y position(rows).
 * @param  fcolor Font color (e.g., PKT_COLOR_RED). You also can pass ANSI 256 color code here.
 * @param  bcolor Background color.
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  fmt A formatted string to print.
 *
 * @return 0 on success, -1 when invalid parameters passed.
 */
int pkt_win_printf_color(const struct pkt_window *win, int x, int y, 
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *fmt, ...);

/**
 * @brief  Draw a border arround subwindow.
 *
 * @note   Be aware that a border consumes first and last column and row. 
 *	   So you need to adjust position or length to put characters.
 *
 * @param  win A subwindow structure.
 *
 * @return 0 on success, -1 if submodule is not big enough to draw border.
 */
int pkt_win_box(struct pkt_window *win);

/**
 * @brief  Draw a border arround subwindow with color and attributes.
 *
 * @note   Be aware that a border consumes first and last column and row. 
 *	   So you need to adjust position or length to put characters.
 *	   
 * @param  fcolor Font color (e.g., PKT_COLOR_RED). You also can pass ANSI 256 color code here.
 * @param  bcolor Background color.
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 *
 * @return 
 */
int pkt_win_box_color(struct pkt_window *win, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr);

#endif
