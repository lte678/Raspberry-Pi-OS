#include <kernel/print.h>
#include "bindings.h"

extern void el1_entry();


int monoterm_restart(int argc, char *argv[]) {
    uart_print("Restarting...\r\n");
    // TODO: Wrap this up in a neat kernel function
    el1_entry();
    // This return should never be called...
    return 0;
}