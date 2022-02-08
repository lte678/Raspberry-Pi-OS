struct monoterm_cmd {
    char cmd[32];
    int (*cmd_func)(int argc, char *argv[]);
};

extern struct monoterm_cmd monoterm_cmds[];