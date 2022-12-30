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
    struct process *next;

    struct address_space *addr_space;
    struct process_memory_handle *allocated_list;
    struct kthread *kern_thread;
    struct kthread *user_thread;
    // The amount of memory allocated to the heap
    uint64_t user_heap_size;
    // The amount of memory used by the heap
    uint64_t user_heap_used;

    // This is the LXE equivalent to a file descriptor.
    // LXE does not have file based devices
    struct stream_descriptor* streams;
    int32_t stream_count;

    uint16_t pid;
    uint8_t state;
};


struct process_memory_handle {
    void* memory;
    struct process_memory_handle* next;
};


struct stream_descriptor {
    struct stream_descriptor* next;
    int32_t id;
    void* dev;
    uint8_t dev_type;
};


// The currently running thread
extern struct process *kernel_curr_process;
extern uint64_t global_pid_counter;
extern struct process *process_list_head;

void print_process(struct process *p);
void print_process_brief(struct process *p);
void *new_process_memory_region(struct process *p, uint64_t size);
struct address_mapping* new_mapped_process_memory(struct process *p, uint64_t target_addr, uint64_t size);
struct process* allocate_process();
int32_t process_new_stream_descriptor(struct process* p, void* dev, uint8_t dev_type);
int32_t process_entry_point_args(struct process* p, int32_t argc, char** argv);
void switch_to_user_thread();
void switch_to_process(struct process *p, bool_t terminating);
void free_process_memory(struct process *p);
void remove_process_mappings(struct process *p);
void free_process_streams(struct process *p);
void destroy_process(struct process *p);