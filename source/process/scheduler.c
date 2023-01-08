#include <kernel/panic.h>
#include <kernel/process.h>
#include <kernel/print.h>
#include <kernel/scheduler.h>


/**
 * @brief Yield. Runs the next scheduled process. Can be used to terminate processes.
 * 
 * @param new_state The state of this process after yielding.
 */
void switch_to_next_process(uint8_t new_state) {
    for(struct process *p = process_list_head; p; p = p->next) {
        if(p->state == PROCESS_STATE_WAITING) {
            // Execute this process
            switch_to_process(p, new_state);
        } else if(p->state == PROCESS_STATE_TERMINATED) {
            // Garbage collection is performed as we encounter terminated processes.
            destroy_process(p);
        }
    }
    print("Scheduler: No runnable process found!\n");
    panic();
}


/**
 * @brief Yield. Runs the next scheduled process. Process is set to state waiting to resume later.
 */
void yield() {
    switch_to_next_process(PROCESS_STATE_WAITING);
}