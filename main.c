#include "config.h"
#include "init.h"
#include "platform.h"
#include "analog.h"
#include "uart.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"

#include <string.h>

void can_message_callback(const can_msg_t *msg) {
    if((msg->sid & 0x7E0) == 0x7E0) {
        LED_1_ON();
        LED_2_ON();
        LED_3_ON();
    } else if ((msg->sid & 0x7E0) == 0x7C0) {
        LED_1_OFF();
        LED_2_OFF();
        LED_3_OFF();
    } else {
        //someday....
    }
}

int main() {
    //initialization functions
    init_pins();
    init_oscillator();
    init_adc();
    init_interrupts();
    init_uart();

    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();

    can_timing_t timing;
    timing.brp = 11;
    timing.sjw = 3;
    timing.sam = 0;
    timing.seg1ph = 4;
    timing.seg2ph = 4;
    timing.prseg = 0;
    timing.btlmode = true;

    can_init(&timing, &can_message_callback);

    can_msg_t msg;
    msg.sid = 0xAA;
    msg.data_len = 1;
    msg.data[0] = 0xAA;
    msg.data[1] = 0xAA;
    while(1) {
    };

    LED_3_OFF();

    //program loop
    while(1) {
        if(uart_byte_available()) {
            char c = uart_read_byte();
            switch(c) {
            case '1':
                LED_1_ON();
                LED_2_OFF();
                LED_3_OFF();
                break;
            case '2':
                LED_1_OFF();
                LED_2_ON();
                LED_3_OFF();
                break;
            case '3':
                LED_1_OFF();
                LED_2_OFF();
                LED_3_ON();
                break;
            default:
                LED_1_OFF();
                LED_2_OFF();
                LED_3_OFF();
                break;
            }
        }
    }

    //unreachable
    return 0;
}
