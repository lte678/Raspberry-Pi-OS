#include <kernel/print.h>
#include "bindings.h"

int monoterm_help(int argc, char *argv[]) {
    struct monoterm_cmd *cmd = monoterm_cmds;
    // Loop until we reach the final pseudo-token
    while(*(cmd->cmd)) {
        // TODO width specifier
        print(" {s}\x1B[16G{s}\r\n", cmd->cmd, cmd->desc);
        cmd++;
    }
    return 0;
}