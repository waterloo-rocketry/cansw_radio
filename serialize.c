#include "serialize.h"
#include <string.h>
#include <stddef.h> // for NULL

char binary_to_base64(uint8_t binary) {
    if(binary <= 25) return binary + 'A';
    if(binary <= 51) return (binary - 26) + 'a';
    if(binary <= 61) return (binary - 52) + '0';
    if(binary == 62) return '&';
    if(binary == 63) return '/';
    // ***TODO***: return an error in all other cases
    return 0;
}

uint8_t base64_to_binary(char base64) {
    if('A' <= base64 && base64 <= 'Z') return base64 - 'A';
    if('a' <= base64 && base64 <= 'z') return base64 - 'a' + 26;
    if('0' <= base64 && base64 <= '9') return base64 - '0' + 52;
    if(base64 == '&') return 62;
    if(base64 == '/') return 63;
    // ***TODO***: return an error in all other cases
    return 255;
}

bool serialize_state(const system_state *state, char *str) {
    if(state == NULL) return false;
    if(str == NULL) return false;

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
    if(state == NULL) return false;
    if(str == NULL) return false;

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

bool serialize_error(const error_t *err, char *str) {
    if(err == NULL) return false;
    if(str == NULL) return false;

    // A temporary variable to store the values to be Base64-encoded
    uint8_t temp = 0;

    // The board ID is is given 4 bits
    temp = (err->board_id & 0b00001111) << 2;
    // The error type is given 6 bits
    temp |= (err->err_type & 0b00110000) >> 4;
    str[0] = binary_to_base64(temp);

    temp = (err->err_type & 0b00001111) << 2;
    temp |= (err->byte4 & 0b11000000) >> 6;
    str[1] = binary_to_base64(temp);

    temp = (err->byte4 & 0b00111111);
    str[2] = binary_to_base64(temp);

    temp = (err->byte5 & 0b11111100) >> 2;
    str[3] = binary_to_base64(temp);

    temp = (err->byte5 & 0b00000011) << 4;
    temp |= (err->byte6 & 0b11110000) >> 4;
    str[4] = binary_to_base64(temp);

    temp = (err->byte6 & 0b00001111) << 2;
    temp |= (err->byte7 & 0b11000000) >> 6;
    str[5] = binary_to_base64(temp);

    temp = (err->byte7 & 0b00111111);
    str[6] = binary_to_base64(temp);

    str[7] = 0;

    return true;
}

bool deserialize_error(error_t *err, const char *str) {
    if(err == NULL) return false;
    if(str == NULL) return false;

    // A temporary variable to store decoded values
    uint8_t temp = 0;

    temp = base64_to_binary(str[0]);
    err->board_id = (temp & 0b00111100) >> 2;
    err->err_type = (temp & 0b00000011) << 4;

    temp = base64_to_binary(str[1]);
    err->err_type |= (temp & 0b00111100) >> 2;
    err->byte4 = (temp & 0b00000011) << 6;

    temp = base64_to_binary(str[2]);
    err->byte4 |= (temp & 0b00111111);

    temp = base64_to_binary(str[3]);
    err->byte5 = (temp & 0b00111111) << 2;

    temp = base64_to_binary(str[4]);
    err->byte5 |= (temp & 0b00110000) >> 4;
    err->byte6 = (temp & 0b00001111) << 4;

    temp = base64_to_binary(str[5]);
    err->byte6 |= (temp & 0b00111100) >> 2;
    err->byte7 = (temp & 0b00000011) << 6;

    temp = base64_to_binary(str[6]);
    err->byte7 |= (temp & 0b00111111);

    return true;
}

bool compare_system_states(const system_state *s, const system_state *p) {
    if(s == NULL) return false;
    if(p == NULL) return false;

    if(s->num_boards_connected != p->num_boards_connected) return false;
    if(s->injector_valve_open != p->injector_valve_open) return false;
    if(s->vent_valve_open != p->vent_valve_open) return false;
    if(s->running_self_test != p->running_self_test) return false;
    if(s->any_errors_detected != p->any_errors_detected) return false;

    return true;
}

bool create_state_command(char *cmd, const system_state *state) {
    if(cmd == NULL) return false;
    if(state == NULL) return false;

    char serialized[SERIALIZED_OUTPUT_LEN];
    if(!serialize_state(state, serialized)) return false;

    // Do not copy the null terminator of the serialized string
    memcpy(cmd + 1, serialized, SERIALIZED_OUTPUT_LEN - 1);

    // Message start indicator
    cmd[0] = STATE_COMMAND_HEADER;
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
        uint8_t curr = (uint8_t) cmd[idx];
        uint8_t odd_sum = 0;
        uint8_t even_sum = 0;
        for(uint8_t i = 0; i < 4; ++i) {
            odd_sum += 0b00000010 & curr;
            even_sum += 0b00000001 & curr;
            curr = curr >> 2;
        }
        total += odd_sum + 3 * even_sum;
        ++idx;
    }
    total %= 64;
    return binary_to_base64(total);
}
