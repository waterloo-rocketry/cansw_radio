#include <xc.h>
#include "init.h"

/*
 * Set up all Analog select (ANSEL), Latch/Port (LAT), and Tristate (TRIS)
 * registers for all pins on the device. comments appear beside each assignment
 * to show what is connected to that pin.
 */
void init_pins() {
    //starting with port A
    ANSELA = ( 0 << 7 | //OSC1 pin
               0 << 6 | //OSC2 pin
               0 << 5 | //Debug led 3
               0 << 4 | //Debug led 2
               1 << 3 | //Battery VSense
               0 << 2 | //BUS_enable line
               1 << 1 | //curr_battery line
               1 << 0); //curr_bus line
    LATA   = ( 0 << 7 | //OSC1 pin
               0 << 6 | //OSC2 pin
               0 << 5 | //Debug led 3. Off at startup
               1 << 4 | //Debug led 2. Off at startup
               0 << 3 | //Battery VSense
               0 << 2 | //BUS_enable line. Off at startup
               0 << 1 | //curr_battery line
               0 << 0); //curr_bus line
    TRISA  = ( 1 << 7 | //OSC1 pin
               1 << 6 | //OSC2 pin
               0 << 5 | //Debug led 3
               0 << 4 | //Debug led 2
               1 << 3 | //Battery VSense
               0 << 2 | //BUS_enable line
               1 << 1 | //curr_battery line
               1 << 0); //curr_bus line
    //now port B
    ANSELB = ( 1 << 7 | //ICSPDAT (otherwise unused)
               1 << 6 | //ICSPCLK (otherwise unused)
               1 << 5 | //NC pin
               0 << 4 | //UART_RX
               0 << 3 | //UART_TX
               0 << 2 | //~UART_RTS
               0 << 1 | //~UART_CTS
               1 << 0); //NC pin
    LATB   = ( 0 << 7 | //ICSPDAT (otherwise unused)
               0 << 6 | //ICSPCLK (otherwise unused)
               0 << 5 | //NC pin
               0 << 4 | //UART_RX
               0 << 3 | //UART_TX
               1 << 2 | //~UART_RTS
               0 << 1 | //~UART_CTS
               0 << 0); //NC pin
    TRISB  = ( 1 << 7 | //ICSPDAT (otherwise unused)
               1 << 6 | //ICSPCLK (otherwise unused)
               1 << 5 | //NC pin
               1 << 4 | //UART_RX
               0 << 3 | //UART_TX
               0 << 2 | //~UART_RTS
               1 << 1 | //~UART_CTS
               1 << 0); //NC pin
    //now port C
    ANSELC = ( 0 << 7 | //XBEE_RESET
               0 << 6 | //XBEE_SLEEP
               1 << 5 | //NC
               1 << 4 | //NC
               0 << 3 | //CANTX
               0 << 2 | //CANRX
               0 << 1 | //Debug LED 1
               1 << 0); //NC
    LATC   = ( 0 << 7 | //XBEE_RESET
               0 << 6 | //XBEE_SLEEP
               0 << 5 | //NC
               0 << 4 | //NC
               0 << 3 | //CANTX
               0 << 2 | //CANRX
               0 << 1 | //Debug LED 1
               0 << 0); //NC
    TRISC  = ( 1 << 7 | //XBEE_RESET
               1 << 6 | //XBEE_SLEEP
               1 << 5 | //NC
               1 << 4 | //NC
               0 << 3 | //CANTX
               1 << 2 | //CANRX
               0 << 1 | //Debug LED 1
               1 << 0); //NC
}

/*
 * Initialize the oscillator
 * Blocks while it tries to run off of the external oscillator.
 */
void init_oscillator() {
    //Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    //wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)  {}
    //if the currently active clock (CON2) isn't the selected clock (CON1)
    if(OSCCON2 != 0b01110000) {
        //infinite loop, something is broken, what even is an assert()?
        while(1) {}
    }
}

