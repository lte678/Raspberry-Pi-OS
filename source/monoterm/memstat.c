#include <kernel/print.h>
#include "bindings.h"
#include "../alloc/buddy.h"


extern monoterm_func monoterm_buddy_print_map;


static void print_memory_usage() {
    print_ulong(memory_allocated());
    uart_print(" bytes allocated\r\n");
}

int monoterm_memstat(int argc, char *argv[]) {
    if(argc == 1) {
        // No argument
        print_memory_usage();
        return 0;
    } else if(argc == 2) {
        // One argument
        if(!strcmp(argv[1], "map")) {
            monoterm_buddy_print_map(argc, argv);
        }
        return 0;
    }
    
    uart_print("Usage: memstat [map]\r\n");
    return 1;
}