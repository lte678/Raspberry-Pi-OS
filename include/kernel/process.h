#pragma once

#include <kernel/types.h>
#include <kernel/page.h>
#include <kernel/thread.h>
#include <kernel/address_space.h>


// Kernel address: 0x0000800000000000
#define USERSPACE_STACK_ADDRESS             0x0000010000000000
#define USERSPACE_HEAP_ADDRESS              0x0000020000000000
#define USERSPACE_PREALLOCATED_STACK_SIZE   PAGE_SIZE
#define KERNEL_STACK_SIZE                   PAGE_SIZE*4

#define PROCESS_STATE_NEW        0
#define PROCESS_STATE_RUNNING    1
#define PROCESS_STATE_WAITING    2
#define PROCESS_STATE_TERMINATED 3


struct process {
    struct process *prev;
    struct process *next;

    struct address_space *addr_space;
    struct process_memory_handle *allocated_list;
    struct kthread *kern_thread;
    struct kthread *user_thread;
    // The amount of memory allocated to the heap
    uint64_t user_heap_size;
    // The amount of memory used by the heap
    uint64_t user_heap_used;
    uint16_t pid;
    uint8_t state;
};


struct process_memory_handle {
    void* memory;
    struct process_memory_handle* next;
};

// The currently running thread
extern struct process *kernel_curr_process;
extern uint64_t global_pid_counter;
extern struct process *process_list_head;

void print_process(struct process *p);
void *new_process_memory_region(struct process *p, uint64_t size);
void *new_mapped_process_memory(struct process *p, uint64_t target_addr, uint64_t size);
struct process* allocate_process();
void switch_to_user_thread();
void switch_to_process(struct process *p, bool_t terminating);
void free_process_memory(struct process *p);
void remove_process_mappings(struct process *p);
void destroy_process(struct process *p);