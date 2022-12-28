#include <kernel/process.h>
#include <kernel/alloc.h>
#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/address_space.h>
#include <kernel/register.h>
#include <kernel/print.h>
#include <kernel/panic.h>
#include <kernel/chardev.h>
#include <kernel/device_types.h>
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

    // Open the default file descriptors
    // TODO: Check for errors
    process_new_stream_descriptor(p, &global_uart, DEVICE_TYPE_CHAR);
    process_new_stream_descriptor(p, &global_uart, DEVICE_TYPE_CHAR);
    process_new_stream_descriptor(p, &global_uart, DEVICE_TYPE_CHAR);

    return p;
}


int32_t process_new_stream_descriptor(struct process* p, void* dev, uint8_t dev_type) {
    struct stream_descriptor *new_stream = (struct stream_descriptor*)kmalloc(sizeof(struct stream_descriptor), 0);
    if(!new_stream) {
        return 1;
    }
    new_stream->dev = dev;
    new_stream->dev_type = dev_type;
    new_stream->id = p->stream_count;
    p->stream_count++;
    
    // Insert into linked list
    new_stream->next = p->streams;
    p->streams = new_stream;

    return 0;
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


void remove_process_mappings(struct process *p) {
    struct address_mapping *i = p->addr_space->mappings;
    while(i) {
        if(unmap_memory_region(p->addr_space, i)) {
            print("Unable to unmap memory region, kernel address space has been corrupted!\n");
            panic();
            return;
        }
        i = i->next;
    }
}


void free_process_streams(struct process *p) {
    struct stream_descriptor *i = p->streams;
    while(i) {
        struct stream_descriptor *next_i = i->next;
        free(i);
        p->stream_count--;
        i = next_i;
    }
}

/**
 * @brief Call on terminated processes. Removes all allocations, mappings and frees structs.
 * 
 * @param p The process to terminate.
 */
void destroy_process(struct process *p) {
    free_process_memory(p);
    remove_process_mappings(p);
    free_process_streams(p);
    free(p->kern_thread);
    free(p->user_thread);
    struct address_mapping *i = p->addr_space->mappings;
    while(i) {
        struct address_mapping *next_i = i->next;
        free(i);
        i = next_i;
    }
    free(p->addr_space);
}


void switch_to_user_thread() {
    #ifdef DEBUG_THREADING
    print("Switching to user thread\n");
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
    #ifdef DEBUG_THREADING
    print("Switching to process with PID {d}\n", p->pid);
    #endif

     // Maintain changes in process structs
    //print_thread_context(kernel_curr_process->kern_thread);
    if(terminating) {
        kernel_curr_process->state = PROCESS_STATE_TERMINATED;
    } else {
        kernel_curr_process->state = PROCESS_STATE_WAITING;
    }

    struct process *prev_proc = kernel_curr_process;
    kernel_curr_process = p;
    kernel_curr_process->state = PROCESS_STATE_RUNNING;

    switch_context(prev_proc->kern_thread, kernel_curr_process->kern_thread);
}


void *new_process_memory_region(struct process *p, uint64_t size) {
    // Allocate memory. This returns a kernel virtual address.
    void* header_memory = kmalloc(size, ALLOC_ZERO_INIT | ALLOC_PAGE_ALIGN);
    if(!header_memory) {
        print("Failed to allocate process memory!\n");
        return 0;
    }

    struct process_memory_handle *al = kmalloc(sizeof(struct process_memory_handle), 0);
    if(!al) {
        free(header_memory);
        free_process_memory(p);
        return 0;
    }
    struct process_memory_handle *old_al = p->allocated_list;
    al->next = old_al;
    al->memory = header_memory;
    p->allocated_list = al;
    return header_memory;
}


void *new_mapped_process_memory(struct process *p, uint64_t target_addr, uint64_t size) {
    // Append memory region to the processes allocation lists
    void* new_mem = new_process_memory_region(p, size);
    if(!new_mem) {
        return 0;
    }

    if(map_memory_region_virt(p->addr_space, target_addr, (uint64_t)new_mem, size)) {
        return 0;
    }
    return new_mem;
}


/**
 * @brief Prints all process information
 * 
 * @param p the process
 */
void print_process(struct process *p) {
    print("--- Process with PID {d} ---\n", p->pid);
    const char* state = "unknown";
    switch(p->state) {
    case PROCESS_STATE_NEW:
        state = "new";
        break;
    case PROCESS_STATE_RUNNING:
        state = "running";
        break;
    case PROCESS_STATE_TERMINATED:
        state = "terminated";
        break;
    case PROCESS_STATE_WAITING:
        state = "waiting";
        break;
    }
    print("State      :{s}\n", state);

    print("User Thread\n");
    print("  PC       :0x{xl}\n", p->user_thread->link_reg);
    print("  Stack    :0x{xl}\n", p->user_thread->sp);
    print("  Stk Size :0x{xl}\n", p->user_thread->stack_size);

    print("Kernel Thread\n");
    print("  PC       :0x{xl}\n", p->kern_thread->link_reg);
    print("  Stack    :0x{xl}\n", p->kern_thread->sp);
    print("  Stk Size :0x{xl}\n", p->kern_thread->stack_size);

    uint32_t nr_of_allocations = 0;
    for(struct process_memory_handle *m = p->allocated_list; m; m=m->next) {
        nr_of_allocations++;
    }
    print("Allocations:{d}\n", nr_of_allocations);

    print("Mappings\n");
    for(struct address_mapping *m = p->addr_space->mappings; m; m=m->next) {
        print("  0x{xl} to 0x{xl}\n", m->vaddress, m->paddress);
    }
}