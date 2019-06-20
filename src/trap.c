#include <stdint.h>
#include <console.h>
#include <smp.h>

static void dump_status(uintptr_t * regs)
{
    static spinlock_t lock;
    static char *regs_names[] = {
        "x0", "ra", "sp", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
    };
    uintptr_t mstatus = csr_read(mstatus);
    uintptr_t mhartid = csr_read(mhartid);
    uintptr_t mepc = csr_read(mepc);
    uintptr_t mtval = csr_read(mtval);
    uintptr_t mcause = csr_read(mcause);

    spinlock_lock(&lock);
    printf("mhartid = %d\n", mhartid);
    printf("mstatus = %016lx\n", mstatus);
    printf("mepc    = %016lx\n", mepc);
    printf("mtval   = %016lx\n", mtval);
    printf("mcause  = %016lx\n", mcause);
    for (int i = 0; i < 32; i++)
        printf("%s\t= %016lx\n", regs_names[i], regs[i]);
    printf("\n");
    spinlock_unlock(&lock);
}

void trap_handle(uintptr_t * regs)
{
    regs[0] = 0;                /* zero */
    regs[2] = (uintptr_t) regs + 32 * __SIZEOF_POINTER__;       /* sp */

    dump_status(regs);
    while (1) {
        asm volatile ("wfi");
    }
}
