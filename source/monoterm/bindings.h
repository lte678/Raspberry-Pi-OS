typedef int (monoterm_func)(int argc, char *argv[]);

struct monoterm_cmd {
    char cmd[16];
    monoterm_func *cmd_func;
    char desc[64];
};

extern struct monoterm_cmd monoterm_cmds[];