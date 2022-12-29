#include <kernel/address_space.h>
#include <kernel/alloc.h>
#include <kernel/print.h>
#include <kernel/page.h>

#include <kernel/mmap.h>


#define MMAP_BASE_ADDR          0x0000A00000000000ul
#define MMAP_MAX_MAPPING_SIZE   0x0000000100000000ul
#define MMAP_MAX_MAPPINGS 1024
#define MMAP_MAPPING_LIST_LENGTH (MMAP_MAX_MAPPINGS / 64)
static uint64_t mmap_free_mask[MMAP_MAPPING_LIST_LENGTH]; 

/**
 * @brief Returns the next free virtual address slot
 * 
 * @return uint64_t 
 */
static uint64_t mmap_get_free_vaddr() {
    for(int i = 0; i < MMAP_MAX_MAPPINGS; i++) {
        int list_idx = i / 64;
        int bit_idx = i % 64;
        if(!(mmap_free_mask[list_idx] & (1ul << bit_idx))) {
            mmap_free_mask[list_idx] |= 1ul << bit_idx;
            return MMAP_BASE_ADDR + (MMAP_MAX_MAPPING_SIZE * (uint64_t)i);
        }
    }
    print("mmap: error: out of virtual addresses!\n");
    return 0;
}

void* mmap(uint64_t pa, uint64_t size) {
    // Preform checks
    if((pa & (PAGE_SIZE - 1)) || (size & (PAGE_SIZE - 1))) {
        print("mmap: error: Invalid page alignment!\n");
        return 0;
    }

    if(size > MMAP_MAX_MAPPING_SIZE) {
        print("mmap: error: max mapping size exceeded!\n");
        return 0;
    }
    // Reserve new virtual address region
    uint64_t vaddr = mmap_get_free_vaddr();
    if(!vaddr) {
        print("mmap: error: failed to obtain free virtual address\n");
        return 0;
    }

    // Create mapping
    struct address_mapping* new_mapping = create_memory_region(kernel_address_space, vaddr, pa, size);
    if(!new_mapping) {
        print("mmap: error: failed to create map for {x} bytes from 0x{xl} -> 0x{xl}\n", size, pa, vaddr);
        return 0;
    }

    // Activate mapping
    if(map_memory_region(kernel_address_space, new_mapping)) {
        print("mmap: error: failed to create map for {x} bytes from 0x{xl} -> 0x{xl}\n", size, pa, vaddr);
        // This tolerates the mapping not being mapped. It will remove it from the linked list.
        unmap_and_remove_memory_region(kernel_address_space, new_mapping);
        free(new_mapping);
        return 0;
    }

    return (void*)vaddr;
}