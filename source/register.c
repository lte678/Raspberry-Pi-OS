#include <kernel/types.h>

void put32(uint64_t addr, uint32_t data) {
    *((uint32_t*)addr) = data; 
}

uint32_t get32(uint64_t addr) {
    return *((uint32_t*)addr); 
}