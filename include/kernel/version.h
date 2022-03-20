#ifndef VERSION_H
#define VERSION_H

#include <kernel/string.h>

/*
 * VERSION HISTORY
 * v0.2: Added buddy block allocator
 * v0.3: Buddy block allocator rewrite
 */

#define MAJOR_VERSION 0
#define MINOR_VERSION 3

int version_str(char *buf, unsigned int n) {
    int written;
    int tmp;
    
    written = itos(MAJOR_VERSION, buf, n);
    if(written < 0) {
        return 1;
    }

    if(n > written) {
        buf[written] = '.';
        written++;
    } else {
        return 1;
    }

    tmp = itos(MINOR_VERSION, buf + written, n - written);
    if(tmp < 0) {
        return 1;
    }
    written += tmp;

    if(n > written) {
        buf[written] = '\0';
    } else {
        return 1;
    }

    return 0;
}

#endif // VERSION_H