#ifndef BUDDY_H
#define BUDDY_H

void init_buddy_allocator();
unsigned long memory_allocated();
int monoterm_buddy_print_map(int argc, char* argv[]);

#endif /* BUDDY_H */
