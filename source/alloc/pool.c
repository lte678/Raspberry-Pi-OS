#include <kernel/alloc.h>
#include <kernel/print.h>

#include "pool.h"


/**
 * @brief Allocates a new pool block
 * 
 * @param p memory_pool struct
 * @param new_block Prespecified memory region to add to pool.
 */
static void pool_allocate_new_block(struct memory_pool *p, void* new_block) {
    /* Create a new block of objects and prepend them to the 'free' linked list. */
    if(!new_block) {
        p->no_new_block = 1;
        new_block = kmalloc(p->block_size * p->object_size, 0);
        p->no_new_block = 0;
        if(!new_block) {
            return;
        }
    } 
    void* start_of_list = new_block;
    /* *(void**)new_block = sets the pointer of the newly created block */
    for(int i = 0; i < p->block_size - 1; i++) {
        *(void**)new_block = new_block + p->object_size;
        new_block += p->object_size;
    }
    *(void**)new_block = p->next_free;
    p->next_free = start_of_list;

    /* Update counters */
    p->objects += p->block_size;
    p->free_objects += p->block_size;
}


/**
 * @brief Returns a suitable memory_pool struct. 
 * 
 * @param object_size The size of the data objects that we will store in the pool. Bounded to >=8 bytes.
 * @param block_size The number of objects that fit into a single pool block.
 * Large block sizes decrease the number allocations, but increase the potentially wasted space.
 * @return struct memory_pool 
 */
struct memory_pool pool_create_pool(unsigned int object_size, unsigned int block_size) {
    struct memory_pool p;

    /* Create an empty pool */
    if(object_size >= 8) {
        p.object_size = object_size;
    } else {
        // Size of pointer
        p.object_size = 8;
    }
    p.block_size = block_size;
    p.next_free = 0;
    p.objects = 0;
    p.free_objects = 0;

    /* Add at least one block to the pool */
    pool_allocate_new_block(&p, 0);

    return p;
}


/**
 * @brief Identical to pool_create_pool(), but uses a prespecified memory region.
 * 
 * @param object_size The size of the data objects that we will store in the pool. Bounded to >=8 bytes.
 * @param block_bytes The size of the provided memory region.
 * The number of objects per pool block is set according to the available space in the memory region.
 * @param block Pointer to the memory region.
 * @return struct memory_pool 
 */
struct memory_pool pool_create_pool_using_memory(unsigned int object_size, unsigned int block_bytes, void* block) {
        struct memory_pool p;

    /* Create an empty pool */
    if(object_size >= 8) {
        p.object_size = object_size;
    } else {
        // Size of pointer
        p.object_size = 8;
    }
    p.block_size = block_bytes / object_size; /* C will round down for us */
    p.next_free = 0;
    p.objects = 0;
    p.free_objects = 0;
    p.no_new_block = 0;

    /* Add at least one block to the pool */
    pool_allocate_new_block(&p, block);

    return p;
}


/**
 * @brief Allocates a new pool object.
 * May attempt to allocate more space for the pool when above 80% utilisation.
 * 
 * @param p Pointer to the memory_pool struct.
 * @return Object pointer.
 */
void* pool_alloc(struct memory_pool *p) {
    int free_percent = (p->free_objects * 100) / (p->objects);
    if(free_percent < 20 && !p->no_new_block) {
        // We are allowed to allocate a new block
        pool_allocate_new_block(p, 0);
    }
    if(!p->next_free && !p->no_new_block) {
        /* Try allocating */
        pool_allocate_new_block(p, 0);
    }
    if(!p->next_free) {
        /* Fail otherwise */
        return 0;
    }

    /* next_free is guaranteed to point to valid memory */
    void* free = p->next_free;
    /* Dereferencing next_free gives us a pointer to the next free (or 0) */
    p->next_free = *(void**)(p->next_free);

    /* Update counter */
    p->free_objects--;
    // print("pool alloc: {p}\n", free);
    return free;
}


/**
 * @brief Frees object from pool.
 * Must belong to the provided memory_pool.
 * 
 * @param p memory_pool struct
 * @param object Pointer to object
 */
void pool_free(struct memory_pool *p, void* object) {
    if(!object) {
        print("Tried to free null pointer!\n");
        return;
    }
    *(void**)object = p->next_free;
    p->next_free = object;

    /*Update counter*/
    p->free_objects++;
    //print("pool.free_objects={u}\n", p->free_objects);
}
