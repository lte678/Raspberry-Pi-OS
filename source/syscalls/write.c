#include <kernel/print.h>

#include "write.h"


static void print_user_string(char *ptr, int len) {
    // We can resolve userspace addresses, since the relevant pages will still be mapped currently
    print(ptr);
}


int syscall_write(int file, char *ptr, int len) {
    switch(file) {
    case 1:
        // STDOUT
        print_user_string(ptr, len);
        return len;
    default:
        print("syscall:write: Invalid file descriptor\r\n");
        return 0;
    }
}