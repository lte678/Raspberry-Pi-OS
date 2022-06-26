#pragma once

#include <kernel/block.h>


int sd_initialize(struct block_dev *dev);

// "sd" command
int monoterm_sd(int argc, char *argv[]);