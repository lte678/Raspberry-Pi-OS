#include <kernel/print.h>
#include "../alloc/buddy.h"


static void print_memory_usage() {
    print_ulong(memory_allocated());
    uart_print(" bytes allocated\r\n");
}

int monoterm_memstat(int argc, char *argv[]) {
    if(argc == 1) {
        // No argument
        print_memory_usage();
        return 0;
    } else {
        uart_print("Usage: memstat\r\n");
        return 1;
    }
}