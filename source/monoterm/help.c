#include <kernel/print.h>
#include "bindings.h"

int monoterm_help(int argc, char *argv[]) {
    struct monoterm_cmd *cmd = monoterm_cmds;
    // Loop until we reach the final pseudo-token
    while(*(cmd->cmd) != '\0') {
        uart_print(" ");
        uart_print(cmd->cmd);
        uart_print("\x1B[16G");
        uart_print(cmd->desc);
        uart_print("\r\n");
        cmd++;
    }
    return 0;
}