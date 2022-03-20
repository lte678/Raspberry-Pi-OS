#ifndef BUDDY_H
#define BUDDY_H


int init_buddy_allocator();
unsigned long memory_allocated();
void* buddy_heap_start();
void* buddy_heap_end();
int monoterm_buddy_print_map(int argc, char* argv[]);


#endif /* BUDDY_H */
