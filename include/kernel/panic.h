//
// Created by leon on 21.03.22.
//

#ifndef RASPBERRY_PI_OS_PANIC_H
#define RASPBERRY_PI_OS_PANIC_H

#include <kernel/print.h>


_Noreturn static void panic() {
    print("####################\n");
    print("### KERNEL PANIC ###\n");
    print("####################\n");
    while(1);
}

#endif //RASPBERRY_PI_OS_PANIC_H
