#pragma once

#include <kernel/address_space.h>


struct process {
    uint16_t pid;
    struct address_space *addr_space;
    struct process_memory_handle *allocated_list;
    void* process_entry_point;
};


struct process_memory_handle {
    void* memory;
    struct process_memory_handle* next;
};


struct process* allocate_process();
void switch_to_process(struct process *p);
void free_process_memory(struct process *p);