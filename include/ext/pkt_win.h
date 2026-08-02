#ifndef PKT_WIN_H
#define PKT_WIN_H

#include <stdint.h>
#include "../pocket.h"

struct pkt_window { // 8 bytes
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

struct pkt_window pkt_win_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

int pkt_win_putc(struct pkt_window *win, uint16_t x, uint16_t y, char c);

int pkt_win_putc_color(struct pkt_window *win, uint16_t x, uint16_t y, 
		struct pkt_fcolor fcolor, struct pkt_color bcolor, uint8_t attr, char c);

int pkt_win_puts(struct pkt_window *win, uint16_t x, uint16_t y, const char *str);

int pkt_win_puts_color(struct pkt_window *win, uint16_t x, uint16_t y,
		struct pkt_fcolor fcolor, struct pkt_color bcolor, uint8_t attr, const char *str);

int pkt_win_printf(struct pkt_window *win, uint16_t x, uint16_t y, const char *fmt, ...);

int pkt_win_printf_color(struct pkt_window *win, uint16_t x, uint16_t y, 
		struct pkt_fcolor fcolor, struct pkt_color bcolor, uint8_t attr, const char *fmt, ...);

int pkt_win_box(struct pkt_window *win);
#endif
