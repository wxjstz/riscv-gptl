#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

void uart_init(unsigned int idx, unsigned int baudrate);

void uart_tx_byte(int idx, uint8_t c);

uint8_t uart_rx_byte(int idx);

#endif /* __UART_H__ */
