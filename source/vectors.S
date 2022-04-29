# Branch to the supplied label, and maintain the correct vector table offset
.macro	ventry	label
	# 2^7 = 128 Byte alignment
    .align	7
	b	\label
	.endm


# Documentation concerning the exception vector table.
# https://developer.arm.com/documentation/den0024/a/AArch64-Exception-Handling/AArch64-exception-table

# Set all entries to jump to the generic exception handler for now.
# Ehhh, dont know what this is for right now. Do we really need page alignement?
.align	11
.globl vectors
vectors:
    ventry	handle_exception			// Synchronous EL1t
	ventry	handle_exception			// IRQ EL1t
	ventry	handle_exception			// FIQ EL1t
	ventry	handle_exception			// Error EL1t

	ventry	handle_exception			// Synchronous EL1h
	ventry	handle_exception			// IRQ EL1h
	ventry	handle_exception			// FIQ EL1h
	ventry	handle_exception			// Error EL1h

	ventry	handle_exception			// Synchronous 64-bit EL0
	ventry	handle_exception			// IRQ 64-bit EL0
	ventry	handle_exception			// FIQ 64-bit EL0
	ventry	handle_exception			// Error 64-bit EL0

	ventry	handle_exception			// Synchronous 32-bit EL0
	ventry	handle_exception			// IRQ 32-bit EL0
	ventry	handle_exception			// FIQ 32-bit EL0
	ventry	handle_exception			// Error 32-bit EL0


.globl irq_init
irq_init:
    adr x0, vectors
    msr vbar_el1, x0
    msr daifclr, #2 
    ret
