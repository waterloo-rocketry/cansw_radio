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
#include "bus_power.h"

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

        // We check for CAN messages regardles of whether the bus is powered.
        // It's possible that the debug header is trying to tell us something,
        // and we should really listen to that
        if (buffered_received_can_message_available()) {
            can_msg_t msg;
            get_buffered_can_message(&msg);
            handle_incoming_can_message(&msg);
        }

        if (is_bus_powered()) {
            // There's no sense in sending CAN messages if the bus isn't
            // powered. There's no one to hear them
            sotscon_heartbeat();
        } else {
            // TODO, what should the radio board do while the bus is powered
            // down? The ADC stuff I guess?
        }

        bus_power_heartbeat();
    }

    //unreachable
    return 0;
}
