#pragma once


// Terminal control commands
void term_set_cursor(int x, int y);
void term_set_cursor_column(int x);
void term_set_cursor_row(int x);
void term_clear();
void term_set_bold();
void term_set_color(int code);
void term_reset_font();