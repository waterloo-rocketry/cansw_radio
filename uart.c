#include <pic18f26k83.h>

#include "uart.h"
#include "safe_ring_buffer.h"
#include "error.h"
#include "message_types.h"

//needed for sending all data out over CAN as well as UART
#include "can.h"
#include "can_common.h"
#include "can_tx_buffer.h"

//safe ring buffers for sending and receiving
static srb_ctx_t rx_buffer;
static srb_ctx_t tx_buffer;

//memory pools to use for those srbs. 100 is a completely arbitrary number
uint8_t rx_buffer_pool[100], tx_buffer_pool[100];

void init_uart(void)
{
    //set RX pin location
    U1RXPPS = (0b001 << 3) | //port B
              (0b011);       // pin 3
    //set CTS pin location
    U1CTSPPS = (0b001 << 3) |
               (0b001);

    //set the TX pin location
    RB4PPS = 0b010011;
    //set the RTS pin location
    RB2PPS = 0b010101;

    //normal speed, not high speed
    U1CON0bits.BRGS = 0;
    //don't autodetect baudrate
    U1CON0bits.ABDEN = 0;
    //normal mode (8 bit, no parity, no 9th bit)
    U1CON0bits.MODE = 0;
    //enable transmit
    U1CON0bits.TXEN = 1;
    //enable receive
    U1CON0bits.RXEN = 1;

    //keep running on overflow, never stop receiving
    U1CON2bits.RUNOVF = 1;
    //disable flow control.... for now...
    U1CON2bits.FLO = 0;

    //baud rate stuff, divide by 78 to get 9600 baud
    U1BRGL = 77;
    U1BRGH = 0;

    //we are go for UART
    U1CON1bits.ON = 1;

    //initialize the rx and tx buffers
    srb_init(&rx_buffer, rx_buffer_pool, sizeof(rx_buffer_pool), sizeof(uint8_t));
    srb_init(&tx_buffer, tx_buffer_pool, sizeof(tx_buffer_pool), sizeof(uint8_t));

    //enable receive interrupt
    PIE3bits.U1RXIE = 1;
    //Do not enable transmit interrupt, that interrupt enable signals that
    //there is data to be sent, which at init time is not true
}

void uart_transmit_byte(uint8_t tx)
{
    //push this byte to ensure ordering
    srb_push(&tx_buffer, &tx);
    //If the module isn't enabled, give it a byte to send and enable it
    if (PIE3bits.U1TXIE == 0) {
        srb_pop(&tx_buffer, &tx);
        U1TXB = tx;
        U1CON0bits.TXEN = 1;
        //also enable the interrupt for when it's ready to send more data
        PIE3bits.U1TXIE = 1;
    }

    //we want to send all of the bytes that we're sending over radio over
    //CAN as well. To do so we buffer these bytes until we have 8 of them,
    //then we send them all at once. Yes this means that a byte could be
    //not delivered for a while, but it's fine, since this is only for debug,
    //and we don't really care about latency
    static char debug_printf_data[9];
    static uint8_t debug_printf_data_len = 0;
    debug_printf_data[debug_printf_data_len++] = (char) tx;
    if (debug_printf_data_len == 8) {
        debug_printf_data[8] = '\0';
        can_msg_t to_send;
        build_printf_can_message(debug_printf_data, &to_send);
        txb_enqueue(&to_send);
    }
}

void uart_transmit_buffer(uint8_t *tx, uint8_t len)
{
    //just call uart_transmit_byte with each byte they want us to send
    while (len--) {
        uart_transmit_byte(*tx);
        tx++;
    }
}

bool uart_byte_available(void)
{
    return !srb_is_empty(&rx_buffer);
}

uint8_t uart_read_byte(void)
{
    uint8_t rcv;
    srb_pop(&rx_buffer, &rcv);
    return rcv;
}

void uart_interrupt_handler(void)
{
    if (PIR3bits.U1TXIF) {
        //check if there are any bytes we still want to transmit
        if (!srb_is_empty(&tx_buffer)) {
            //if so, transmit them
            uint8_t tx;
            srb_pop(&tx_buffer, &tx);
            U1TXB = tx;
        } else {
            // If we have no data to send, disable this interrupt
            PIE3bits.U1TXIE = 0;
            //if not, disable the TX part of the uart module so that TXIF
            //isn't triggered again and so that we reenable the module on
            //the next call to uart_transmit_byte
            U1CON0bits.TXEN = 0;
        }
        PIR3bits.U1TXIF = 0;
    } else if (PIR3bits.U1RXIF) {
        //we received a byte, need to push into RX buffer and return
        uint8_t rcv = U1RXB;
        srb_push(&rx_buffer, &rcv);
        PIR3bits.U1RXIF = 0;
    } else if (PIR3bits.U1EIF) {
        //report the error, don't do anything else
        report_error(BOARD_UNIQUE_ID, E_CODING_FUCKUP, U1ERRIR, 0, 0, 0);
        PIR3bits.U1EIF = 0;
    } else if (PIR3bits.U1IF) {
        PIR3bits.U1IF = 0;
    }
}
