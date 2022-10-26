#pragma once

#define unpack_from(memory, type, offset) (*(type*)((unsigned char*)memory + offset))

void memset(unsigned char val, void *buf, unsigned int size);
void memcpy(char *dest, char *src, unsigned int n);