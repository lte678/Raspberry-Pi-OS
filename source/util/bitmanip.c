#include <kernel/bitmanip.h>

uint32_t byte_swap32(uint32_t x) {
    return (x << 24) | ((x << 8) & 0xFF0000) | ((x >> 8) & 0xFF00) | (x >> 24);
}

uint64_t byte_swap64(uint64_t x) {
    return ((uint64_t)byte_swap32(x & 0xFFFFFFFF) << 32) | byte_swap32(x >> 32);
}