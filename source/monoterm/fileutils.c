#include <kernel/print.h>
#include <kernel/inode.h>


int monoterm_ls(int argc, char *argv[]) {
    char *path;    
    
    if(argc == 1) {
        path = "/";
    } else if(argc == 2) {
        path = argv[1];
    } else {
        uart_print("Invalid number of arguments.\r\nUsage: ls [path]\r\n");
        return 1;
    }

    struct inode* folder = inode_from_path(g_root_inode, path);

    if(!folder) {
        uart_print("Could not resolve path.\r\n");
        return 1;
    }
    if(folder->state & INODE_TYPE_FILE) {
        term_set_color(15);
        uart_print(folder->filename);
        uart_print("\r\n");
        term_reset_font();
        return 0;
    }
    if(folder->state & INODE_STATE_NEW) {
        if(inode_read_data(folder)) {
            return 1;
        }
    }

    struct inode *it = folder->child_nodes;
    while(it) {
        if(it->state & INODE_TYPE_FILE) {
            term_set_color(15);
        } else {
            term_set_color(4);
        }
        
        uart_print(it->filename);
        uart_print("\r\n");
        term_reset_font();
        it = it->peer_nodes;
    }
    return 0;
}