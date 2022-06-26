#include <kernel/block.h>


struct inode {
    struct block_dev *blk_dev;
    unsigned int blk_index;
    void *data;
    // Struct is only not-null for directories.
    struct inode *child_nodes;
    // For all other inodes belonging to same parent
    struct inode *peer_nodes;
    char *filename;
}