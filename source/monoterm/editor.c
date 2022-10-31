#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/inode.h>


int monoterm_edit(int argc, char *argv[]) {
    char *path;    
    if(argc == 2) {
        path = argv[1];
    } else {
        print("Invalid number of arguments.\r\nUsage: edit [file]\r\n");
        return 1;
    }

    struct inode* file = inode_from_path(g_root_inode, path);
    if(!file) {
        print("Could not resolve path.\r\n");
        return 1;
    }
    if(!inode_is_file(file)) {
        print("{s} is not a file\r\n", file->filename);
        return 1;
    }

    char* line_buffer = kmalloc(file->data_size, 0);
    if(inode_read(file, line_buffer, 0) != file->data_size) {
        print("Failed to read entire contents of file!\r\n");
        free(line_buffer);
        return 1;
    }
    term_clear();
    char* to_print = line_buffer;
    for(int i = 0; i < term_getrows(); i++) {
        term_set_cursor(0, i);
        while(*to_print != 0 && *to_print != '\n') {
            print("{c}", *to_print);
            to_print++;
        }

        if(*to_print == '\n') {
            to_print++;
        } else {
            break;
        }        
    }

    while(1) {
        char new_char = uart_recv();
    }

    free(line_buffer);
    return 0;
}