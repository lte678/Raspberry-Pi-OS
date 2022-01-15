#ifndef PRINT_H
#define PRINT_H

#include <kernel/string.h>


extern void uart_print(char *string);
extern void print_int(int number);
// extern void print_uint(unsigned int number);


#endif  // PRINT_H