#include <kernel/process.h>
#include <kernel/alloc.h>
#include <kernel/address_space.h>


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