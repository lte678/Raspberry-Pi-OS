#pragma once
#include <kernel/types.h>

#define read_system_reg(reg) ({ uint64_t ret; asm volatile ("mrs %0, " #reg ";" : "=r"(ret) : : ); ret; })
#define write_system_reg(reg, val) ({ uint64_t _val = val; asm volatile ("msr " #reg ", %0;" : : "r"(_val) : ); })

#define read_stack_pointer() ({ uint64_t ret; asm volatile ("mov %0, sp;" : "=r"(ret) : : ); ret; })

void put32(uint64_t addr, uint32_t data);
uint32_t get32(uint64_t addr);