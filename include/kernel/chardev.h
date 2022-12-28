#pragma once

#include <kernel/types.h>

extern struct char_dev global_uart;

struct char_dev {
    // Char dev interface
    int64_t (*read)(struct char_dev *dev, char *buf, int64_t count);
    int64_t (*write)(struct char_dev *dev, char *buf, int64_t count);

    // Utility data:
    char driver_str[16];
};


struct char_dev *alloc_char_dev();
int64_t read_char(struct char_dev *dev, char* buf, int64_t count);
int64_t write_char(struct char_dev *dev, void *buf, int64_t count);
// Wrapper function
int64_t write_string_char(struct char_dev *dev, char *buf);