#include <kernel/chardev.h>
#include <kernel/alloc.h>
#include <kernel/string.h>


struct char_dev *alloc_char_dev() {
    struct char_dev *dev = (struct char_dev*)kmalloc(sizeof(struct char_dev), ALLOC_ZERO_INIT);
    return dev;
}

int64_t read_char(struct char_dev *dev, char* buf, int64_t count) {
    if(!dev->read) {
        return -1;
    }
    return dev->read(dev, buf, count);
}

int64_t write_char(struct char_dev *dev, void *buf, int64_t count) {
    if(!dev->write) {
        return -1;
    }
    return dev->write(dev, buf, count);
}

int64_t write_string_char(struct char_dev *dev, char *buf) {
    return write_char(dev, buf, strlen(buf));
}