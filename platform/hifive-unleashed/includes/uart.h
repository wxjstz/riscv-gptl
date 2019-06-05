#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

void fu540_uart_init(unsigned int idx, unsigned int baudrate);

void fu540_uart_tx_byte(int idx, uint8_t c);

uint8_t fu540_uart_rx_byte(int idx);

#endif /* __UART_H__ */
