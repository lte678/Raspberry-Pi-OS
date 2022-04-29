#include <kernel/print.h>
#include <kernel/register.h>


#define INT_BASE 0x3F00B000

#define INT_ENABLE_IRQS_1 (INT_BASE + 0x210)
#define INT_ENABLE_IRQS_2 (INT_BASE + 0x214)
#define INT_DISABLE_IRQS_1 (INT_BASE + 0x21C)
#define INT_DISABLE_IRQS_2 (INT_BASE + 0x220)


void handle_exception(void)
{
	uart_print("Received exception.\r\n");
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