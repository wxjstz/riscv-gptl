#define __ASSEMBLY__
#include <asm.h>

	.align   3
	.section ".entry","ax"
	.global  _start
_start:
	/* disable interrupt of all harts */
	csrwi	mie, 0
	csrwi	mip, 0

	/* initialize stack pointer */
	addi	t0, a0, 1
	slli	t0, t0, RISCV_PGSHIFT
	la	t1, __stack_start
	add	sp, t0, t1

	/* initialize trap handle */
	la	t0, _trap_entry
	csrw	mtvec, t0
1:
	csrr	t1, mtvec
	bne	t0, t1, 1b

	bnez	a0, _waitting_boot_hart_complete

	/* initialize bss section to zero */
	la	t0, __bss_start
	la	t1, __bss_end
1:
	bge	t0, t1, 1f
	sb	zero, 0(t0)
	addi	t0, t0, 1
	j	1b
1:

	/* wakeup other hartid */
	li	t0, 1
	la	t1, _boot_hart_completed
	REG_S	t0, 0(t1)

_waitting_boot_hart_complete:
	fence	rw, rw
	la	t0, _boot_hart_completed
	REG_L	t0, 0(t0)
	beqz	t0, _waitting_boot_hart_complete

	mv	s0, a0
	mv	s1, a1
	call	platform_init
	mv	a0, s0
	mv	a1, s1

	call	main
1:
	wfi
	j	1b

	.align   3
	.section ".text","ax"
_trap_entry:
	add	sp, sp, - 32 * __SIZEOF_POINTER__
//	REG_S	x0,   0 * __SIZEOF_POINTER__ (sp)	; zero
	REG_S	x1,   1 * __SIZEOF_POINTER__ (sp)
//	REG_S	x2,   2 * __SIZEOF_POINTER__ (sp)	; sp
	REG_S	x3,   3 * __SIZEOF_POINTER__ (sp)
	REG_S	x4,   4 * __SIZEOF_POINTER__ (sp)
	REG_S	x5,   5 * __SIZEOF_POINTER__ (sp)
	REG_S	x6,   6 * __SIZEOF_POINTER__ (sp)
	REG_S	x7,   7 * __SIZEOF_POINTER__ (sp)
	REG_S	x8,   8 * __SIZEOF_POINTER__ (sp)
	REG_S	x9,   9 * __SIZEOF_POINTER__ (sp)
	REG_S	x10, 10 * __SIZEOF_POINTER__ (sp)
	REG_S	x11, 11 * __SIZEOF_POINTER__ (sp)
	REG_S	x12, 12 * __SIZEOF_POINTER__ (sp)
	REG_S	x13, 13 * __SIZEOF_POINTER__ (sp)
	REG_S	x14, 14 * __SIZEOF_POINTER__ (sp)
	REG_S	x15, 15 * __SIZEOF_POINTER__ (sp)
	REG_S	x16, 16 * __SIZEOF_POINTER__ (sp)
	REG_S	x17, 17 * __SIZEOF_POINTER__ (sp)
	REG_S	x18, 18 * __SIZEOF_POINTER__ (sp)
	REG_S	x19, 19 * __SIZEOF_POINTER__ (sp)
	REG_S	x20, 20 * __SIZEOF_POINTER__ (sp)
	REG_S	x21, 21 * __SIZEOF_POINTER__ (sp)
	REG_S	x22, 22 * __SIZEOF_POINTER__ (sp)
	REG_S	x23, 23 * __SIZEOF_POINTER__ (sp)
	REG_S	x24, 24 * __SIZEOF_POINTER__ (sp)
	REG_S	x25, 25 * __SIZEOF_POINTER__ (sp)
	REG_S	x26, 26 * __SIZEOF_POINTER__ (sp)
	REG_S	x27, 27 * __SIZEOF_POINTER__ (sp)
	REG_S	x28, 28 * __SIZEOF_POINTER__ (sp)
	REG_S	x29, 29 * __SIZEOF_POINTER__ (sp)
	REG_S	x30, 30 * __SIZEOF_POINTER__ (sp)
	REG_S	x31, 31 * __SIZEOF_POINTER__ (sp)

	add	a0, sp, zero
	call	trap_handle

//	REG_L	x0,   0 * __SIZEOF_POINTER__ (sp)	; zero
	REG_L	x1,   1 * __SIZEOF_POINTER__ (sp)
//	REG_L	x2,   2 * __SIZEOF_POINTER__ (sp)	; sp
	REG_L	x3,   3 * __SIZEOF_POINTER__ (sp)
	REG_L	x4,   4 * __SIZEOF_POINTER__ (sp)
	REG_L	x5,   5 * __SIZEOF_POINTER__ (sp)
	REG_L	x6,   6 * __SIZEOF_POINTER__ (sp)
	REG_L	x7,   7 * __SIZEOF_POINTER__ (sp)
	REG_L	x8,   8 * __SIZEOF_POINTER__ (sp)
	REG_L	x9,   9 * __SIZEOF_POINTER__ (sp)
	REG_L	x10, 10 * __SIZEOF_POINTER__ (sp)
	REG_L	x11, 11 * __SIZEOF_POINTER__ (sp)
	REG_L	x12, 12 * __SIZEOF_POINTER__ (sp)
	REG_L	x13, 13 * __SIZEOF_POINTER__ (sp)
	REG_L	x14, 14 * __SIZEOF_POINTER__ (sp)
	REG_L	x15, 15 * __SIZEOF_POINTER__ (sp)
	REG_L	x16, 16 * __SIZEOF_POINTER__ (sp)
	REG_L	x17, 17 * __SIZEOF_POINTER__ (sp)
	REG_L	x18, 18 * __SIZEOF_POINTER__ (sp)
	REG_L	x19, 19 * __SIZEOF_POINTER__ (sp)
	REG_L	x20, 20 * __SIZEOF_POINTER__ (sp)
	REG_L	x21, 21 * __SIZEOF_POINTER__ (sp)
	REG_L	x22, 22 * __SIZEOF_POINTER__ (sp)
	REG_L	x23, 23 * __SIZEOF_POINTER__ (sp)
	REG_L	x24, 24 * __SIZEOF_POINTER__ (sp)
	REG_L	x25, 25 * __SIZEOF_POINTER__ (sp)
	REG_L	x26, 26 * __SIZEOF_POINTER__ (sp)
	REG_L	x27, 27 * __SIZEOF_POINTER__ (sp)
	REG_L	x28, 28 * __SIZEOF_POINTER__ (sp)
	REG_L	x29, 29 * __SIZEOF_POINTER__ (sp)
	REG_L	x30, 30 * __SIZEOF_POINTER__ (sp)
	REG_L	x31, 31 * __SIZEOF_POINTER__ (sp)
	add	sp, sp, 32 * __SIZEOF_POINTER__
	mret

	.align   3
	.section ".data", "aw"
_boot_hart_completed:
	RISCV_PTR	0

