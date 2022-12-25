#pragma once

#define __NR_exit    1
#define __NR_fstat   2
#define __NR_read    3
#define __NR_write   4
#define __NR_getpid  5
#define __NR_open    6
#define __NR_close   7
#define __NR_notimpl 255


uint64_t handle_syscall(uint64_t syscall, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6);