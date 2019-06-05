#include <io.h>
#include <uart.h>
#include <clock.h>
#include <addressmap.h>

#define BIT(n)	(1 << (n))

#define TXDATA_FULL			BIT(31)
#define RXDATA_EMPTY		BIT(31)
#define TXCTRL_TXEN			BIT(0)
#define TXCTRL_NSTOP_SHIFT	1
#define TXCTRL_NSTOP(x)		(((x)-1) << TXCTRL_NSTOP_SHIFT)
#define TXCTRL_TXCNT_SHIFT	16
#define TXCTRL_TXCNT(x)		((x) << TXCTRL_TXCNT_SHIFT)
#define RXCTRL_RXEN			BIT(0)
#define RXCTRL_RXCNT_SHIFT	16
#define RXCTRL_RXCNT(x)		((x) << RXCTRL_RXCNT_SHIFT)
#define IP_TXWM				BIT(0)
#define IP_RXWM				BIT(1)

struct sifive_uart_registers {
    uint32_t txdata;            /* Transmit data register */
    uint32_t rxdata;            /* Receive data register */
    uint32_t txctrl;            /* Transmit control register */
    uint32_t rxctrl;            /* Receive control register */
    uint32_t ie;                /* UART interrupt enable */
    uint32_t ip;                /* UART interrupt pending */
    uint32_t div;               /* Baud rate divisor */
} __attribute__ ((packed));

void fu540_uart_init(unsigned int idx, unsigned int baudrate)
{
    struct sifive_uart_registers *uart =
        (struct sifive_uart_registers *) (uintptr_t) FU540_UART(idx);
    unsigned int refclk = clock_get_tlclk_khz() * 1000;
    unsigned int div = ((1 + (2 * refclk)) / baudrate) / 2;
    write32(&uart->div, div);
    write32(&uart->txctrl,
            TXCTRL_TXEN | TXCTRL_NSTOP(1) | TXCTRL_TXCNT(1));
    write32(&uart->rxctrl, RXCTRL_RXEN | RXCTRL_RXCNT(0));
}

void fu540_uart_tx_byte(int idx, uint8_t c)
{
    struct sifive_uart_registers *uart =
        (struct sifive_uart_registers *) (uintptr_t) FU540_UART(idx);
    while (read32(&uart->txdata) & TXDATA_FULL);
    write32(&uart->txdata, c & 0xff);
}

uint8_t fu540_uart_rx_byte(int idx)
{
    uint32_t rxdata;
    struct sifive_uart_registers *uart =
        (struct sifive_uart_registers *) (uintptr_t) FU540_UART(idx);
    do {
        rxdata = read32(&uart->rxdata);
    }
    while (rxdata & RXDATA_EMPTY);
    return rxdata & 0xff;
}
