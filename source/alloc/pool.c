#include <kernel/alloc.h>

#include "pool.h"


static void pool_allocate_new_block(struct memory_pool *p, void* new_block) {
    /* Create a new block of objects and prepend them to the 'free' linked list. */
    if(!new_block) {
        new_block = kmalloc(p->block_size * p->object_size);
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

    /* Add at least one block to the pool */
    pool_allocate_new_block(&p, block);

    return p;
}


void* pool_alloc(struct memory_pool *p) {
    if(!p->next_free) {
        /* Try allocating */
        pool_allocate_new_block(p, 0);
        if(!p->next_free) {
            /* Fail otherwise */
            return 0;
        }
    }

    /* next_free is guaranteed to point to valid memory */
    void* free = p->next_free;
    /* Dereferencing next_free gives us a pointer to the next free (or 0) */
    p->next_free = *(void**)(p->next_free);

    /* Update counter */
    p->free_objects--;
    return free;
}

void pool_free(struct memory_pool *p, void* object) {
    *(void**)object = p->next_free;
    p->next_free = object;

    /*Update counter*/
    p->free_objects++;
}
