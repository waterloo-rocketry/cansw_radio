#include "serialize.h"

char binary_to_base64(uint8_t binary) {
    if(binary <= 25) return binary + 'A';
    if(binary <= 51) return (binary - 26) + 'a';
    if(binary <= 61) return (binary - 52) + '0';
    if(binary == 62) return '&';
    return '/';
}

uint8_t base64_to_binary(char base64) {
    if('A' <= base64 && base64 <= 'Z') return base64 - 'A';
    if('a' <= base64 && base64 <= 'z') return base64 - 'a' + 26;
    if('0' <= base64 && base64 <= '9') return base64 - '0' + 52;
    if(base64 == '&') return 62;
    return 63;
}

bool serialize_state(const system_state *state, char *str) {
    if(!state) return false;
    if(!str) return false;

    uint8_t raw = 0;
    // Bits 5-2 represent the number of boards connected
    raw |= (state->num_boards_connected & 0b00001111) << 2;
    // Bit 1 represents the injector valve state
    if(state->injector_valve_open) raw |= 0b00000010;
    // Bit 0 represents the vent valve state
    if(state->vent_valve_open) raw |= 0b00000001;
    str[0] = binary_to_base64(raw);

    raw = 0;
    // Bit 5 represents whether self-testing is enabled
    if(state->running_self_test) raw |= 0b00100000;
    // Bit 4 represents whether errors have been detected
    if(state->any_errors_detected) raw |= 0b00010000;
    str[1] = binary_to_base64(raw);

    str[2] = 0;

    return true;
}

bool deserialize_state(system_state *state, const char *str) {
    if(!state) return false;
    if(!str) return false;

    uint8_t raw = base64_to_binary(str[0]);

    // Bits 5-2 represent the number of boards connected
    state->num_boards_connected = (raw & 0b00111100) >> 2;
    // Bit 1 represents the injector valve state
    state->injector_valve_open = raw & 0b00000010;
    // Bit 0 represents the vent valve state
    state->vent_valve_open = raw & 0b00000001;

    raw = base64_to_binary(str[1]);

    // Bit 5 represents whether self-testing is enabled
    state->running_self_test = raw & 0b00100000;
    // Bit 4 represents whether errors have been detected
    state->any_errors_detected = raw & 0b00010000;

    return true;
}

bool compare_system_states(const system_state *s, const system_state *p) {
    if(!s) return false;
    if(!p) return false;

    if(s->num_boards_connected != p->num_boards_connected) return false;
    if(s->injector_valve_open != p->injector_valve_open) return false;
    if(s->vent_valve_open != p->vent_valve_open) return false;
    if(s->running_self_test != p->running_self_test) return false;
    if(s->any_errors_detected != p->any_errors_detected) return false;

    return true;
}

bool create_state_command(char *cmd, const system_state *state) {
    if(cmd == 0) return false;
    if(state == 0) return false;

    char serialized[SERIALIZED_OUTPUT_LEN];
    if(!serialize_state(state, serialized)) return false;

    // Do not copy the null terminator of the serialized string
    memcpy(cmd + 1, serialized, SERIALIZED_OUTPUT_LEN - 1);

    // Message start indicator
    cmd[0] = '{';
    // Checksum
    cmd[STATE_COMMAND_LEN - 2] = checksum(serialized);
    // Null terminator
    cmd[STATE_COMMAND_LEN - 1] = 0;

    return true;
}

char checksum(char *cmd) {
    uint8_t total = 0;
    uint8_t idx = 0;
    while(cmd[idx] != 0) {
        char curr = cmd[idx];
        uint8_t odd_sum = 0b10101010 & curr, even_sum = 0b01010101 & curr;
        total += odd_sum + 3 * even_sum;
        ++idx;
    }
    total %= 64;
    return binary_to_base64(total);
}
