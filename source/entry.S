#include "init_sysregs.h"

.section .init
.globl _start
_start:
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
    bl kernel_entry_point

hang: b hang

.globl dummy
dummy:
    ret

//-------------------------------------------------------------------------
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------
