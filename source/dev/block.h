/*
* Block devices model hardware that supports block-based reads.
* This includes devices such as SD cards and hard drives.
*
* Block devices can define the following interface:
*   int read_blk(unsigned int blk, void *buff);             // Returns the number of bytes read (1 block)
*   int read_blks(unsigned int blk, void *buff, int n);     // Returns the number of bytes read
*   int write_blk(unsigned int blk, void *buff);            // Returns the number of bytes written (1 block)
*   int write_blks(unsigned int blk, void *buff, int n);    // Returns the number of bytes written
*/

struct block_dev {
    int iblk;  // Block index
    int block_size;  // Size of blocks in bytes
    // Block dev interface
    int (*read_blk)(unsigned int blk, void *buf);
    int (*read_blks)(unsigned int blk, int n, void *buf);
    int (*write_blk)(unsigned int blk, void *buf);
    int (*write_blks)(unsigned int blk, int n, void *buf);

    // Utility data:
    char driver_str[16];
};

/* Allocates and registers a new block device */
// struct block_dev *alloc_block_dev() {
//    struct block_dev *dev = malloc(sizeof(struct block_dev));
//    return dev;
//}