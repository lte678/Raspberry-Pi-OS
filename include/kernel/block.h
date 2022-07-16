#pragma once

#include <kernel/mem.h>
#include <kernel/alloc.h>

/*
* Block devices model hardware that supports block-based reads.
* This includes devices such as SD cards and hard drives.
*
* Block devices can define the following interface:
*   int read_blk(unsigned int blk, void *buff);             // Returns the number of bytes read (1 block)
*   int read_nblk(unsigned int blk, void *buff, n);         // Returns the number of bytes read (1 block). Limit to buffer size.
*   int read_blks(unsigned int blk, void *buff, int n);     // Returns the number of bytes read
*   int write_blk(unsigned int blk, void *buff);            // Returns the number of bytes written (1 block)
*   int write_blks(unsigned int blk, void *buff, int n);    // Returns the number of bytes written
*/

struct block_dev {
    unsigned int iblk;  // Block index
    int block_size;  // Size of blocks in bytes
    // Block dev interface
    int (*read_blk)(struct block_dev *dev, void *buf);
    int (*read_nblk)(struct block_dev *dev, void *buf, unsigned int n);
    int (*read_blks)(struct block_dev *dev, int n, void *buf);
    int (*write_blk)(struct block_dev *dev, void *buf);
    int (*write_blks)(struct block_dev *dev, int n, void *buf);
    int (*seek_blk)(struct block_dev *dev, unsigned int iblk);

    // Utility data:
    char driver_str[16];
};

struct block_dev *alloc_block_dev();

int read_blk(struct block_dev *dev, void *buf);
int read_nblk(struct block_dev *dev, void *buf, unsigned int n);
int read_blks(struct block_dev *dev, int n, void *buf);
int write_blk(struct block_dev *dev, void *buf);
int write_blks(struct block_dev *dev, int n, void *buf);
int seek_blk(struct block_dev *dev, unsigned int iblk);