#ifndef ALLOC_H
#define ALLOC_H

#include <kernel/types.h>

/* Allocation flags */
#define ALLOC_ZERO_INIT  0x00000001  // Initialize memory region to zero.
#define ALLOC_PAGE_ALIGN 0x00000002  // Allocations must be page aligned

void* kmalloc(unsigned long size, uint32_t flags);
void* kmalloc_largest_available(unsigned long size, uint32_t flags, unsigned long *allocated_size);
void free(void* memory);

#endif