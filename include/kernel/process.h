#pragma once

#include <kernel/address_space.h>


struct process {
    uint16_t pid;
    struct address_space *addr_space;
};


struct process* allocate_process();