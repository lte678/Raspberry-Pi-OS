#pragma once


#include <kernel/block.h>

#define INODE_STATE_MASK    0x0F
#define INODE_STATE_NEW     0x01
#define INODE_STATE_VALID   0x02

#define INODE_TYPE_MASK     0xF0
#define INODE_TYPE_FILE     0x10
#define INODE_TYPE_DIR      0x20


struct inode;

struct inode_ops {
    int (*fetch_data)(struct inode *n);
    int (*push_data)(struct inode *n);
};


struct inode {
    struct block_dev *blk_dev;
    struct inode_ops ops;
    // File system specific data, such as the cluster number for FAT32
    void *fs_data;
    // Contains file data.
    void *data;
    // Points to the byte in the file we are reading next
    uint64_t seek_address;
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
int inode_fetch_data(struct inode *n);
int inode_write_data(struct inode *n);
int inode_read(struct inode *node, void* dest, uint32_t n);
int inode_is_file(struct inode *node);
void inode_insert_child(struct inode *parent, struct inode *child);
struct inode *inode_from_path(struct inode *root, char *path);
void inode_print(struct inode *n);

extern struct inode* g_root_inode;