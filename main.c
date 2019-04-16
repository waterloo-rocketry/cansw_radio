#include "config.h"
#include "init.h"
#include "platform.h"
#include "analog.h"
#include "uart.h"
#include "pic18f26k83_can.h"
#include "can_rcv_buffer.h"
#include "can_common.h"
#include "pic18_time.h"
#include "sotscon.h"
#include "sotscon_sender.h"
#include "radio_handler.h"
#include "bus_power.h"
#include "timing_util.h"
#include "can_tx_buffer.h"

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

    rcvb_push_message(msg);
}

//enough space to buffer 10 CAN messages
uint8_t can_receive_buffer[140];
uint8_t can_transmit_buffer[140];

int main()
{
    //initialization functions
    init_pins();
    LED_1_ON();
    init_oscillator();
    LED_2_ON();
    init_adc();
    init_timer0();
    init_interrupts();
    init_uart();
    LED_3_ON();
    init_sotscon();
    rcvb_init(can_receive_buffer, sizeof(can_receive_buffer));
    txb_init(can_transmit_buffer, sizeof(can_transmit_buffer), &can_send, &can_send_rdy);

    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();

    can_timing_t timing;
    can_generate_timing_params(_XTAL_FREQ, &timing);
    can_init(&timing, &can_message_callback);

    LED_3_OFF();

    //program loop
    while (1) {
        if (uart_byte_available()) {
            radio_handle_input_character(uart_read_byte());
        }

        // We check for CAN messages regardless of whether the bus is powered.
        // It's possible that the debug board is trying to tell us something,
        // and we should really listen to that
        if (!rcvb_is_empty()) {
            can_msg_t msg;
            rcvb_pop_message(&msg);
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
        analog_heartbeat();
        txb_heartbeat();
        radio_heartbeat();
    }

    //unreachable
    return 0;
}
