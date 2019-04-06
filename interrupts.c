#include "interrupts.h"
#include "analog.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "pic18_time.h"
#include "uart.h"
#include <xc.h>
#include <stdint.h>

/*
 * Interrupt service routine for adc finishing.
 */
static void adc_isr()
{
    //assume that there is a completed value in ADRES{H,L}
    uint16_t read_value = ADRESL;
    read_value |= (((uint16_t) ADRESH) << 8);

    //check which port we just read this off of
    uint8_t channel = ADPCH;

    //call the analog state machine with this newly received value
    analog_report_value(read_value, channel);
}

void __interrupt() isr()
{
    //check if it was the ADC
    if ( PIE1bits.ADIE == 1 && PIR1bits.ADIF == 1) {
        adc_isr();
        PIR1bits.ADIF = 0;
    } else if (PIR5) {
        //something has happened to do with CAN, let the CAN driver handle it
        can_handle_interrupt();
    } else if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    } else if (PIR3 & 0x78) {
        //it's one of the UART1 interrupts, let them handle it
        uart_interrupt_handler();
        //that function is responsible for clearing PIR3
    } else {
        //unhandled interrupt. No idea what to do here, so just infinite loop
        while (1);
    }
}
