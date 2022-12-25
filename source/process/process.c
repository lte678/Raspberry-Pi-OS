#include <kernel/process.h>
#include <kernel/alloc.h>
#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/address_space.h>
#include <kernel/register.h>
#include <kernel/print.h>
#include <init_sysregs.h>


/**
 * @brief The currently running process.
 * 
 */
struct process *kernel_curr_process;
uint64_t global_pid_counter = 1;
struct process *process_list_head;

/**
 * @brief Allocates and returns a process struct.
 */
struct process* allocate_process() {
    struct process *p = kmalloc(sizeof(struct process), ALLOC_ZERO_INIT);
    if(!p) {
        return 0;
    }
    p->addr_space = allocate_address_space();
    if(!p->addr_space) {
        free(p);
        return 0;
    }
    p->kern_thread = allocate_kthread();
    if(!p->kern_thread) {
        free(p->addr_space);
        free(p);
        return 0;
    }
    p->user_thread = allocate_kthread();
    if(!p->user_thread) {
        free(p->kern_thread);
        free(p->addr_space);
        free(p);
        return 0;
    }

    p->user_thread->sp = USERSPACE_STACK_ADDRESS;

    p->pid = global_pid_counter;
    global_pid_counter++;

    return p;
}


/**
 * @brief Frees the process_memory_handle structs and their contents.
 * Is used to deallocate all the process associated memory.
 * 
 * @param p Process
 */
void free_process_memory(struct process *p) {
    struct process_memory_handle *i = p->allocated_list;
    while(i) {
        free(i->memory);
        struct process_memory_handle *next_i = i->next;
        free(i);
        i = next_i;
    }
}


void switch_to_user_thread() {
    #ifdef DEBUG_THREADING
    print("Switching to user thread\r\n");
    #endif
    // Set the ERET jump address to the start of the application
    write_system_reg(ELR_EL1, (uint64_t)(kernel_curr_process->user_thread->link_reg));
    // Configure user space processor state
    write_system_reg(SP_EL0, (uint64_t)(kernel_curr_process->user_thread->sp));
    
    // This does not work, since we are not using a split address space!
    // Userspace inherits TTBR0_EL1

    // Setup page table address
    //write_system_reg(TTBR0_EL0, (uint64_t)p->addr_space->page_table);
    // System control register is always inherited from SCTLR_EL1
    // Page translation control register is always inherited from TCR_EL1

    asm volatile("eret");
}

/**
 * @brief Stores the current process context, and resumes the supplied process.
 * 
 * @param p The process to switch to.
 * @param terminating True if the current process should be terminated.
 */
void switch_to_process(struct process *p, bool_t terminating) {
    // Store old context
    store_context(kernel_curr_process->kern_thread);
    kernel_curr_process->kern_thread->link_reg = (uint64_t)__builtin_return_address(0);

    //print_thread_context(kernel_curr_process->kern_thread);
    if(terminating) {
        kernel_curr_process->state = PROCESS_STATE_TERMINATED;
    } else {
        kernel_curr_process->state = PROCESS_STATE_WAITING;
    }
    
    #ifdef DEBUG_THREADING
    print("Switching to process with PID {d}\r\n", p->pid);
    #endif

    // Load new context
    kernel_curr_process = p;
    kernel_curr_process->state = PROCESS_STATE_RUNNING;
    //write_reg("pstate", kernel_curr_process->kern_thread->pstate);
    load_context(kernel_curr_process->kern_thread);
}