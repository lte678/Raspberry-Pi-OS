#ifndef ALLOC_H
#define ALLOC_H

#include <kernel/types.h>

/* Allocation flags */
#define ALLOC_ZERO_INIT 0x00000001  // Initialize memory region to zero.

void* kmalloc(unsigned long size, uint32_t flags);
void free(void* memory);

#endif