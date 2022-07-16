#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/inode.h>


int monoterm_ls(int argc, char *argv[]) {
    char *path;    
    
    if(argc == 1) {
        path = "/";
    } else if(argc == 2) {
        path = argv[1];
    } else {
        print("Invalid number of arguments.\r\nUsage: ls [path]\r\n");
        return 1;
    }

    struct inode* folder = inode_from_path(g_root_inode, path);

    if(!folder) {
        print("Could not resolve path.\r\n");
        return 1;
    }
    if(folder->state & INODE_TYPE_FILE) {
        term_set_color(15);
        print("{s}\r\n", folder->filename);
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
        
        // TODO: Width specifier
        print("{s}", it->filename);
        for(int i = 20 - strlen(it->filename); i > 0; i--) {
            print(" ");
        }
        print("{u}B\r\n", it->data_size);
        term_reset_font();
        it = it->peer_nodes;
    }
    return 0;
}