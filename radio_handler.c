#include "radio_handler.h"
#include "serialize.h"
#include "sotscon.h"
#include "uart.h"
#include "pic18_time.h" // for millis()
#include "bus_power.h"
#include <string.h> // for memcpy

static enum VALVE_STATE inj_valve_state = VALVE_UNK;
static enum VALVE_STATE vent_valve_state = VALVE_OPEN;


static uint32_t last_contact_millis = 0;

enum VALVE_STATE radio_get_expected_inj_valve_state(void)
{
    return inj_valve_state;
}

enum VALVE_STATE radio_get_expected_vent_valve_state(void)
{
    // Return VALVE_OPEN, regardless of vent_valve_state,
    // if we haven't received any complete messages in a while
    uint32_t time_since_last_contact = millis() - last_contact_millis;
    if (time_since_last_contact >= TIME_NO_CONTACT_BEFORE_SAFE_STATE) {
        return VALVE_OPEN;
    }
    return vent_valve_state;
}

void radio_handle_input_character(uint8_t c)
{
    static char message[STATE_COMMAND_LEN] = {0};
    static uint8_t chars_received = 0;

    if (c == STATE_REQUEST_HEADER) {
        //we need to serialize our current state and send it over the radio
        char state_to_send[STATE_COMMAND_LEN];
        system_state current_state;
        current_state.tank_pressure = current_tank_pressure();
        current_state.num_boards_connected = current_num_boards_connected();
        current_state.injector_valve_open = (current_inj_valve_position() ==
                                             VALVE_OPEN);
        current_state.vent_valve_open = (current_vent_valve_position() == VALVE_OPEN);
        current_state.bus_is_powered = is_bus_powered();
        current_state.any_errors_detected = any_errors_active();
        create_state_command(state_to_send, &current_state);
        uart_transmit_buffer((uint8_t *) state_to_send, STATE_COMMAND_LEN - 1);
    } else if (c == STATE_COMMAND_HEADER) {
        chars_received = 1;
        message[0] = STATE_COMMAND_HEADER;
    } else if (chars_received == STATE_COMMAND_LEN - 2) {
        // Only STATE_COMMAND_LEN - 1 characters are actually transmitted
        // in each message (the null terminator is left out).
        // This means that this character is the checksum.

        // The checksum only includes the serialized state bytes.
        // As such, we shouldn't include the { in it.
        char actual_checksum = checksum(message + 1);
        if (actual_checksum == c) {
            // The message was correctly received
            char serialized[SERIALIZED_OUTPUT_LEN];
            memcpy(serialized, message + 1, SERIALIZED_OUTPUT_LEN - 1);
            serialized[SERIALIZED_OUTPUT_LEN - 1] = 0;
            system_state state;
            deserialize_state(&state, serialized);
            inj_valve_state = state.injector_valve_open ? VALVE_OPEN
                              : VALVE_CLOSED;
            vent_valve_state = state.vent_valve_open ? VALVE_OPEN
                               : VALVE_CLOSED;
            /* control whether the bus is powered */
            if(state.bus_is_powered) {
                trigger_bus_powerup();
            } else {
                trigger_bus_shutdown();
            }

            last_contact_millis = millis();
        } else {
            // Discard this message
            chars_received = 0;
        }
    } else {
        // This character is in the middle of a message.
        // Check that we've received the starting character first.
        if (chars_received > 0) {
            message[chars_received] = c;
            ++chars_received;
        }
        // Otherwise, simply discard the character.
    }
}

void radio_heartbeat(void)
{
    //if we have an error message ready to send, and it's been longer than 1
    //second since we last sent an error message, then send that error message.
    static uint32_t time_last_error_msg_sent = 0;
    if (millis() - time_last_error_msg_sent > 1000) {
        //the +2 is to add a error_command_header and a checksum
        char error_msg_to_send[ERROR_COMMAND_LENGTH + 2];
        if (get_next_serialized_error(error_msg_to_send + 1)) {
            error_msg_to_send[0] = ERROR_COMMAND_HEADER;
            error_msg_to_send[ERROR_COMMAND_LENGTH] = '\0'; //make sure doesn't read past here
            error_msg_to_send[ERROR_COMMAND_LENGTH] = checksum(error_msg_to_send);
            error_msg_to_send[ERROR_COMMAND_LENGTH + 1] = '\0';
            uart_transmit_buffer((uint8_t *) error_msg_to_send, ERROR_COMMAND_LENGTH + 1);
            time_last_error_msg_sent = millis();
        }
    }
}
