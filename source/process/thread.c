#include <kernel/thread.h>
#include <kernel/print.h>
#include <kernel/register.h>
#include <kernel/alloc.h>
#include <kernel/process.h>


struct kthread *allocate_kthread() {
    struct kthread *thread = kmalloc(sizeof(struct kthread), ALLOC_ZERO_INIT);
    if(!thread) {
        return 0;
    }
    return thread;
}


void print_thread_context(struct kthread *t) {
    print("PSTATE:{xl}\n", t->pstate);
    print_register_block(t->registers);
}


void print_register_block(uint64_t* registers) {
    for(int i = 0; i < 15; i++) {
        int reg1 = 2*i + 0;
        int reg2 = 2*i + 1;
        if(i < 5) {
            print(" X{i}:{xl}        X{i}:{xl}\n", reg1, registers[reg1], reg2, registers[reg2]);
        } else {
            print("X{i}:{xl}       X{i}:{xl}\n", reg1, registers[reg1], reg2, registers[reg2]);
        }
        
    }
    print(" SP:{xl}        PC:{xl}\n", registers[30], registers[31]);
}