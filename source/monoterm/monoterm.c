#include <kernel/string.h>
#include <kernel/print.h>
#include <kernel/term.h>
#include <kernel/chardev.h>

#include "bindings.h"
#include "disk/sd.h"
#include "uart.h"
#include "monoterm.h"

#define MAX_INPUT_LENGTH 4096
#define MAX_TOKENS       64


uint64_t get_token_length(char *s, char delimiter) {
    uint64_t len = 0;
    while(s[len] != delimiter && s[len] != '\0') {
        len++;
    }
    return len;
}


int process_input(char *s) {
    // Up to MAX_TOKENS arguments
    char *args[MAX_TOKENS];

    // Tokenize input
    uint64_t token_idx = 0;
    while(*s) {
        int64_t token_len = get_token_length(s, ' ');
        if(token_len > 0) {
            args[token_idx] = kmalloc(token_len + 1, 0);
            memcpy(args[token_idx], s, token_len);
            args[token_idx][token_len] = '\0';
            s += token_len;
            token_idx++;
        } else {
            // We are pointing at whitespace, continue to the next character
            s++;
        }
    }
    
    // Find and execute command
    struct monoterm_cmd *cmd = monoterm_cmds;
    // Loop until we reach the final pseudo-token
    while(strcmp(cmd->cmd, "\0")) {
        if(!strcmp(cmd->cmd, args[0])) {
            // Returns the result/return value of the command
            return (cmd->cmd_func)(token_idx, args);
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
    // Non-zeroed memory
    char *input = kmalloc(MAX_INPUT_LENGTH, 0);

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
            if(input_i < MAX_INPUT_LENGTH) {
                input[input_i] = c;
                // Echo user input
                write_char(&global_uart, &c, 1);
                input_i++;
            }
        }
        prev = c;
    }
}