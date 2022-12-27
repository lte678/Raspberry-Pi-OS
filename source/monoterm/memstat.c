#include <kernel/print.h>
#include <kernel/pagetable.h>
#include <kernel/address_space.h>
#include <kernel/types.h>
#include "bindings.h"
#include "../alloc/buddy.h"


extern monoterm_func monoterm_buddy_print_map;
extern monoterm_func monoterm_buddy_print_free_lists;

// Linkers symbols to print memory regions
extern uint32_t __static_memory_start[];
extern uint32_t __static_memory_end[];


static void print_memory_usage() {
    print("{ul} bytes allocated\r\n", memory_allocated());
    print("{ul} mem_blk structs used\r\n", buddy_used_block_structs());
    print("{ul} mem_blk structs free\r\n", buddy_free_block_structs());
}

static void print_regions() {
    print("Kernel Static: {p} - {p}\r\n", (void*)__static_memory_start, (void*)__static_memory_end);
    print("Heap         : {p} - {p}\r\n", buddy_heap_start(), buddy_heap_end());
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
        } else if(!strcmp(argv[1], "pagetable")) {
            page_table_print(kernel_page_table);
            return 0;
        } else if(!strcmp(argv[1], "mappings")) {
            print_address_space(kernel_address_space);
            return 0;
        }
    }
    
    print("Usage: memstat [map|regions|free_lists|pagetable|mappings]\r\n");
    return 1;
}