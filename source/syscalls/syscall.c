#include <kernel/types.h>
#include <kernel/print.h>
#include <kernel/register.h>
#include <kernel/panic.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>
#include "write.h"
#include "close.h"
#include "sbrk.h"

#include <kernel/syscall.h>


uint64_t handle_syscall(uint64_t syscall, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    // Cast the return int to a uint64_t implicitly
    switch(syscall) {
    case __NR_exit:
        #ifdef TRACE_SYSCALLS
        print("exit({d})\n", (int)a1);
        #endif
        // The true flag indicates that we are terminating the process
        switch_to_next_process(PROCESS_STATE_TERMINATED);
        print("ERROR: Resumed exited thread!\n");
        panic();
        //return syscall_exit((int)a1);
        break;

    case __NR_write:
        #ifdef TRACE_SYSCALLS
        print("write({d}, {p}, {d})\n", (int)a1, (char*)a2, (int)a3);
        #endif
        return syscall_write((int)a1, (char*)a2, (int)a3);
        break;

    case __NR_close:
        #ifdef TRACE_SYSCALLS
        print("close({d})\n", (int)a1);
        #endif
        return syscall_close((int)a1);
        break;

    case __NR_sbrk:
        #ifdef TRACE_SYSCALLS
        print("sbrk({d})\n", (int)a1);
        #endif
        return syscall_sbrk((int)a1);
        break;

    default:
        print("Unknown syscall with number {ul}\n", syscall);
        // Exception link register (fault address)
	    print("Userspace caller address 0x{xl}\n", read_system_reg(ELR_EL1));
        panic();

    }
    return 0;
}