#include <kernel/print.h>

// Terminal control commands

void term_set_cursor(int x, int y) {
    print("\x1B[{i};{i}H", y, x);
}

void term_set_cursor_column(int x) {
    print("\x1B[{i}G", x);
}

void term_set_cursor_row(int x) {
    // TODO
    print("\nterm_set_cursor_row not implemented!\n");
}

void term_clear() {
    print("\x1B[2J");
}

void term_set_bold() {
    print("\x1B[1m");
}

void term_set_color(int code) {
    print("\x1B[38;5;{i}m", code);
}

void term_set_background_color(int code) {
    print("\x1B[48;5;{i}m", code);
}

void term_reset_font() {
    print("\x1B[0m");
}

void term_move_cursor_up() {
    print("\x1B[A");
}

void term_move_cursor_down() {
    print("\x1B[B");
}

void term_move_cursor_forward() {
    print("\x1B[C");
}

void term_move_cursor_back() {
    print("\x1B[D");
}