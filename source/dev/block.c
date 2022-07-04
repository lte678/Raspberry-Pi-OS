#include <kernel/block.h>


/* Allocates and registers a new block device */
struct block_dev *alloc_block_dev() {
    struct block_dev *dev = (struct block_dev*)kmalloc(sizeof(struct block_dev), ALLOC_ZERO_INIT);
    return dev;
}

int read_blk(struct block_dev *dev, void *buf) {
    if(!dev->read_blk) {
        return -1;
    }
    return dev->read_blk(dev, buf);
}

int read_nblk(struct block_dev *dev, void *buf, unsigned int n) {
    if(!dev->read_nblk) {
        return -1;
    }
    return dev->read_nblk(dev, buf, n);
}

int read_blks(struct block_dev *dev, int n, void *buf) {
    if(!dev->read_blks) {
        return -1;
    }
    return dev->read_blks(dev, n, buf);
}

int write_blk(struct block_dev *dev, void *buf) {
    if(!dev->write_blk) {
        return -1;
    }
    return dev->write_blk(dev, buf);
}

int write_blks(struct block_dev *dev, int n, void *buf) {
    if(!dev->write_blks) {
        return -1;
    }
    return dev->write_blks(dev, n, buf);
}

int seek_blk(struct block_dev *dev, unsigned int iblk) {
    if(!dev->seek_blk) {
        return -1;
    }
    return dev->seek_blk(dev, iblk);
}