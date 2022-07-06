#ifndef PRINT_H
#define PRINT_H

#include <kernel/string.h>
#include <kernel/types.h>


extern void uart_send(unsigned char c);
extern void uart_print(char *string);
extern void print_int(int number);
extern void print_uint(unsigned int number);
extern void print_long(long number);
extern void print_ulong(unsigned long number);
extern void print_hex(unsigned char* bytes, unsigned short n);
extern void print_hex_be(unsigned char* bytes, unsigned short n);
extern void print_hex_uint32(unsigned int num);
extern void print_hex_uint64(uint64_t num);
extern void print_address(void* addr);
// extern void print_uint(unsigned int number);

// Terminal control commands
extern void term_set_cursor(int x, int y);
extern void term_set_cursor_column(int x);
extern void term_set_cursor_row(int x);
extern void term_set_bold();
extern void term_set_color(int code);
extern void term_reset_font();

#endif  // PRINT_H