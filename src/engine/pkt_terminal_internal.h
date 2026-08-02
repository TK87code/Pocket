#ifndef PKT_TERMINAL_INTERNAL_H
#define PKT_TERMINAL_INTERNAL_H

#define PKT_DEFAULT_SCOL 80
#define PKT_DEFAULT_SROW 24

struct pkt_config;

int __pkt_terminal_update(void);
int __pkt_terminal_init(struct pkt_config *config);
int __pkt_terminal_restore(void);
int __pkt_terminal_read_input(void);
int __pkt_terminal_putc(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, uint8_t attr, char c);
int __pkt_terminal_puts(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, uint8_t attr, const char *str);

/*
 * Check terminal resize and store current cols and rows to the pointer passed as parameters.
 * Return 1 if resize occured, and 0 if didn't.
 */
int __pkt_terminal_check_resize(int *out_cols, int *out_rows);
int __pkt_terminal_get_termsize(int *out_cols, int *out_rows);

#endif

