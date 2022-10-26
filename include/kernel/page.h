#pragma once

#include <kernel/types.h>


#define VA_OFFSET 0x0000800000000000
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 512
#define PAGE_TABLE_LEVELS 3

#define PT_ADDRESS_TO_KERN(addr) ((uint64_t*)((char*)addr + VA_OFFSET))
#define KERN_TO_PHYS(addr) ((uint64_t*)((char*)addr - VA_OFFSET))

uint64_t round_up_to_page(uint64_t a);