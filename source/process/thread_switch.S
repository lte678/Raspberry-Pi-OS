# void switch_context(struct kthread *t1, struct kthread *t2);
.globl switch_context
switch_context:
# TODO: Preserve x0 and x1
#	stp	x0, x1,   [x0, #16 * 0]
	stp	x2, x3,   [x0, #16 * 1]
	stp	x4, x5,   [x0, #16 * 2]
	stp	x6, x7,   [x0, #16 * 3]
	stp	x8, x9,   [x0, #16 * 4]
	stp	x10, x11, [x0, #16 * 5]
	stp	x12, x13, [x0, #16 * 6]
	stp	x14, x15, [x0, #16 * 7]
	stp	x16, x17, [x0, #16 * 8]
	stp	x18, x19, [x0, #16 * 9]
	stp	x20, x21, [x0, #16 * 10]
	stp	x22, x23, [x0, #16 * 11]
	stp	x24, x25, [x0, #16 * 12]
	stp	x26, x27, [x0, #16 * 13]
	stp	x28, x29, [x0, #16 * 14]
    mov x8, sp
	stp	x8, x30,  [x0, #16 * 15]
	mrs x8, NZCV
	str x8,       [x0, #16 * 16]

	# Load new context
    ldp	x8, x30,  [x1, #16 * 15]
    mov sp, x8
	ldr x8,	      [x1, #16 * 16]
	msr NZCV,     x8

	ldp	x2, x3,   [x1, #16 * 1]
	ldp	x4, x5,   [x1, #16 * 2]
	ldp	x6, x7,   [x1, #16 * 3]
	ldp	x8, x9,   [x1, #16 * 4]
	ldp	x10, x11, [x1, #16 * 5]
	ldp	x12, x13, [x1, #16 * 6]
	ldp	x14, x15, [x1, #16 * 7]
	ldp	x16, x17, [x1, #16 * 8]
	ldp	x18, x19, [x1, #16 * 9]
	ldp	x20, x21, [x1, #16 * 10]
	ldp	x22, x23, [x1, #16 * 11]
	ldp	x24, x25, [x1, #16 * 12]
	ldp	x26, x27, [x1, #16 * 13]
	ldp	x28, x29, [x1, #16 * 14]
#	ldp	x0, x1,   [x1, #16 * 0]
	mov x1, #1
# Jumps to new link register value
    ret


# void load_user_context(struct kthread *t);
.globl load_user_context
load_user_context:
	# Load new context
	# We may not modify sp, because we are not restoring registers in exceptions!
    # ldp	x8, x30,  [x0, #16 * 15]
    # mov sp, x8
	ldr x8,	      [x0, #16 * 16]
	msr NZCV,     x8

	ldp	x2,  x3,  [x0, #16 * 1]
	ldp	x4,  x5,  [x0, #16 * 2]
	ldp	x6,  x7,  [x0, #16 * 3]
	ldp	x8,  x9,  [x0, #16 * 4]
	ldp	x10, x11, [x0, #16 * 5]
	ldp	x12, x13, [x0, #16 * 6]
	ldp	x14, x15, [x0, #16 * 7]
	ldp	x16, x17, [x0, #16 * 8]
	ldp	x18, x19, [x0, #16 * 9]
	ldp	x20, x21, [x0, #16 * 10]
	ldp	x22, x23, [x0, #16 * 11]
	ldp	x24, x25, [x0, #16 * 12]
	ldp	x26, x27, [x0, #16 * 13]
	ldp	x28, x29, [x0, #16 * 14]
	ldp	x0,  x1,  [x0, #16 * 0]
# Jumps to the userspace address set in ELR_EL1
    eret
