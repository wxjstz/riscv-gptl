#include <io.h>
#include <stdint.h>
#include <addressmap.h>
#include <console.h>

// 33.33 Mhz after reset
#define FU540_BASE_FQY 33330

#define PRCI_CORECLK_MASK 1

#define PRCI_PLLCFG_DIVR_SHIFT 0
#define PRCI_PLLCFG_DIVF_SHIFT 6
#define PRCI_PLLCFG_DIVQ_SHIFT 15
#define PRCI_PLLCFG_DIVR_MASK (0x03f << PRCI_PLLCFG_DIVR_SHIFT)
#define PRCI_PLLCFG_DIVF_MASK (0x1ff << PRCI_PLLCFG_DIVF_SHIFT)
#define PRCI_PLLCFG_DIVQ_MASK (0x007 << PRCI_PLLCFG_DIVQ_SHIFT)

#define PRCI_CLKMUX_STATUS_TLCLKSEL	(1 << 1)

struct prci_ctlr {
    uint32_t hfxosccfg;         /* offset 0x00 */
    uint32_t corepllcfg0;       /* offset 0x04 */
    uint32_t reserved08;        /* offset 0x08 */
    uint32_t ddrpllcfg0;        /* offset 0x0c */
    uint32_t ddrpllcfg1;        /* offset 0x10 */
    uint32_t reserved14;        /* offset 0x14 */
    uint32_t reserved18;        /* offset 0x18 */
    uint32_t gemgxlpllcfg0;     /* offset 0x1c */
    uint32_t gemgxlpllcfg1;     /* offset 0x20 */
    uint32_t coreclksel;        /* offset 0x24 */
    uint32_t devicesresetreg;   /* offset 0x28 */
    uint32_t clkmuxstatusreg;   /* offset 0x2c */
};

static struct prci_ctlr *prci = (void *) FU540_PRCI;

int clock_get_coreclk_khz(void)
{
    if (read32(&prci->coreclksel) & PRCI_CORECLK_MASK)
        return FU540_BASE_FQY;

    uint32_t cfg = read32(&prci->corepllcfg0);
    uint32_t divr =
        (cfg & PRCI_PLLCFG_DIVR_MASK) >> PRCI_PLLCFG_DIVR_SHIFT;
    uint32_t divf =
        (cfg & PRCI_PLLCFG_DIVF_MASK) >> PRCI_PLLCFG_DIVF_SHIFT;
    uint32_t divq =
        (cfg & PRCI_PLLCFG_DIVQ_MASK) >> PRCI_PLLCFG_DIVQ_SHIFT;

//      printf("clk: r=%d f=%d q=%d\n", divr, divf, divq);
    return FU540_BASE_FQY * 2 * (divf + 1) / (divr + 1) / (1ul << divq);
}

/* Get the TileLink clock's frequency, in KHz */
int clock_get_tlclk_khz(void)
{
    if (read32(&prci->clkmuxstatusreg) & PRCI_CLKMUX_STATUS_TLCLKSEL)
        return clock_get_coreclk_khz();
    else
        return clock_get_coreclk_khz() / 2;
}
