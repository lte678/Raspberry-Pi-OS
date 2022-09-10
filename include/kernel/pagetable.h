#pragma once

#include <kernel/types.h>


int page_table_map_address(uint64_t* root_table, uint64_t pa, uint64_t va, uint64_t size);
int page_table_alignment(uint64_t address);
uint64_t page_table_block_size(int level);