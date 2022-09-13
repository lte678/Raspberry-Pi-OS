#pragma once

#include <kernel/types.h>

extern uint64_t* physical_kernel_pgt_addr;


int page_table_map_address(uint64_t* root_table, uint64_t pa, uint64_t va, uint64_t size);
int page_table_alignment(uint64_t address);
uint64_t page_table_block_size(int level);
uint64_t page_table_virtual_to_physical(uint64_t* table, uint64_t a);