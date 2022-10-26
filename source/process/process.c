#include <kernel/process.h>
#include <kernel/alloc.h>
#include <kernel/address_space.h>
#include <kernel/register.h>
#include <init_sysregs.h>


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


void switch_to_process(struct process *p) {
    // Set the ERET jump address to the start of the application
    write_system_reg(ELR_EL1, (uint64_t)(p->process_entry_point));
    // Configure user space processor state
    write_system_reg(SP_EL0, (uint64_t)(p->process_entry_point));
    
    // This does not work, since we are not using a split address space!
    // Userspace inherits TTBR0_EL1

    // Setup page table address
    //write_system_reg(TTBR0_EL0, (uint64_t)p->addr_space->page_table);
    // System control register is always inherited from SCTLR_EL1
    // Page translation control register is always inherited from TCR_EL1

    asm volatile("eret");
}