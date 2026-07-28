#ifndef PKT_TERMINAL_INTERNAL_H
#define PKT_TERMINAL_INTERNAL_H

int __pkt_terminal_update(void);
int __pkt_terminal_init(void);
int __pkt_terminal_restore(void);
int __pkt_terminal_getch(void);
int __pkt_terminal_putch(int x, int y, int color, char c);
int __pkt_terminal_putstr(int x, int y, int color, const char *str);

#endif

