#include <kernel/print.h>
#include <kernel/stacktrace.h>
#include <kernel/process.h>
#include "bindings.h"


static void list_processes() {
    struct process *p = process_list_head;
    while(p) {
        print_process_brief(p);
        p = p->next;
    }
}


static int dump_process(char *pid_str) {
    int32_t pid;
    if(atoi(pid_str, &pid)) {
        print("PID must be a number\n");
        return 1;
    }

    struct process *p = process_list_head;
    while(p) {
        if(p->pid == (uint16_t)pid) {
            print_process(p);
            print("Kernel registers\n");
            print_thread_context(p->kern_thread);
            print("User registers\n");
            if(p->exception_stack_pointer) {
                print_register_block(p->exception_stack_pointer);
            }
            return 0;
        }
        p = p->next;
    }
    print("No process with PID {i}\n", pid);
    return 1;
}


static int stacktrace(char *pid_str) {
    int32_t pid;
    if(atoi(pid_str, &pid)) {
        print("PID must be a number\n");
        return 1;
    }

    struct process *p = process_list_head;
    while(p) {
        if(p->pid == (uint16_t)pid) {
            user_process_print_stacktrace(p);
            return 0;
        }
        p = p->next;
    }
    print("No process with PID {i}\n", pid);
    return 1;
}


int monoterm_ps(int argc, char *argv[]) {
    if(argc == 1) {
        list_processes();
        return 0;
    } else if(argc == 3) {
        if(strcmp(argv[1], "dump") == 0) {
            return dump_process(argv[2]);
        } else if(strcmp(argv[1], "stacktrace") == 0) {
            return stacktrace(argv[2]);
        } else {
            print("Unknown operation \"{s}\"\n", argv[1]);
        }
    } else {
        print("Usage: ps [dump {pid} | stacktrace {pid}]\n");
    }
    return 1;
}