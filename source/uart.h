#ifndef _UART
#define _UART

#include <kernel/register.h>

void uart_pre_init(void);
int uart_init(void);

void uart_send(unsigned char c);
unsigned char uart_recv(void);
void uart_flush(void);

void hexstrings(unsigned int d);
void hexstring(unsigned int d);
void uart_print(char *string);

#endif
