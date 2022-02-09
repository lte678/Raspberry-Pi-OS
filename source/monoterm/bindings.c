#include "bindings.h"

extern monoterm_func monoterm_help;
extern monoterm_func monoterm_sd;


struct monoterm_cmd monoterm_cmds[] = {
    {.cmd = "help", .cmd_func = monoterm_help, .desc = "Displays list of available commands"},
    {.cmd = "sd", .cmd_func = monoterm_sd, .desc = "BCM2837 SD MMIO driver funtions"},
    {.cmd = "", .cmd_func = 0, .desc = ""} // Terminate list
};