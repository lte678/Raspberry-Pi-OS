#include <kernel/print.h>
#include "bindings.h"

extern void el1_entry();

// LEGACY: Non-functional in recent versions
int monoterm_restart(int argc, char *argv[]) {
    print("Restarting...\r\n");
    // Simple jump doens't work anymore, due to virtual memory etc...
    el1_entry();
    // This return should never be called...
    return 0;
}