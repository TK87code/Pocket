#ifndef PKT_TERMINAL_INTERNAL_H
#define PKT_TERMINAL_INTERNAL_H

#define PKT_DEFAULT_SCOL 80
#define PKT_DEFAULT_SROW 24

struct pkt_config;

int __pkt_terminal_update(void);
int __pkt_terminal_init(struct pkt_config *config);
int __pkt_terminal_restore(void);
int __pkt_terminal_read_input(void);
int __pkt_terminal_putc(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, char c);
int __pkt_terminal_puts(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, const char *str);

#endif

