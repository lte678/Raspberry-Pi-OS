# Branch to the supplied label, and maintain the correct vector table offset
.macro	ventry	label
	# 2^7 = 128 Byte alignment
    .align	7
	b	\label
	.endm

.macro	kernel_call	label
	# 2^7 = 128 Byte alignment
    kernel_entry
	bl	\label
	kernel_exit
	.endm


.macro	kernel_entry
	sub	sp, sp, 256
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	str	x30, [sp, #16 * 15] 
	.endm

.macro	kernel_exit
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	ldr	x30, [sp, #16 * 15] 
	add	sp, sp, 256		
	eret
	.endm

# Documentation concerning the exception vector table.
# https://developer.arm.com/documentation/den0024/a/AArch64-Exception-Handling/AArch64-exception-table

# Set all entries to jump to the generic exception handler for now.
# Ehhh, dont know what this is for right now. Do we really need page alignement?
.align	11
.globl vectors
vectors:
	# ???
    ventry	_handle_unknown_exception			// Synchronous EL1t
	ventry	_handle_unknown_exception			// IRQ EL1t
	ventry	_handle_unknown_exception			// FIQ EL1t
	ventry	_handle_unknown_exception			// Error EL1t

	# From current EL
	ventry	_handle_sync						// Synchronous EL1h
	ventry	_handle_irq							// IRQ EL1h
	ventry	_handle_fiq							// FIQ EL1h
	ventry	_handle_serror						// Error EL1h

	# From lower EL (64-bit)
	ventry	_handle_sync_el0					// Synchronous EL0
	ventry	_handle_irq_el0						// IRQ EL0
	ventry	_handle_fiq_el0						// FIQ EL0
	ventry	_handle_serror_el0					// Error EL0

	# From lower EL (32-bit)
	ventry	_handle_unknown_exception			// Synchronous 32-bit EL0
	ventry	_handle_unknown_exception			// IRQ 32-bit EL0
	ventry	_handle_unknown_exception			// FIQ 32-bit EL0
	ventry	_handle_unknown_exception			// Error 32-bit EL0


_handle_sync:
	kernel_call handle_exception_sync

_handle_irq:
	kernel_call handle_exception_irq

_handle_fiq:
	kernel_call handle_exception_fiq

_handle_serror:
	kernel_call handle_exception_serror

_handle_sync_el0:
	kernel_call handle_exception_sync_el0

_handle_irq_el0:
	kernel_call handle_exception_irq_el0

_handle_fiq_el0:
	kernel_call handle_exception_fiq_el0

_handle_serror_el0:
	kernel_call handle_exception_serror_el0

_handle_unknown_exception:
	kernel_call handle_unknown_exception
