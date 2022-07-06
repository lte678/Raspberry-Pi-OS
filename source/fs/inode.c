#include <kernel/inode.h>
#include <kernel/print.h>
#include <kernel/string.h>


struct inode* g_root_inode = 0;


struct inode *alloc_inode() {
    // TODO: Use slab allocator
    struct inode *n = (struct inode*)kmalloc(sizeof(struct inode), ALLOC_ZERO_INIT);
    n->state = INODE_STATE_NEW;
    return n;
}

int inode_read_data(struct inode *n) {
    if(!n->ops.read_data) {
        uart_print("Error: ops.read_data not defined\r\n");
        return -1;
    }
    return n->ops.read_data(n);
}

int inode_write_data(struct inode *n) {
    if(!n->ops.write_data) {
        uart_print("Error: ops.write_data not defined\r\n");
        return -1;
    }
    return n->ops.write_data(n);
}

void inode_insert_child(struct inode *parent, struct inode *child) {
    if(!parent->child_nodes) {
        parent->child_nodes = child;
    } else {
        struct inode *old_child = parent->child_nodes;
        parent->child_nodes = child;
        child->peer_nodes = old_child;
    }
}

static struct inode *search_children(struct inode *n, char *filename) {
    struct inode *it = n->child_nodes;
    while(it) {
        if(!strcmp(filename, it->filename)) {
            return it;
        }
        it = it->peer_nodes;
    }
    return 0;
}

/*
 * Finds the inode corresponding to the path.
 * Requires the node to start searching from (only relevant for relative paths).
 */
struct inode *inode_from_path(struct inode *root, char *path) {
    struct inode *curr_node = root;
    int path_index = 0;
    // Start at current or root node.
    if(path[0] == '/') {
        while(curr_node->parent_node) {
            curr_node = curr_node->parent_node;
        }
        path_index++;
    }

    // If its just a /, we dont need to do a lookup
    if(path[1] == 0 || path[0] == 0) {
        return curr_node;
    }

    // Start search
    for(int i = 0; i < 255; i++) {
        char token[256];
        // Find next slash
        int next_slash = path_index;
        while(path[next_slash] != '/' && path[next_slash] != 0) {
            next_slash++;
        }

        strncpy(token, path + path_index, next_slash - path_index);
        token[next_slash - path_index] = 0;

        if(curr_node->state & INODE_STATE_NEW) {
            if(inode_read_data(curr_node)) {
                return 0;
            }
        }
        curr_node = search_children(curr_node, token);
        if(!curr_node) {
            // Path could not be resolved.
            return 0;
        }

        if(path[next_slash] == 0) {
            // Final token. No trailing slash
            return curr_node;
        } else if(path[next_slash + 1] == 0) {
            // Final token. Trailing slash
            if(curr_node->state & INODE_TYPE_FILE) {
                return 0;
            }
            return curr_node;
        } 
        // else {
        //     Intermediate token
        // }
        
        path_index = next_slash + 1;
    }
    // Search failed after 255 levels.
    uart_print("Exceeded maximum inode depth!\r\n");
    return 0;
}

void inode_print(struct inode *n) {
    if(!n) {
        uart_print("Inode pointer invalid\r\n");
        return;
    }

    uart_print("Inode\r\n");
    uart_print("  State:    ");
    if((n->state & INODE_STATE_MASK) == INODE_STATE_NEW) {
        uart_print("new ");
    }
    if((n->state & INODE_STATE_MASK) == INODE_STATE_VALID) {
        uart_print("valid ");
    }
    if((n->state & INODE_TYPE_MASK) == INODE_TYPE_DIR) {
        uart_print("directory ");
    }
    if((n->state & INODE_TYPE_MASK) == INODE_TYPE_FILE) {
        uart_print("file ");
    }
    uart_print("\r\n");

    uart_print("  Filename: ");
    uart_print(n->filename);
    uart_print("\r\n");

    uart_print("  Size:     ");
    print_uint(n->data_size);
    uart_print("\r\n");
}