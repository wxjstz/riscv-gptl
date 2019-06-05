#define __ASSEMBLY__
#include <asm.h>

.section ".entry","ax"
.global _start
_start:
	/* disable interrupt of all harts */
	csrwi	mie, 0
	csrwi	mip, 0

	/* initialize stack pointer */
	addi	t0, a0, 1
	slli	t0, t0, RISCV_PGSHIFT
	la	t1, __stack_start
	add	sp, t0, t1

	bnez	a0, waiting_boot_hart_complete

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
	la	t1, boot_hart_completed
	REG_S	t0, 0(t1)

waiting_boot_hart_complete:
	fence	rw, rw
	la	t0, boot_hart_completed
	REG_L	t0, 0(t0)
	beqz	t0, waiting_boot_hart_complete

	mv	s0, a0
	mv	s1, a1
	call	platform_init
	mv	a0, s0
	mv	a1, s1

	call	main
1:
	wfi
	j	1b

.data
boot_hart_completed:
	RISCV_PTR	0

