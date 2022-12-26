#pragma once

#include <kernel/types.h>


struct address_mapping {
    struct address_mapping *next, *prev;
    uint64_t vaddress;
    uint64_t paddress;
    uint64_t size;
    bool_t   active;
};


struct address_space {
    struct address_mapping *mappings;
    uint64_t *page_table;
};


extern struct address_space* kernel_address_space;


int map_memory_region(struct address_space *aspace, uint64_t vaddr, uint64_t paddr, uint64_t size);
int map_memory_region_virt(struct address_space *aspace, uint64_t vaddr, uint64_t kaddr, uint64_t size);
int unmap_memory_region(struct address_space *aspace, struct address_mapping *mapping);
int unmap_and_remove_memory_region(struct address_space *aspace, struct address_mapping *mapping);
struct address_space* allocate_address_space();
struct address_space* init_kernel_address_space_struct();