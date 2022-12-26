#include <kernel/process.h>
#include <kernel/print.h>
#include <kernel/panic.h>
#include "sbrk.h"


uint64_t syscall_sbrk(int incr) {
    struct process *p = kernel_curr_process;

    uint64_t user_addr = USERSPACE_HEAP_ADDRESS + p->user_heap_used;
    int64_t required_space = p->user_heap_used + incr - p->user_heap_size;
    if(required_space > 0) {
        // print("sbrk requires {l} more bytes.\r\n", required_space);
        required_space = round_up_to_page(required_space);
        void* new_mem = new_mapped_process_memory(p, user_addr, required_space);
        if(!new_mem) {
            print("Failed to allocate new process memory.\r\n");
            // TODO: Just cause a fault
            panic();
        }
        p->user_heap_size += required_space;
    }
    p->user_heap_used += incr;

    return user_addr;
}