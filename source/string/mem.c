
// Kernel memory utility functions

#include <kernel/mem.h>

void memset(unsigned char val, void *buf, unsigned int size) {
    unsigned int i;
    for(i = 0; i < size; i++) {
        *(char*)(buf + i) = val;
    }
}

void memcpy(char *dest, char *src, unsigned int n) {
    unsigned int i = 0;
    while(i < n) {
        *dest = *src;
        dest++;
        src++;
        i++;
    }
}