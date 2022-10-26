#include <kernel/alloc.h>
#include <kernel/types.h>


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
#define VMALLOC_MAX_MAPPINGS 1024
static uint64_t vmalloc_free_mask[VMALLOC_MAX_MAPPINGS / 64]; 

struct vmalloc_mapping* vmalloc_mapping_list;


int vmalloc_reallocate(void* addr, unsigned long new_size) {
    // TODO get mapping
    struct vmalloc_mapping* mapping;

    while(mapping->mapped_bytes < new_size) {
        uint64_t appended_bytes;
        void* new_mem = kmalloc_largest_available(new_size - mapping->mapped_bytes, mapping->allocation_flags, &appended_bytes);
        if(!new_mem) {
            // Not enough system memory left!
            return 1;
        }
        // TODO: Create address mapping

        struct vmalloc_mapping_block* new_blk = kmalloc(sizeof(struct vmalloc_mapping_block), 0);
        if(!new_blk) {
                // Could not allocate the struct
            return 1;
        }
        struct vmalloc_mapping_block* old_blk = mapping->block_list;
        new_blk->next = old_blk;
        new_blk->mapped_block = new_mem;
        mapping->block_list = new_blk;
        mapping->mapped_bytes += appended_bytes;
    }
}

void* vmalloc(unsigned long size, uint32_t flags) {
    struct vmalloc_mapping* new = kmalloc(sizeof(struct vmalloc_mapping), 0);
    if(!new) {
        return 0;
    }
    new->allocation_flags = flags;
    if(vmalloc_reallocate(new, size)) {
        (new);
        free(new);
        return 0;
    }
    
    // Insert the mapping into the global list
    struct vmalloc_mapping* prev_mapping = vmalloc_mapping_list;
    new->next = prev_mapping;
    vmalloc_mapping_list = new;

}

void vmalloc_free(void* address) {
    struct vmalloc_mapping* i = vmalloc_mapping_list;
    struct vmalloc_mapping* prev = 0;
    while(i) {
        if(i->vaddress == address) {
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
                blk = blk->next;
            }
            // Free mapping
            free(i);
        }
        prev = i;
        i = i->next;
    }
}