#include <kernel/string.h>
#include "uart.h"


void print_int(int number) {
    // Theoretically, we should never surpass 12 characters (including null byte)
    char buff[12];
    if(itos(number, buff, sizeof(buff)) > 0) {
        uart_print(buff);
        return;
    }
    uart_print("ERR");
}