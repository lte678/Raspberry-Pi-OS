#pragma once

#include <kernel/types.h>


struct address_mapping {
    struct address_mapping *next;
    uint64_t vaddress;
    uint64_t paddress;
    uint64_t size;
};


struct address_space {
    struct address_mapping *mappings;
    uint64_t *page_table;
};


int map_memory_region(struct address_space *aspace, uint64_t vaddr, uint64_t paddr, uint64_t size);