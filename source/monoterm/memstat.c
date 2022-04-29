#include <kernel/print.h>
#include <kernel/types.h>
#include "bindings.h"
#include "../alloc/buddy.h"


extern monoterm_func monoterm_buddy_print_map;
extern monoterm_func monoterm_buddy_print_free_lists;

// Linkers symbols to print memory regions
extern uint32_t __static_memory_start[];
extern uint32_t __static_memory_end[];


static void print_memory_usage() {
    print_ulong(memory_allocated());
    uart_print(" bytes allocated\r\n");
    print_ulong(buddy_used_block_structs());
    uart_print(" mem_blk structs used\r\n");
    print_ulong(buddy_free_block_structs());
    uart_print(" mem_blk structs free\r\n");
}

static void print_regions() {
    uart_print("Kernel Static: 0x");
    print_hex_uint32((uint64_t)__static_memory_start);
    uart_print(" - 0x");
    print_hex_uint32((uint64_t)__static_memory_end);
    uart_print("\r\n");
    uart_print("Heap         : 0x");
    print_hex_uint32((uint64_t)buddy_heap_start());
    uart_print(" - 0x");
    print_hex_uint32((uint64_t)buddy_heap_end());
    uart_print("\r\n");
}


int monoterm_memstat(int argc, char *argv[]) {
    if(argc == 1) {
        // No argument
        print_memory_usage();
        return 0;
    } else if(argc >= 2) {
        // One or more arguments
        if(!strcmp(argv[1], "map")) {
            return monoterm_buddy_print_map(argc, argv);
        } else if(!strcmp(argv[1], "regions")) {
            print_regions();
            return 0;
        } else if(!strcmp(argv[1], "free_lists")) {
            return monoterm_buddy_print_free_lists(argc, argv);
        }
    }
    
    uart_print("Usage: memstat [map|regions|free_lists]\r\n");
    return 1;
}