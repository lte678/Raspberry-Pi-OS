#ifndef BUDDY_H
#define BUDDY_H

int init_buddy_allocator();

unsigned long memory_allocated();
unsigned int buddy_free_block_structs();
unsigned int buddy_used_block_structs();
void* buddy_heap_start();
void* buddy_heap_end();

int monoterm_buddy_print_map(int argc, char* argv[]);


#endif /* BUDDY_H */
