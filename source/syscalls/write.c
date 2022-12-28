#include <kernel/print.h>
#include <kernel/device_types.h>
#include <kernel/chardev.h>
#include <kernel/process.h>

#include "write.h"


int syscall_write(int file, char *ptr, int len) {
    struct stream_descriptor *i = kernel_curr_process->streams;
    while(i) {
        if(i->id == file) {
        switch(i->dev_type) {
        case DEVICE_TYPE_CHAR:
            write_char(i->dev, ptr, len);
            return len;
        default:
            print("Cannot write to non-char devices!\n");
        }

        }
        i = i->next;
    }
    return 0;
}