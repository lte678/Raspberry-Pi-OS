#include "disk/sd.h"
#include "bindings.h"


struct monoterm_cmd monoterm_cmds[] = {
    {.cmd = "sd", .cmd_func = monoterm_sd},
    {.cmd = "_", .cmd_func = 0} // Terminate list
};