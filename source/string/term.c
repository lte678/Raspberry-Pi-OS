#include <kernel/print.h>

// Terminal control commands

void term_set_cursor(int x, int y) {
    print("\x1B[{i};{i}H", x, y);
}

void term_set_cursor_column(int x) {
    print("\x1B[{i}G", x);
}

void term_set_cursor_row(int x) {
    // TODO
    print("\r\nterm_set_cursor_row not implemented!\r\n");
}

void term_set_bold() {
    print("\x1B[1m");
}

void term_set_color(int code) {
    print("\x1B[38;5;{i}m", code);
}

void term_reset_font() {
    print("\x1B[0m");
}