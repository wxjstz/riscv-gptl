#include <io.h>
#include <asm.h>
#include <smp.h>
#include <atomic.h>
#include <stdint.h>
#include <sd.h>
#include <uart.h>
#include <clock.h>
#include <string.h>
#include <platform.h>
#include <addressmap.h>

#include <console.h>

#define bs	512

static struct spi_ctrl *spi_ctrl = (struct spi_ctrl *) FU540_QSPI2;

static void *platform_read_(size_t offset, size_t size, void *buff)
{
    uint8_t tmp[bs];
    if (offset / bs == (offset + size - 1) / bs) {
        size_t lba = offset / bs;
        size_t o = offset % bs;
        size_t l = size;
        if (sd_copy(spi_ctrl, tmp, lba, 1) != 0)
            return NULL;
        memcpy(buff, tmp + o, l);
    } else {
        int has_begin = ! !(offset % bs);
        int has_end = ! !((offset + size) % bs);
        size_t first_lba = offset / bs;
        size_t last_lba = (offset + size - 1) / bs;
        if (has_begin) {
            size_t lba = first_lba;
            size_t o = offset % bs;
            size_t l = bs - o;
            if (sd_copy(spi_ctrl, tmp, lba, 1) != 0)
                return NULL;
            memcpy(buff, tmp + o, l);
        }
        if (first_lba + has_begin <= last_lba - has_end) {
            size_t lba = first_lba + has_begin;
            size_t count = (last_lba - has_end) - lba + 1;
            size_t o = has_begin ? bs - offset % bs : 0;
            if (sd_copy(spi_ctrl, buff + o, lba, count) != 0)
                return NULL;
        }
        if (has_end) {
            size_t lba = last_lba;
            size_t o = 0;
            size_t l = (offset + size) % bs;
            if (sd_copy(spi_ctrl, tmp, lba, 1) != 0)
                return NULL;
            memcpy(buff + size - l, tmp + o, l);
        }
    }
    return buff;
}

static void platform_init_()
{
    if (csr_read(mhartid) == 0) {
        /* init uart for console */
        fu540_uart_init(0, 115200);

        /* init spi for sdcard */
        sd_init(spi_ctrl);
    }

    smp_wait(platform_hart_num());
}

static void platform_putchar_(char c)
{
    fu540_uart_tx_byte(0, c);
}

struct platform_interface platform = {
    .hart_num = 5,
    .platform_name = "HiFive-Unleashed",
    .init = platform_init_,
    .putchar = platform_putchar_,
    .logic_block_size = bs,
    .read = platform_read_,
};
