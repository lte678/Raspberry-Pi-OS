#include "bindings.h"

extern monoterm_func monoterm_help;
extern monoterm_func monoterm_restart;
extern monoterm_func monoterm_sd;
extern monoterm_func monoterm_memstat;
extern monoterm_func monoterm_memtest;



struct monoterm_cmd monoterm_cmds[] = {
    {.cmd = "help", .cmd_func = monoterm_help, .desc = "Displays list of available commands"},
    {.cmd = "restart", .cmd_func = monoterm_restart, .desc = "Preforms warm restart"},
    {.cmd = "sd", .cmd_func = monoterm_sd, .desc = "BCM2837 SD MMIO driver funtions"},
    {.cmd = "memstat", .cmd_func = monoterm_memstat, .desc = "System memory and allocator statistics"},
    {.cmd = "memtest", .cmd_func = monoterm_memtest, .desc = "System memory and allocator tests"},
    {.cmd = "", .cmd_func = 0, .desc = ""} // Terminate list
};