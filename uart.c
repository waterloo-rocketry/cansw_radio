#include <pic18f26k83.h>

#include "uart.h"

void uart_transmit(uint8_t tx) {
    while(!U1ERRIRbits.TXMTIF) {}
    U1TXB = tx;
    U1CON0bits.TXEN = 1;
}

bool uart_byte_available(void) {
    return PIR3bits.U1RXIF == 1;
}

uint8_t uart_read_byte() {
    uint8_t rec = U1RXB;
    return rec;
}
