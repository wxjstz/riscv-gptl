#ifndef __ASM_H__
#define __ASM_H__

#define RISCV_PGSHIFT	12
#define RISCV_PGSIZE	(1 << RISCV_PGSHIFT)

#ifdef __ASSEMBLY__

#if __riscv_xlen == 32
#define REG_L	lw
#define REG_S	sw
#define RISCV_PTR	.word
#elif __riscv_xlen == 64
#define REG_L	ld
#define REG_S	sd
#define RISCV_PTR	.dword
#else
#error "only support 32-bit / 64-bit machines"
#endif

#else				/* __ASSEMBLY__ */

#define wfi()	asm volatile("wfi");
#define mret()	asm volatile("mret");
#define csr_read(csr)\
	({\
	 register unsigned long __v;\
	 __asm__ __volatile__("csrr %0, " #csr \
			 : "=r"(__v)\
			 :\
			 : "memory");\
	__v;\
	})

#define csr_write(csr, val)\
	({\
	unsigned long __v = (unsigned long)(val);\
	__asm__ __volatile__("csrw " #csr ", %0"\
			:\
			: "rK"(__v)\
			: "memory");\
	})

#endif				/* __ASSEMBLY__ */

#endif				/* __ASM_H__ */
