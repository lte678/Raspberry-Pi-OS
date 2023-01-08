#include <kernel/print.h>
#include <kernel/stacktrace.h>


void user_process_print_stacktrace(struct process* p) {
    if(!p->exception_stack_pointer) {
        // We do not currently have access to the user registers
        print("Stacktrace failed, registers not accessible\n");
        return;
    }

    // Put the user mappings into the kernel so we can access the stack
    restore_process_mappings(p);

    print("Stacktrace:\n");
    print("  [{xl}]\n", p->exception_stack_pointer[30] - 4);
    uint64_t stack_frame = p->exception_stack_pointer[29];
    while(stack_frame) {
        if(*((uint64_t*)stack_frame + 1)) {
            print("  [{xl}]\n", *((uint64_t*)stack_frame + 1) - 4);
        }
        //print("Caller stackframe: {xl}\n", stack_frame);
        stack_frame = *(uint64_t*)stack_frame;
    }
    print("  [--top of stack--]\n");

    remove_process_mappings(p);
}