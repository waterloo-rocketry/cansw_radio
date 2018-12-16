#ifndef UART_H_
#define	UART_H_

#include <stdint.h>
#include <stdbool.h>

void uart_transmit(uint8_t tx);

bool uart_byte_available(void);

uint8_t uart_read_byte(void);

#endif	/* UART_H */

