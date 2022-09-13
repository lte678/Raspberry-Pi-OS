#pragma once

#include <kernel/types.h>


#define VA_OFFSET 0x0000800000000000
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 512


static uint64_t round_up_to_page(uint64_t a) {
    if(a == 0) {
        return 0;
    }
    return ((a - 1) & ~(PAGE_SIZE-1)) + PAGE_SIZE;
}