#pragma once

#include <kernel/types.h>


uint64_t read_system_timer();
void enable_system_timer_interrupt();
void disable_system_timer_interrupt();
void set_system_timer_interrupt(uint32_t compare_val);