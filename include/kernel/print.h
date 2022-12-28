#ifndef PRINT_H
#define PRINT_H

#include <kernel/string.h>
#include <kernel/types.h>

// Supports 0-10 arguments
#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)
#define numargs(...) VA_NARGS_IMPL(_, ## __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define print(format_string, ...)  _print(format_string, numargs(__VA_ARGS__), ##__VA_ARGS__)

void _print(char *fstring, int numargs, ...);

void print_int(int number);
void print_uint(unsigned int number);
void print_long(long number);
void print_ulong(unsigned long number);
void print_hex(unsigned char* bytes, unsigned short n);
void print_hex_be(unsigned char* bytes, unsigned short n);
void print_hex_uint32(unsigned int num);
void print_hex_uint64(uint64_t num);
void print_address(void* addr);
// extern void print_uint(unsigned int number);

#endif  // PRINT_H