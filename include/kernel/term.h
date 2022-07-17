#pragma once


// Terminal control commands
extern void term_set_cursor(int x, int y);
extern void term_set_cursor_column(int x);
extern void term_set_cursor_row(int x);
extern void term_clear();
extern void term_set_bold();
extern void term_set_color(int code);
extern void term_reset_font();