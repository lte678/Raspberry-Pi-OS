#include <kernel/inode.h>
#include <kernel/print.h>


struct inode *alloc_inode() {
    // TODO: Use slab allocator
    struct inode *n = (struct inode*)kmalloc(sizeof(struct inode), ALLOC_ZERO_INIT);
    n->state = INODE_STATE_NEW;
    return n;
}

int inode_read_data(struct inode *n) {
    if(!n->ops.read_data) {
        return -1;
    }
    return n->ops.read_data(n);
}

int inode_write_data(struct inode *n) {
    if(!n->ops.write_data) {
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

void inode_print(struct inode *n) {
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