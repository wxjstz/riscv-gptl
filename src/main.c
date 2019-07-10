#include <asm.h>
#include <smp.h>
#include <atomic.h>
#include <stdint.h>
#include <gptl.h>
#include <device_tree.h>
#include <console.h>
#include <platform.h>
#include <malloc.h>

#define banner \
	"\n"\
	"  ___|  _ \\__ __| |\n"\
	" |     |   |  |   |\n"\
	" |   | ___/   |   |\n"\
	"\\____|_|     _|  _____|\n"\
	"\n"


static void
gptl_park(void **opensbi, void **kernel, void **initrd, void **end_addr)
{
    void *park_addr;
    size_t park_size;

    park_addr = (void *) CONFIG_OPENSBI_ADDR;
    park_size = gptl_load_opensbi(park_addr);
    if ((uintptr_t) park_addr + park_size > CONFIG_KERNEL_ADDR) {
        printf("Not enough memory to park opensbi");
        abort();
    }
    if (opensbi)
        *opensbi = park_size > 0 ? park_addr : NULL;

    park_addr = (void *) CONFIG_KERNEL_ADDR;
    park_size = gptl_load_kernel(park_addr);
    if ((uintptr_t) park_addr + park_size > CONFIG_FDT_ADDR) {
        printf("Not enough memory to park opensbi");
        abort();
    }
    if (kernel)
        *kernel = park_size > 0 ? park_addr : NULL;

    park_addr = (void *) (CONFIG_FDT_ADDR + CONFIG_RESERVED_FOR_FDT);
    park_size = gptl_load_initrd(*initrd);
    if (initrd)
        *initrd = park_size > 0 ? park_addr : NULL;

    if (end_addr)
        *end_addr =
            (void *) ALIGN_UP((uintptr_t) park_addr + park_size,
                              RISCV_PGSIZE);
}

/**
 * /{
 *     ......
 *     chosen {
 *	       bootargs = ;
 *         linux,initrd-start = ;
 *         linux,initrd_end = ;
 *     }
 * }
 */
static void
modify_dt(void *fdt, void *park_addr, void *kernel,
          void *initrd, size_t initrd_size, const char *bootargs)
{
    struct device_tree_node *chosen_node;
    struct device_tree *dt = fdt_unflatten(fdt);
    chosen_node = dt_find_node_by_path(dt->root, "chosen", NULL, NULL, 1);

    if (initrd && initrd_size) {
        uint64_t initrd_start = (uint64_t) initrd;
        uint64_t initrd_end = initrd_start + initrd_size;
        dt_add_uint64_t_prop(chosen_node, "linux,initrd-start",
                             initrd_start);
        dt_add_uint64_t_prop(chosen_node, "linux,initrd-end", initrd_end);
    }
    if (bootargs)
        dt_add_string_prop(chosen_node, "bootargs", bootargs);

    dt_flatten(dt, park_addr);
}

void main(int hartid, void *fdt)
{
    static void *new_fdt;
    static void (*opensbi_func) (int, void *);

    /* lucky hart performs the main task */
    if (hartid == 0) {
        printf("%s", banner);

        struct gptl_header *gptl_header = search_gptl();
        assert(gptl_header != NULL);

        size_t fdt_size = gptl_header_get_field(gptl_header, fdt_size);
        if (fdt_size) {
            fdt = malloc(fdt_size);
            gptl_load_fdt(fdt);
        }

        void *opensbi, *kernel, *initrd, *park_end;
        gptl_park(&opensbi, &kernel, &initrd, &park_end);
        opensbi_func = opensbi;
        new_fdt = park_end;

        size_t initrd_size =
            gptl_header_get_field(gptl_header, initrd_size);

        modify_dt(fdt, new_fdt, kernel, initrd, initrd_size,
                  gptl_bootargs());
    }
    smp_wait(platform_hart_num());

#ifdef DEBUG
    static spinlock_t lock;
    spinlock_lock(&lock);
    printf("running opensbi at %p, hartid:%d, fdt: %p\n", opensbi_func,
           hartid, new_fdt);
    spinlock_unlock(&lock);
    smp_wait(platform_hart_num());
#endif

    asm volatile ("fence rw, rw");
    uintptr_t status = csr_read(mstatus);
    status = INSERT_FIELD(status, MSTATUS_MPIE, 0);
    status = INSERT_FIELD(status, MSTATUS_MPP, PRV_M);
    csr_write(mstatus, status);
    csr_write(mie, 0);
    csr_write(mtvec, opensbi_func);
    csr_write(mepc, opensbi_func);
    asm volatile ("mv	a0, %0\n\t"
                  "mv	a1, %1\n\t"
				  "mret"
				  :
				  :"r" (hartid), "r"(new_fdt)
                  :"a0", "a1");
}
