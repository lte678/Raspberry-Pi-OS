#include <kernel/print.h>
#include <kernel/process.h>
#include "bindings.h"

int monoterm_ps(int argc, char *argv[]) {
    struct process *p = process_list_head;
    while(p) {
        print_process_brief(p);
        p = p->next;
    }
    return 0;
}