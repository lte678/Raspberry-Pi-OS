#ifndef MEM_H
#define MEM_H

void memset(unsigned char val, void *buf, unsigned int size) {
    unsigned int i;
    for(i = 0; i < size; i++) {
        *(char*)(buf + i) = val;
    }
}

#endif // MEM_H