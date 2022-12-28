#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/inode.h>
#include <kernel/chardev.h>
#include "../uart.h"


#define TERM_WIDTH 80
#define TERM_HEIGHT 24

// https://en.wikipedia.org/wiki/ANSI_escape_code
#define ESCAPE_TYPE_NONE    0
#define ESCAPE_TYPE_INIT    1  // Received ESC, but no [
#define ESCAPE_TYPE_UNKNOWN 2
#define ESCAPE_TYPE_CSI     3


#define KEYCODE_UNKNOWN        0
#define KEYCODE_ARROW_UP       1
#define KEYCODE_ARROW_DOWN     2
#define KEYCODE_ARROW_LEFT     3
#define KEYCODE_ARROW_RIGHT    4

static int editor_x = 0;
static int editor_y = 0;


static void print_buffer(char *buffer_start) {
    term_clear();
    char* to_print = buffer_start;
    for(int i = 0; i < TERM_HEIGHT - 1; i++) {
        term_set_cursor(0, i);
        int j = 0;
        while(*to_print != 0 && *to_print != '\n' && j < TERM_WIDTH) {
            if(is_character(*to_print)) {
                print("{c}", *to_print);
                j++;
            } else {
                print("?");
                j++;
            }            
            to_print++;
        }

        if(*to_print == '\n') {
            to_print++;
        } else {
            break;
        }        
    }
    term_set_cursor(editor_x, editor_y);
}


//static void scroll_down() {}


static void handle_special_key(int keycode) {
    switch(keycode) {
    case KEYCODE_ARROW_UP:
        if(editor_y > 0) {
            editor_y--;
        }
        break;
    case KEYCODE_ARROW_DOWN:
        if(editor_y < TERM_HEIGHT-2) {
            editor_y++;
        }
        break;
    case KEYCODE_ARROW_LEFT:
        if(editor_x > 0) {
            editor_x--;
        }
        break;
    case KEYCODE_ARROW_RIGHT:
        if(editor_x < TERM_WIDTH-1) {
            editor_x++;
        }
        break;
    default:
        print("?");
    }
    term_set_cursor(editor_x, editor_y);
}


int monoterm_edit(int argc, char *argv[]) {
    char *path;    
    if(argc == 2) {
        path = argv[1];
    } else {
        print("Invalid number of arguments.\nUsage: edit [file]\n");
        return 1;
    }

    struct inode* file = inode_from_path(g_root_inode, path);
    if(!file) {
        print("Could not resolve path.\n");
        return 1;
    }
    if(!inode_is_file(file)) {
        print("{s} is not a file\n", file->filename);
        return 1;
    }

    char* line_buffer = kmalloc(file->data_size, 0);
    if(inode_read(file, line_buffer, 0) != file->data_size) {
        print("Failed to read entire contents of file!\n");
        free(line_buffer);
        return 1;
    }

    print_buffer(line_buffer);
    term_set_cursor(0, TERM_HEIGHT-1);
    term_set_background_color(COLORCODE_GRAY);
    print(file->filename);
    term_reset_font();
    term_set_cursor(editor_x, editor_y);


    int escaped = ESCAPE_TYPE_NONE;
    while(1) {
        char new_char;
        read_char(&global_uart, &new_char, 1);
        if(new_char == KEYCODE_CTRL_C) {
            break;
        } else if(new_char == KEYCODE_ESCAPE) {
            escaped = ESCAPE_TYPE_INIT;
        } else if(escaped == ESCAPE_TYPE_INIT) {
            if(new_char == '[') {
                escaped = ESCAPE_TYPE_CSI;
            } else {
                escaped = ESCAPE_TYPE_UNKNOWN;
            }
        } else if(escaped == ESCAPE_TYPE_CSI) {
            if(new_char == 'A') {
                handle_special_key(KEYCODE_ARROW_UP);
            } else if(new_char == 'B') {
                handle_special_key(KEYCODE_ARROW_DOWN);
            } else if(new_char == 'C') {
                handle_special_key(KEYCODE_ARROW_RIGHT);
            } else if(new_char == 'D') {
                handle_special_key(KEYCODE_ARROW_LEFT);
            } else {
                print("{c}", new_char);
            }
            if(is_csi_sequence_terminator(new_char)) {
                escaped = ESCAPE_TYPE_NONE;
            }
        } else if(is_character(new_char)) {
            print("{c}", new_char);
        } else {
            term_set_cursor(0, TERM_HEIGHT-1);
            term_set_background_color(COLORCODE_GRAY);
            print("Received: {x}", (int)new_char);
            term_reset_font();
            term_set_cursor(editor_x, editor_y);
        }
        
    }

    free(line_buffer);
    term_clear();
    term_set_cursor(0, 0);
    return 0;
}