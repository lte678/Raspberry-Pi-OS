#include <kernel/print.h>

#include "close.h"


int syscall_close(int fd) {
    print("TODO: Closing fd {d}\r\n", fd);
    return 0;
}