#ifndef POOL_H
#define POOL_H

struct memory_pool {
    /* The next free block of memory */
    void* next_free;
    unsigned int object_size;
    unsigned int block_size;
    unsigned int objects;
    unsigned int free_objects;
};

struct memory_pool pool_create_pool(unsigned int object_size, unsigned int block_size);
void* pool_alloc(struct memory_pool *p);
void pool_free(struct memory_pool *p, void* object);

#endif /* POOL_H */