#include <kernel/print.h>

#include "close.h"


int syscall_close(int fd) {
    // Streams cannot be closed. This is a no-op in our os currently.
    return 0;
}