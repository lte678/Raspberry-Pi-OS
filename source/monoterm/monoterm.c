#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/chardev.h>

#include "bindings.h"
#include "disk/sd.h"
#include "uart.h"
#include "monoterm.h"

#define MAX_INPUT_LENGTH 256


int process_input(char *s) {
    // Up to 8 arguments with a length of 32 characters each
    char raw_args[8*32];
    char *args[8];
    for(int i = 0; i < 8; i++) {
        args[i] = &raw_args[32*i];
    }

    // Tokenize input
    // TODO: Make sure we are not exceeding array bounds
    int arg_i = 0;
    int str_i = 0;
    while(*s) {
        args[arg_i][str_i] = *s;
        str_i++;
        s++;
        if(*s == ' ') {
            // Finish string
            args[arg_i][str_i] = '\0';
            while(*s == ' ') {
                s++;
            }
            if(*s) {
                // Dont add argument for trailing whitespace
                arg_i++;
                str_i = 0;
            }
        }
    }
    // Finish string
    args[arg_i][str_i] = '\0';

    // Find and execute command
    struct monoterm_cmd *cmd = monoterm_cmds;
    // Loop until we reach the final pseudo-token
    while(strcmp(cmd->cmd, "\0")) {
        if(!strcmp(cmd->cmd, args[0])) {
            // Returns the result/return value of the command
            return (cmd->cmd_func)(arg_i + 1, args);
        }
        cmd++;
    }
    print("Command not found!\n");
    return -1;
}

static void print_prompt() {
    term_set_bold();
    term_set_color(2);
    print(">>>");
    term_reset_font();
}

void monoterm_start() {
    char input[MAX_INPUT_LENGTH];

    print_prompt();
    int input_i = 0;
    char prev = '\0'; // To check whether the last character was an escape
    while(1) {
        char c;
        read_char(&global_uart, &c, 1);
        // Only execute \n and \n\r once
        if((c == '\r' && prev != '\n') || (c == '\n' && prev != '\r')) {
            input[input_i] = '\0';
            print("\n"); 
            // Process user input buffer
            if(process_input(input)) {
                print("Command terminated with error.\n");
            }
            // Display new prompt
            print_prompt();
            input_i = 0;
        } else if(c == '\x7F' || c == '\x08') {
            // Handle backspace
            if(input_i) {
                print("\x08\x1B[K");
                input_i--;
            }
        } else {
            input[input_i] = c;
            // Echo user input
            write_char(&global_uart, &c, 1);
            input_i++;
        }
        prev = c;
    }
}