#include <xc.h>
#include <pic18f26k83.h>
#include "init.h"

/*
 * Set up all Analog select (ANSEL), Latch/Port (LAT), and Tristate (TRIS)
 * registers for all pins on the device. comments appear beside each assignment
 * to show what is connected to that pin.
 */
void init_pins(void)
{
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
               0 << 5 | //NC
               0 << 4 | //NC
               0 << 3 | //CANTX
               0 << 2 | //CANRX
               0 << 1 | //Debug LED 1
               0 << 0); //NC
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

    //setup CAN output pins
    //CANRX on RC2
    CANRXPPS = 0b010010;

    //CANTX on RC3
    RC3PPS = 0b110011;

}

/*
 * Initialize the oscillator
 * Blocks while it tries to run off of the external oscillator.
 */
void init_oscillator(void)
{
    //Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    //wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)  {}
    //if the currently active clock (CON2) isn't the selected clock (CON1)
    if (OSCCON2 != 0b01110000) {
        //infinite loop, something is broken, what even is an assert()?
        while (1) {}
    }
}

/*
 * Initializes all necessary registers for the adc module
 * Currently only sets up for reading the battery voltage from pin ANA3
 */
void init_adc(void)
{
    //enable fixed voltage reference
    FVRCONbits.EN = 1;
    //set the voltage reference to be 4.096V on both outputs
    FVRCONbits.CDAFVR = 0b11;
    FVRCONbits.ADFVR  = 0b11;
    //disable the temperature indicator.... for now....
    FVRCONbits.TSEN = 0;
    //wait for the FVR to stabilize
    while (FVRCONbits.RDY == 0) {}

    //turn on the ADC
    ADCON0bits.ON = 1;

    //disable repeating operations
    // if this is set to 1, as soon as ADC finished a reading it starts another
    ADCON0bits.CONT = 0;

    //use system clock as ADC timer. Divider set by ADCLK
    ADCON0bits.CS = 0;

    //this assumes FOSC is 12MHz this value sets ADC clock period to
    // 1.5uS (Fosc/16). Before you change this number, please check
    // datasheet table 37-1
    ADCLK = 0b000111;

    //right justify the 12 bit output of the ADC.  if this value is 0,
    // the top 8 bits of read value are put in ADRESH, and the bottom
    // 4 bits are put in the top 4 bits of ADRESL. In this mode,
    // bottom 8 bits are in ADRESL, top 4 are in bottom 4 bits of
    // ADRESH.
    ADCON0bits.FM = 1;

    //set the references
    // negative reference is ground
    ADREFbits.NREF = 0; //1 would set to external Vref-
    // positive reference to internal FVR module
    ADREFbits.PREF = 0b11;
}


void init_uart(void)
{
    //set RX pin location
    U1RXPPS = (0b001 << 3) | //port B
              (0b100);       // pin 4
    //set CTS pin location
    U1CTSPPS = (0b001 << 3) |
               (0b001);

    //set the TX pin location
    RB3PPS = 0b010011;
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
}

void init_timer0(void)
{
    //disable the module so we can screw with it
    T0CON0bits.EN = 0;
    //set timer up to be an 8 bit timer
    T0CON0bits.MD16 = 0;
    //set the pre and postscalars to 0. Because I don't know what they do
    T0CON0bits.OUTPS = 0;
    T0CON1bits.CKPS = 0;
    //drive the timer from 500 kHz internal oscillator
    T0CON1bits.CS = 0x5;
    T0CON1bits.ASYNC = 0;

    //enable the module
    T0CON0bits.EN = 1;
}

/*
 * Turn on all the interrupts that we want
 * If you want a new interrupt, please turn it on here.
 */
void init_interrupts(void)
{
    //enable global interrupts
    INTCON0bits.GIE = 1;
    //disable interrupt priorities. Another thing we could be fancy about
    INTCON0bits.IPEN = 0;

    //enable ADC interrupt
    PIE1bits.ADIE = 1;

    //enable timer 0 interrupt
    PIE3bits.TMR0IE = 1;


    //at present we are not using vectored interrupts. If we were being fancy,
    //we would be using vectored interrupts. If you feel like being fancy and
    //adding that functionality, the register you want to screw with is IVTBASE
}
