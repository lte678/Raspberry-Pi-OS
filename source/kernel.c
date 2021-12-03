#include "uart.h"

void kernel_entry_point(void) {
    uart_init();
    uart_print("Booting LXE...\n");
    uart_print("Developed by Leon Teichroeb :)\n");
}
