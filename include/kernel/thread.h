#pragma once

#include <kernel/types.h>


struct kthread {
    // BEGIN: Ordered members for asm access
    // Contains registers x0 to x29
    uint64_t registers[30];
    // Stack pointer
    uint64_t sp;
    // Equivalent to PC of the running thread
    uint64_t link_reg;
    // The processor state
    uint64_t pstate;
    // END: Ordered members

    // This may not be necessary, but lets keep track of the thread stack size
    uint64_t stack_size;
};


struct kthread *allocate_kthread();

void switch_context(struct kthread *from, struct kthread *to);
void load_user_context(struct kthread *to);
void print_thread_context(struct kthread *t);
void switch_to_thread(struct kthread *t);
