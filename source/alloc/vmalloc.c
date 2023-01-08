#include <kernel/alloc.h>
#include <kernel/types.h>
#include <kernel/print.h>
#include <kernel/address_space.h>

// TODO: All sections of this file are only partially implemented!

struct vmalloc_mapping_block {
    struct vmalloc_mapping_block* next;
    void* mapped_block;
};

struct vmalloc_mapping {
    struct vmalloc_mapping* next;
    struct vmalloc_mapping_block* block_list;
    uint64_t vaddress;
    uint64_t mapped_bytes;
    uint32_t allocation_flags;
};

// Up to 1024 vmalloc mappings
#define VMALLOC_BASE_ADDR          0x0000B00000000000ul
#define VMALLOC_MAX_MAPPING_SIZE   0x0000000100000000ul
#define VMALLOC_MAX_MAPPINGS 1024
#define VMALLOC_MAPPING_LIST_LENGTH (VMALLOC_MAX_MAPPINGS / 64)
static uint64_t vmalloc_free_mask[VMALLOC_MAPPING_LIST_LENGTH]; 

struct vmalloc_mapping* vmalloc_mapping_list;


/**
 * @brief Returns the next free virtual address slot
 * 
 * @return uint64_t 
 */
static uint64_t vmalloc_get_free_vaddr() {
    for(int i = 0; i < VMALLOC_MAX_MAPPINGS; i++) {
        int list_idx = i / 64;
        int bit_idx = i % 64;
        if(!(vmalloc_free_mask[list_idx] & (1ul << bit_idx))) {
            vmalloc_free_mask[list_idx] |= 1ul << bit_idx;
            return VMALLOC_BASE_ADDR + (VMALLOC_MAX_MAPPING_SIZE * (uint64_t)i);
        }
    }
    print("mmap: error: out of virtual addresses!\n");
    return 0;
}


int vmalloc_reallocate(struct vmalloc_mapping* mapping, unsigned long new_size) {
    while(mapping->mapped_bytes < new_size) {
        // Allocate the new memory
        uint64_t appended_bytes;
        void* new_mem = kmalloc_largest_available(new_size - mapping->mapped_bytes, mapping->allocation_flags, &appended_bytes);
        if(!new_mem) {
            // Not enough system memory left!
            return 1;
        }
        
        struct vmalloc_mapping_block* new_blk = kmalloc(sizeof(struct vmalloc_mapping_block), 0);
        if(!new_blk) {
            return 1;
        }

        uint64_t vaddr = mapping->vaddress + mapping->mapped_bytes;

        // Insert the mappings into our page table
        struct address_mapping* new_mapping = create_memory_region_virt(kernel_address_space, vaddr, (uint64_t)new_mem, appended_bytes);
        if(!new_mapping) {
            print("vmalloc: error: failed to create map for {x} bytes from 0x{xl} -> 0x{xl}\n", appended_bytes, (uint64_t)new_mem, vaddr);
            free(new_mem);
            return 1;
        }

        // Activate mapping
        if(map_memory_region(kernel_address_space, new_mapping)) {
            print("vmalloc: error: failed to create map for {x} bytes from 0x{xl} -> 0x{xl}\n", appended_bytes, (uint64_t)new_mem, vaddr);
            // This tolerates the mapping not being mapped. It will remove it from the linked list.
            unmap_and_remove_memory_region(kernel_address_space, new_mapping);
            free(new_mapping);
            free(new_mem);
            return 1;
        }

        // Insert mappings into lists
        new_blk->mapped_block = new_mem;
        new_blk->next = mapping->block_list;
        mapping->block_list = new_blk;
        // Update size of mapping
        mapping->mapped_bytes += appended_bytes;
    }
    return 0;
}


void* vmalloc(unsigned long size, uint32_t flags) {
    // Allocate space for the metadata
    struct vmalloc_mapping* new = kmalloc(sizeof(struct vmalloc_mapping), 0);
    if(!new) {
        return 0;
    }
    new->allocation_flags = flags;
    new->vaddress = vmalloc_get_free_vaddr();
    if(!new->vaddress) {
        return 0;
    }
    new->mapped_bytes = 0;
    new->block_list = 0;
    new->next = 0;

    // Allocate and map the requested memory
    if(vmalloc_reallocate(new, size)) {
        free(new);
        return 0;
    }
    
    // Insert the mapping into the global list
    new->next = vmalloc_mapping_list;
    vmalloc_mapping_list = new;
    return (void*)(new->vaddress);
}


void vmalloc_free(void* address) {
    print("vmalloc_free: maybe unimplemented!\n");
    struct vmalloc_mapping* i = vmalloc_mapping_list;
    struct vmalloc_mapping* prev = 0;
    while(i) {
        if(i->vaddress == (uint64_t)address) {
            // This is the mapping to delete
            // Remove mapping from list
            if(prev) {
                prev->next = i->next;
            } else {
                // The root node
                vmalloc_mapping_list = i->next;
            }  
            // Remove allocated blocks
            struct vmalloc_mapping_block* blk = i->block_list;
            while(blk) {
                // Free block and its descriptor
                free(blk->mapped_block);
                struct vmalloc_mapping_block* next_blk = blk->next;
                free(blk);
                blk = next_blk;
            }
            // Free mapping
            free(i);
        }
        prev = i;
        i = i->next;
    }
}