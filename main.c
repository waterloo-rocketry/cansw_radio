#include "config.h"
#include "init.h"
#include "platform.h"
#include "analog.h"
#include "uart.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "pic18_time.h"

#include <string.h>

void can_message_callback(const can_msg_t *msg)
{
    if ((msg->sid & 0x7E0) == 0x7E0) {
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

int main()
{
    //initialization functions
    init_pins();
    init_oscillator();
    init_adc();
    init_timer0();
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
    while (1) {
    };

    LED_3_OFF();

    //program loop
    uint32_t led_counter = 0;
    while (1) {
        if (millis() - led_counter < 1000) {
            LED_1_ON();
            LED_2_OFF();
            LED_3_OFF();
        } else if (millis() - led_counter < 2000) {
            LED_1_OFF();
            LED_2_ON();
            LED_3_OFF();
        } else if (millis() - led_counter < 3000) {
            LED_1_OFF();
            LED_2_OFF();
            LED_3_ON();
        } else {
            led_counter = millis();
        }
    }

    //unreachable
    return 0;
}
