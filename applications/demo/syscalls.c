#include <sys/stat.h>
#include <sys/errno.h>

// SYSCALLS
///////////////////////

#define __NR_exit   1
#define __NR_fstat  2
#define __NR_read   3
#define __NR_write  4
#define __NR_getpid 5


long syscall(long sysnum, long a, long b, long c, long d, long e, long f)
{
	register long _x0 __asm__("x0")=(long)(sysnum);
	register long _x6 __asm__("x6")=(long)(f);
	register long _x5 __asm__("x5")=(long)(e);
	register long _x4 __asm__("x4")=(long)(d);
	register long _x3 __asm__("x3")=(long)(c);
	register long _x2 __asm__("x2")=(long)(b);
	register long _x1 __asm__("x1")=(long)(a);
	__asm__ __volatile__(
			"svc #0"
			: "=r"(_x0)
			: "r"(_x0), "r"(_x1),
			"r"(_x2), "r"(_x3), "r"(_x4), "r"(_x5),
			"r"(_x6)
			: "memory");
	if(_x0 >=(unsigned long) -4095) {
		long err = _x0;
		errno=(-err);
		_x0=(unsigned long) -1;
	}
	return (long) _x0;
}

// LIBC SYSCALLS
/////////////////////

extern int _end;

void *_sbrk(int incr) {
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;

    if (heap == NULL) {
        heap = (unsigned char *)&_end;
    }
    prev_heap = heap;

    heap += incr;

    return prev_heap;
}

int _close(int file) {
    return -1;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;

    return 0;
}

int _isatty(int file) {
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

void _exit(int status) {
    while(1) {
        syscall(__NR_exit, status, 0, 0, 0, 0, 0);
    }
}

void _kill(int pid, int sig) {
    return;
}

int _getpid(void) {
    return syscall(__NR_getpid, 0, 0, 0, 0, 0, 0);
}

int _write (int file, void *ptr, int len) {
    return syscall(__NR_write, file, (unsigned long)ptr, len, 0, 0, 0);
}

int _read (int file, char * ptr, int len) {
    return syscall(__NR_read, file, (unsigned long)ptr, len, 0, 0, 0);
}