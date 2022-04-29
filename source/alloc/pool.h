#ifndef POOL_H
#define POOL_H

struct memory_pool {
    /* The next free block of memory */
    void* next_free;
    unsigned int object_size;
    unsigned int block_size;
    unsigned int objects;
    unsigned int free_objects;
    /* Flag indicating that a new block is being created, this may require the pool allocator, so we
    must make sure that we are not recursively trying to extend the memory region. */
    int no_new_block;
};

struct memory_pool pool_create_pool(unsigned int object_size, unsigned int block_size);
struct memory_pool pool_create_pool_using_memory(unsigned int object_size, unsigned int block_bytes, void* block);
void* pool_alloc(struct memory_pool *p);
void pool_free(struct memory_pool *p, void* object);

#endif /* POOL_H */