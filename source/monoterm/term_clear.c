#include <kernel/term.h>


int monoterm_clear(int argc, char *argv[]) {
    term_clear();
    term_set_cursor(1, 1);
    return 0;
}