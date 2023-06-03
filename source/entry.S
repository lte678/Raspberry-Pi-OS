#include "init_sysregs.h"

.section .init
.globl _start
_start:
    // Save the device tree address passed by the GPU bootloader blob
    mov x19, x0
    mrs x0,mpidr_el1
    mov x1,#0xC1000000
    bic x0,x0,x1
    cbz x0,master
    b hang

master:
	ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0		

	ldr	x0, =HCR_VALUE
	msr	hcr_el2, x0

    // Control SIMD and FP instruction availability
    ldr x0, =CPTR_VALUE
    msr cptr_el2, x0
    msr cpacr_el1, x0

	ldr x0, =SPSR_VALUE
    msr spsr_el2, x0

	adr	x0, el1_entry		
	msr	elr_el2, x0

	eret				

.globl el1_entry
el1_entry:
    ldr x0, =0x80000
    mov sp, x0
    bl mmu_enable
    bl zero_bss
    mov x0, xzr
    movk x0, 0x8, lsl 16
    movk x0, 0x8000, lsl 32
    mov sp, x0
    mov x0, x19
    bl kernel_entry_point

hang: b hang

.globl dummy
dummy:
    ret
