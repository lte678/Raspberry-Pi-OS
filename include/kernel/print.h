#ifndef PRINT_H
#define PRINT_H

#include <kernel/string.h>


extern void uart_print(char *string);
extern void print_int(int number);
extern void print_uint(unsigned int number);
extern void print_hex(unsigned char* bytes, unsigned short n);
extern void print_hex_uint32(unsigned int num);
// extern void print_uint(unsigned int number);


#endif  // PRINT_H