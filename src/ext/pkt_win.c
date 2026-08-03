#define _XOPEN_SOURCE 500 	// X/Open System Interfaces Extension (XSI) Issue 5
#include <stdlib.h>		// mbtowc 
#include <wchar.h>		// wchar_t, wcwidth
#include <string.h>		// memcpy
#include <stdarg.h>		// va_list, va_start, va_end
#include "pocket.h"
#include "ext/pkt_win.h"

static int __pkt_win_clipstr(const struct pkt_window *win, int x, const char *str);

struct pkt_window pkt_win_create(int x, int y, int width, int height)
{
	return (struct pkt_window) {
		.x = (int16_t)x,
		.y = (int16_t)y,
		.width = (int16_t)width,
		.height = (int16_t)height
	};
}

int pkt_win_putc(const struct pkt_window *win, int x, int y, char c)
{
	return pkt_win_putc_color(win, x, y, PKT_COLOR_DEFAULT, PKT_COLOR_DEFAULT, PKT_ATTR_NONE, c); 
}

int pkt_win_putc_color(const struct pkt_window *win, int x, int y, 
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, char c)
{
	if (x < 0 || x >= win->width || y < 0 || y >= win->height)
		return -1;

	return pkt_putc_color(win->x + x, win->y + y, fcolor, bcolor, attr, c);
}

int pkt_win_puts(const struct pkt_window *win, int x, int y, const char *str)
{
	return pkt_win_puts_color(win, x, y, PKT_COLOR_DEFAULT, PKT_COLOR_DEFAULT, PKT_ATTR_NONE, str);
}

int pkt_win_puts_color(const struct pkt_window *win, int x, int y, 
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *str)
{
	if (x < 0 || x >= win->width || y < 0 || y >= win->height)
		return -1;

	const char *s = __pkt_win_clipstr(win, x, str);

	return pkt_puts_color(win->x + x, win->y + y, fcolor, bcolor, attr, s);
}

int pkt_win_printf(const struct pkt_window *win, int x, int y, const char *fmt, ...)
{
	if (x < 0 || x >= win->width || y < 0 || y >= win->height)
		return -1;

	char buf[512] = {0};
	va_list v;
	va_start(v, fmt);
	vsnprintf(buf, sizeof(buf), fmt, v);
	va_end;
	
	const char *s = __pkt_win_clipstr(win, x, buf);

	return pkt_puts_color(win->x + x, win->y + y, PKT_COLOR_DEFAULT, PKT_COLOR_DEFAULT, PKT_ATTR_NONE, s);
}

int pkt_win_printf_color(const struct pkt_window *win, int x, int y, 
		enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *fmt, ...)
{
	if (x < 0 || x >= win->width || y < 0 || y >= win->height)
		return -1;

	char buf[512] = {0};
	va_list v;
	va_start(v, fmt);
	vsnprintf(buf, sizeof(buf), fmt, v);
	va_end(v);
	
	const char *s = __pkt_win_clipstr(win, x, buf);

	return pkt_puts_color(win->x + x, win->y + y, fcolor, bcolor, attr, s);
}

int pkt_win_box(struct pkt_window *win)
{
	return pkt_win_box_color(win, PKT_COLOR_DEFAULT, PKT_COLOR_DEFAULT, PKT_ATTR_DEFAULT);
}

int pkt_win_box_color(struct pkt_window *win, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr)
{
	if (win->width < 2 || win->height < 2)
		return -1; // not enough size
	
	pkt_win_puts_color(win, 0, 0, fcolor, bcolor, attr, "┌");
	pkt_win_puts_color(win, win->width - 1, 0, fcolor, bcolor, attr, "┐");
	pkt_win_puts_color(win, 0, win->height - 1, fcolor, bcolor, attr, "└");
	pkt_win_puts_color(win, win->width - 1, win->height - 1, fcolor, bcolor, attr, "┘");

	for (int x = 1; x < win->width - 1; x++) {
		pkt_win_puts_color(win, x, 0, fcolor, bcolor, attr, "─");
		pkt_win_puts_color(win, x, win->height - 1, fcolor, bcolor, attr, "─");
	}

	for (int y = 1; y < win->height - 1; y++) {
		pkt_win_puts_color(win, 0, y, fcolor, bcolor, attr, "│");
		pkt_win_puts_color(win, win->width - 1, y, fcolor, bcolor, attr, "│");
	}

	return 0;
}

/*
 * Calculate bytes to fit within the subwindow, considering multibyte characters might be passed to this function.
 * If the string fits in the subwindow, return the original string, otherwise clip the string and return the pointer to it
 * using Pocket's scratch memory arena.
 */
static const char *__pkt_win_clipstr(const struct pkt_window *win, int x, const char *str)
{
	int i = 0;
	int current_x = x;
	int bytes_to_fit = 0;

	while (str[i] != '\0') {
		wchar_t wc = 0;
		int bytes = mbtowc(&wc, &str[i], MB_CUR_MAX);
		if (bytes <= 0)
			break;
		
		int w = wcwidth(wc);
		if (w < 0)
			w = 0;

		if (current_x + w > win->width)
			break;

		current_x += w;
		bytes_to_fit += bytes;
		i += bytes;
	}

	if (str[bytes_to_fit] == '\0')
		return str;	

	char *clipped_buf = (char *)pkt_scratch_alloc((size_t)(bytes_to_fit + 1));
	if (!clipped_buf)
		return str;

	memcpy(clipped_buf, str, (size_t)bytes_to_fit);
	clipped_buf[bytes_to_fit] = '\0';

	return clipped_buf;
}
