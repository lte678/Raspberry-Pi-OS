#pragma once


#define COLORCODE_BLACK 0
#define COLORCODE_RED   1
#define COLORCODE_GRAY  8
#define COLORCODE_ORANGE 208
#define COLORCODE_WARNING COLORCODE_ORANGE

// Terminal control commands
void term_set_cursor(int x, int y);
void term_set_cursor_column(int x);
void term_set_cursor_row(int x);
void term_clear();
void term_set_bold();
void term_set_background_color(int code);
void term_set_color(int code);
void term_reset_font();
void term_move_cursor_up();
void term_move_cursor_down();
void term_move_cursor_forward();
void term_move_cursor_back();