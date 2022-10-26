#include <kernel/print.h>
#include <kernel/register.h>
#include <kernel/timer.h>
#include <kernel/panic.h>


#define INT_BASE 0x3F00B000

#define INT_PENDING_1 (INT_BASE + 0x204)
#define INT_PENDING_2 (INT_BASE + 0x208)
#define INT_ENABLE_IRQS_1 (INT_BASE + 0x210)
#define INT_ENABLE_IRQS_2 (INT_BASE + 0x214)
#define INT_DISABLE_IRQS_1 (INT_BASE + 0x21C)
#define INT_DISABLE_IRQS_2 (INT_BASE + 0x220)

#define PSTATE_DEBUG_INT_MASK (0b1000ul << 6)
#define PSTATE_SERROR_INT_MASK (0b0100ul << 6)
#define PSTATE_IRQ_INT_MASK (0b0010ul << 6)
#define PSTATE_FIQ_INT_MASK (0b0001ul << 6)

#define TIMER_BASE 0x3F003000

#define TIMER_CS  (TIMER_BASE + 0x00)

extern int vectors;


void unmask_exception_class(uint64_t mask) {
	// Debug, system error, irq, fiq mask bits 
	uint64_t daif = read_system_reg(DAIF);
	daif &= ~PSTATE_IRQ_INT_MASK;
	write_system_reg(DAIF, daif);
}

void mask_exception_class(uint64_t mask) {
	// Debug, system error, irq, fiq mask bits 
	uint64_t daif = read_system_reg(DAIF);
	daif |= PSTATE_IRQ_INT_MASK;
	write_system_reg(DAIF, daif);
}

void init_exceptions() {
    write_system_reg(VBAR_EL1, (uint64_t)&vectors);
    unmask_exception_class(PSTATE_IRQ_INT_MASK);
}

static void print_esr_and_far(uint64_t esr, uint64_t far, unsigned int error_class) {
	// Exception link register (fault address)
	print("ELR: {xl}\r\n", read_system_reg(ELR_EL1));
	// Exception syndrome register
	print("ESR: {xl}    FAR: {xl}\r\n", esr, far);
	print("EC: {u}    IL: {u}    ISS: {u}\r\n", error_class, (esr >> 25) & 1, esr & 0x1FFFFFF);
}

void handle_unknown_exception()
{
	print("## UNKNOWN EXCEPTION ##\r\n");
	panic();
}

void handle_exception_sync()
{
	uint64_t esr = read_system_reg(ESR_EL1);
	uint64_t far = read_system_reg(FAR_EL1);
	unsigned int error_class = (esr & (0b111111 << 26)) >> 26;

	print("## SYNC EXCEPTION ##\r\n");
	switch(error_class) {
	case 7:
		print("Invalid vector instruction.\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
	case 37:
		// 0b100101: Data Abort taken without change in exception level.
		print("Invalid memory access from kernel!\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
		break;
	default:
		print("Unidentified kernel exception.\r\n");	
		print("See D13-5347 in ARM Reference Manual\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
	}
}

void handle_exception_irq() {
	print(".");
	// TODO: Allow us to register interrupt handlers.
	uint64_t time = read_system_timer();
	set_system_timer_interrupt((time + 1000000) & 0xFFFFFFFF);
}

void handle_exception_fiq()
{
	print("## UNHANDLED FIQ EXCEPTION ##\r\n");
	panic();
}

void handle_exception_serror()
{
	print("## SERROR EXCEPTION ##\r\n");
	panic();
}

void handle_exception_sync_el0()
{
	uint64_t esr = read_system_reg(ESR_EL1);
	uint64_t far = read_system_reg(FAR_EL1);
	unsigned int error_class = (esr & (0b111111 << 26)) >> 26;

	print("## EL0 SYNC EXCEPTION ##\r\n");
	switch(error_class) {
	case 7:
		print("Invalid vector instruction.\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
	case 36:
		// 0b100101: Data Abort taken without change in exception level.
		print("Invalid memory access from userspace!\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
		break;
	default:
		print("Unidentified exception in userspace.\r\n");
		print("See D13-5347 in ARM Reference Manual\r\n");
		print_esr_and_far(esr, far, error_class);
		panic();
	}
}

void handle_exception_irq_el0() {
	print("## UNHANDLED EL0 IRQ EXCEPTION ##\r\n");
	panic();
}

void handle_exception_fiq_el0()
{
	print("## UNHANDLED EL0 FIQ EXCEPTION ##\r\n");
	panic();
}

void handle_exception_serror_el0()
{
	print("## EL0 SERROR EXCEPTION ##\r\n");
	panic();
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