#include <kernel/types.h>
#include <kernel/register.h>
#include <kernel/exception.h>


#define TIMER_BASE 0x3F000000

#define TIMER_CS  (TIMER_BASE + 0x00)
#define TIMER_CLO (TIMER_BASE + 0x04)
#define TIMER_CHI (TIMER_BASE + 0x08)
#define TIMER_C0  (TIMER_BASE + 0x0C)  // Used by GPU
#define TIMER_C1  (TIMER_BASE + 0x10)
#define TIMER_C2  (TIMER_BASE + 0x14)  // Used by GPU
#define TIMER_C3  (TIMER_BASE + 0x18)

uint64_t read_system_timer() {
    uint32_t lower, upper;
    upper = get32(TIMER_CHI);
    lower = get32(TIMER_CLO);
    while(upper != get32(TIMER_CHI)) {
        upper = get32(TIMER_CHI);
        // Make sure the upper bits do not change in the time when we are sampling the lower bits.
        lower = get32(TIMER_CLO);
    }
    return (uint64_t)upper << 32ul | lower;
}

void enable_system_timer_interrupt() {
    enable_peripheral_interrupt(PERIPHERAL_INTERRUPT_CLOCK1);
}

void disable_system_timer_interrupt() {
    disable_peripheral_interrupt(PERIPHERAL_INTERRUPT_CLOCK1);
}

void set_system_timer_interrupt(uint32_t compare_val) {
    // Reset the flag for timer 1
    put32(TIMER_CS, 1 << 1);
    // Set the compare value
    put32(TIMER_C1, compare_val);
}