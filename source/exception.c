#include <kernel/print.h>
#include <kernel/register.h>
#include <kernel/timer.h>


#define INT_BASE 0x3F00B000

#define INT_PENDING_1 (INT_BASE + 0x204)
#define INT_PENDING_2 (INT_BASE + 0x208)
#define INT_ENABLE_IRQS_1 (INT_BASE + 0x210)
#define INT_ENABLE_IRQS_2 (INT_BASE + 0x214)
#define INT_DISABLE_IRQS_1 (INT_BASE + 0x21C)
#define INT_DISABLE_IRQS_2 (INT_BASE + 0x220)


#define TIMER_BASE 0x3F003000

#define TIMER_CS  (TIMER_BASE + 0x00)


void handle_exception(void)
{
	uart_print("Received exception.\r\n");

    uint64_t time = read_system_timer();
	set_system_timer_interrupt((time + 10000000) & 0xFFFFFFFF);
}

void enable_peripheral_interrupt(int interrupt_number) {
	// See page 113 of BCM2835 peripheral manual
	if(interrupt_number < 32) {
		put32(INT_ENABLE_IRQS_1, 1u << interrupt_number);
	} else if(interrupt_number < 64) {
		put32(INT_ENABLE_IRQS_2, 1u << (interrupt_number - 32));
	}
}

void disable_peripheral_interrupt(int interrupt_number) {
	// See page 113 of BCM2835 peripheral manual
	if(interrupt_number < 32) {
		put32(INT_DISABLE_IRQS_1, 1u << interrupt_number);
	} else if(interrupt_number < 64) {
		put32(INT_DISABLE_IRQS_2, 1u << (interrupt_number - 32));
	}
}