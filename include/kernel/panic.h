//
// Created by leon on 21.03.22.
//

#ifndef RASPBERRY_PI_OS_PANIC_H
#define RASPBERRY_PI_OS_PANIC_H

#include <kernel/print.h>


_Noreturn static void panic() {
    uart_print("####################\r\n");
    uart_print("### KERNEL PANIC ###\r\n");
    uart_print("####################\r\n");
    while(1);
}

#endif //RASPBERRY_PI_OS_PANIC_H
