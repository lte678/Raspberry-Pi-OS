#pragma once
#include <kernel/types.h>

#define read_system_reg(reg) ({ uint64_t ret; __asm__ ("mrs %0, " #reg ";" : "=r"(ret) : : ); ret; })

extern void put32(uint64_t addr, uint32_t data);
extern uint32_t get32(uint64_t addr);