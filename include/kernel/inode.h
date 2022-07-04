#include <kernel/block.h>

#define INODE_STATE_MASK    0x0F
#define INODE_STATE_NEW     0x00
#define INODE_STATE_VALID   0x01

#define INODE_TYPE_MASK     0xF0
#define INODE_TYPE_FILE     0x10
#define INODE_TYPE_DIR      0x20


struct inode;

struct inode_ops {
    int (*read_data)(struct inode *n);
    int (*write_data)(struct inode *n);
};


struct inode {
    struct block_dev *blk_dev;
    struct inode_ops ops;
    // File system specific data, such as the cluster number for FAT32
    void *fs_data;
    // Contains file data.
    void *data;
    unsigned int data_size;
    // Struct is only not-null for directories.
    struct inode *child_nodes;
    struct inode *parent_node;
    // For all other inodes belonging to same parent
    struct inode *peer_nodes;
    // State of inode. Is "new" before loading data
    unsigned char state;
    char filename[256];
};


struct inode *alloc_inode();
int inode_read_data(struct inode *n);
int inode_write_data(struct inode *n);
void inode_insert_child(struct inode *parent, struct inode *child);
void inode_print(struct inode *n);