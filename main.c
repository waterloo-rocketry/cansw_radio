#include "config.h"
#include "init.h"
#include "platform.h"
#include "analog.h"
#include "uart.h"
#include "pic18f26k83_can.h"
#include "buffer_received_can_message.h"
#include "can_common.h"
#include "pic18_time.h"
#include "sotscon.h"
#include "sotscon_sender.h"
#include "radio_handler.h"

#include <string.h>

void can_message_callback(const can_msg_t *msg)
{
    uint16_t message_type = get_message_type(msg);
    if (message_type == MSG_LEDS_ON) {
        LED_1_ON();
        LED_2_ON();
        LED_3_ON();
    } else if (message_type == MSG_LEDS_OFF) {
        LED_1_OFF();
        LED_2_OFF();
        LED_3_OFF();
    }

    buffer_received_can_message(msg);
}

//enough space to buffer 10 CAN messages
uint8_t can_receive_buffer[140];

int main()
{
    //initialization functions
    init_pins();
    init_oscillator();
    init_adc();
    init_timer0();
    init_interrupts();
    init_uart();
    init_sotscon();
    receive_buffer_init(can_receive_buffer, sizeof(can_receive_buffer));

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

    LED_3_OFF();

    //program loop
    while (1) {
        if (uart_byte_available()) {
            radio_handle_input_character(uart_read_byte());
        }

        if (buffered_received_can_message_available()) {
            can_msg_t msg;
            get_buffered_can_message(&msg);
            handle_incoming_can_message(&msg);
        }

        sotscon_heartbeat();
    }

    //unreachable
    return 0;
}
