#include <kernel/page.h>


uint64_t round_up_to_page(uint64_t a) {
    if(a == 0) {
        return 0;
    }
    return ((a - 1) & ~(PAGE_SIZE-1)) + PAGE_SIZE;
}