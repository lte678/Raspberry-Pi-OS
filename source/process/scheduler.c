#include <kernel/panic.h>
#include <kernel/process.h>
#include <kernel/print.h>
#include <kernel/scheduler.h>


void switch_to_next_process(bool_t terminating) {
    for(struct process *p = process_list_head; p; p = p->next) {
        if(p->state == PROCESS_STATE_WAITING) {
            // Execute this process
            switch_to_process(p, terminating);
        } else if(p->state == PROCESS_STATE_TERMINATED) {
            // Garbage collection is performed as we encounter terminated processes.
            destroy_process(p);
        }
    }
    print("Scheduler: No runnable process found!\n");
    panic();
}