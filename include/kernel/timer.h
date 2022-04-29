#pragma once

#include <kernel/types.h>


extern uint64_t read_system_timer();
extern void enable_system_timer_interrupt();
extern void disable_system_timer_interrupt();
extern void set_system_timer_interrupt(uint32_t compare_val);