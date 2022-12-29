#include "bindings.h"

extern monoterm_func monoterm_help;
extern monoterm_func monoterm_clear;
extern monoterm_func monoterm_ls;
extern monoterm_func monoterm_cat;
//extern monoterm_func monoterm_restart;
extern monoterm_func monoterm_elfdump;
extern monoterm_func monoterm_run;
extern monoterm_func monoterm_ps;
extern monoterm_func monoterm_edit;
extern monoterm_func monoterm_sd;
extern monoterm_func monoterm_memstat;
extern monoterm_func monoterm_memtest;



struct monoterm_cmd monoterm_cmds[] = {
    {.cmd = "help", .cmd_func = monoterm_help, .desc = "Displays list of available commands"},
    {.cmd = "clear", .cmd_func = monoterm_clear, .desc = "Clear terminal"},
    {.cmd = "ls", .cmd_func = monoterm_ls, .desc = "Lists files"},
    {.cmd = "cat", .cmd_func = monoterm_cat, .desc = "Outputs file contents to console"},
    {.cmd = "elfdump", .cmd_func = monoterm_elfdump, .desc = "Prints ELF header"},
    {.cmd = "run", .cmd_func = monoterm_run, .desc = "Executes ELF file"},
    {.cmd = "ps", .cmd_func = monoterm_ps, .desc = "Print processes known to the kernel"},
    {.cmd = "edit", .cmd_func = monoterm_edit, .desc = "Edit text files"},
    {.cmd = "sd", .cmd_func = monoterm_sd, .desc = "BCM2837 SD MMIO driver funtions"},
    {.cmd = "memstat", .cmd_func = monoterm_memstat, .desc = "System memory and allocator statistics"},
    {.cmd = "memtest", .cmd_func = monoterm_memtest, .desc = "System memory and allocator tests"},
//  {.cmd = "restart", .cmd_func = monoterm_restart, .desc = "Preforms warm restart"},
    {.cmd = "", .cmd_func = 0, .desc = ""} // Terminate list
};