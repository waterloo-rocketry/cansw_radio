#include "radio_handler.h"
#include "serialize.h"
#include <string.h> // for memcpy

static enum VALVE_STATE inj_valve_state = VALVE_UNK;
static enum VALVE_STATE vent_valve_state = VALVE_OPEN;

enum VALVE_STATE radio_get_expected_inj_valve_state(void)
{
    return inj_valve_state;
}

enum VALVE_STATE radio_get_expected_vent_valve_state(void)
{
    return vent_valve_state;
}

void radio_handle_input_character(uint8_t c)
{
    static char message[STATE_COMMAND_LEN] = {0};
    static uint8_t chars_received = 0;

    if(c == STATE_COMMAND_HEADER) {
        chars_received = 1;
        message[0] = STATE_COMMAND_HEADER;
    }

    else if(chars_received == STATE_COMMAND_LEN - 2) {
        // Only STATE_COMMAND_LEN - 1 characters are actually transmitted
        // in each message (the null terminator is left out).
        // This means that this character is the checksum.

        // The checksum only includes the serialized state bytes.
        // As such, we shouldn't include the { in it.
        char actual_checksum = checksum(message + 1);
        if(actual_checksum == c) {
            // The message was correctly received
            char serialized[SERIALIZED_OUTPUT_LEN];
            memcpy(serialized, message + 1, SERIALIZED_OUTPUT_LEN - 1);
            serialized[SERIALIZED_OUTPUT_LEN - 1] = 0;
            system_state state;
            deserialize_state(&state, serialized);
            inj_valve_state = state.injector_valve_open ? VALVE_OPEN
                : VALVE_CLOSED;
            vent_valve_state = state.vent_valve_open ? VALVE_OPEN : VALVE_CLOSED;
        }
        else {
            // Discard this message
            chars_received = 0;
        }
    }

    else {
        // This character is in the middle of a message.
        // Check that we've received the starting character first.
        if(chars_received > 0) {
            message[chars_received] = c;
            ++chars_received;
        }
        // Otherwise, simply discard the character.
    }
}
