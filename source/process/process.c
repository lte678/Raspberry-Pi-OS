#include <kernel/process.h>
#include <kernel/alloc.h>
#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/address_space.h>
#include <kernel/register.h>
#include <kernel/print.h>
#include <kernel/panic.h>
#include <kernel/chardev.h>
#include <kernel/pagetable.h>
#include <kernel/mem.h>
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

    // Put into global process list
    p->next = process_list_head;
    process_list_head = p;

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


int32_t process_entry_point_args(struct process* p, int32_t argc, char** argv) {
    if(p->state != PROCESS_STATE_NEW) {
        print("process_entry_point_args: process {d} must be in state 'new'\n", p->pid);
        return 1;
    }

    // The array of pointers, and the strings are pushed onto the stack
    // Bottom of stack -> char pointers -> char strings
    uint32_t argv_len = argc * 8;
    for(int i = 0; i < argc; i++) {
        argv_len += strlen(argv[i]) + 1; // 1 byte for 0 terminator
    }
    // 16 byte alignment
    argv_len = stack_aligned(argv_len);
    // Put argv and strings onto the stack
    p->user_thread->sp -= argv_len;
    // Address of stack in kernel space. It is not currently mapped
    void *stack_address = PHYS_TO_KERN(address_space_virtual_to_physical(p->addr_space, (void*)p->user_thread->sp));

    // Copy the strings and addresses to the stack
    uint64_t current_str_offset = 0;
    for(int i = 0; i < argc; i++) {
        // sp + argc*8 is the beginning of the strings section
        // Put address on stack
        ((uint64_t*)stack_address)[i] = (uint64_t)((char*)(p->user_thread->sp) + argc * 8 + current_str_offset);
        // Put string on stack
        uint64_t str_len = strlen(argv[i]) + 1;
        memcpy((char*)stack_address + argc*8 + current_str_offset, argv[i], str_len);
        current_str_offset += str_len;
    }

    // Insert into user context
    memcpy((char*)&p->user_thread->registers[0], (char*)&argc, 4);
    // argv pointed to the parameters in kernel space. The user stack pointer now points to it in user space.
    p->user_thread->registers[1] = p->user_thread->sp;
    return 0;
}


/**
 * @brief Frees the process_memory_handle structs and their contents.
 * Is used to deallocate all the process associated memory.
 * 
 * @param p Process
 */
void free_process_memory(struct process *p) {
    while(p->allocated_list) {
        free(p->allocated_list->memory);
        struct process_memory_handle *next = p->allocated_list->next;
        free(p->allocated_list);
        p->allocated_list = next;
    }
}


void remove_process_mappings(struct process *p) {
    while(p->addr_space->mappings) {
        if(unmap_and_remove_memory_region(p->addr_space, p->addr_space->mappings)) {
            print("Unable to unmap memory region, kernel address space has been corrupted!\n");
            panic();
            return;
        }
    }
}


void restore_process_mappings(struct process *p) {
    struct address_mapping *i = p->addr_space->mappings;
    while(i) {
        if(map_memory_region(p->addr_space, i)) {
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
    free(p->addr_space);

    // Remove process from linked list
    struct process *i_prev = 0;
    struct process *i = process_list_head;
    while(i) {
        if(i == p) {
            // Found the process
            if(i_prev) {
                i_prev->next = i->next;
            } else {
                process_list_head = i->next;
            }
            free(i);
            return;
        }
        i_prev = i;
        i = i->next;
    }
    print("destroy_process: warning: process not found in process list.\n");
}


void switch_to_user_thread() {
    #ifdef DEBUG_THREADING
    print("Switching to user thread\n");
    #endif
    // Set the ERET jump address to the start of the application
    write_system_reg(ELR_EL1, (uint64_t)(kernel_curr_process->user_thread->link_reg));
    // Configure user space processor state
    write_system_reg(SP_EL0, (uint64_t)(kernel_curr_process->user_thread->sp));

    // Load the userspace thread state
    load_user_context(kernel_curr_process->user_thread);
}

/**
 * @brief Stores the current process context, and resumes the supplied process.
 * 
 * @param p The process to switch to.
 * @param terminating True if the current process should be terminated.
 */
void switch_to_process(struct process *p, bool_t terminating) {
    // Unmap user mappings. This is equivalent to all process specific mappings.
    // This call only sets the adress_mapping to unused and removes it from the global page table.
    remove_process_mappings(kernel_curr_process);

    // Add new user mappings to page table
    restore_process_mappings(p);

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


struct address_mapping* new_mapped_process_memory(struct process *p, uint64_t target_addr, uint64_t size) {
    // Append memory region to the processes allocation lists
    void* new_mem = new_process_memory_region(p, size);
    if(!new_mem) {
        return 0;
    }
    return create_memory_region_virt(p->addr_space, target_addr, (uint64_t)new_mem, size);
}


static const char* process_state_string(struct process *p) {
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
    return state;
}


/**
 * @brief Prints all process information
 * 
 * @param p the process
 */
void print_process(struct process *p) {
    print("--- Process with PID {d} ---\n", p->pid);

    print("State      :{s}\n", process_state_string(p));

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


/**
 * @brief Prints brief process information
 * 
 * @param p the process
 */
void print_process_brief(struct process *p) {
    print("Process with PID {d}, state [{s}]\n", p->pid, process_state_string(p));
}