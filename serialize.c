#include "serialize.h"
#include <string.h>
#include <stddef.h> // for NULL

char binary_to_base64(uint8_t binary)
{
    if (binary <= 25)
        return binary + 'A';
    if (binary <= 51)
        return (binary - 26) + 'a';
    if (binary <= 61)
        return (binary - 52) + '0';
    if (binary == 62)
        return '&';
    if (binary == 63)
        return '/';
    // ***TODO***: return an error in all other cases
    return 0;
}

uint8_t base64_to_binary(char base64)
{
    if ('A' <= base64 && base64 <= 'Z')
        return base64 - 'A';
    if ('a' <= base64 && base64 <= 'z')
        return base64 - 'a' + 26;
    if ('0' <= base64 && base64 <= '9')
        return base64 - '0' + 52;
    if (base64 == '&')
        return 62;
    if (base64 == '/')
        return 63;
    // ***TODO***: return an error in all other cases
    return 255;
}

bool serialize_state(const system_state *state, char *str)
{
    if (state == NULL)
        return false;
    if (str == NULL)
        return false;

    uint8_t raw = 0;
    // Bits 5-2 represent the number of boards connected
    raw |= (state->num_boards_connected & 0b00001111) << 2;
    // Bits 1-0 represent injector valve state
    raw |= (state->injector_valve_state & 0x3);
    str[0] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-4 represent vent valve state
    raw |= ((state->vent_valve_state & 0x3) << 4);
    // Bits 3-0 are the top bits 9-6 of the tank pressure
    raw |= ((state->tank_pressure >> 6) & 0xf);
    str[1] = binary_to_base64(raw);

    // Use all the bits of this next one to hold bits 5-0 of tank pressure
    raw = (state->tank_pressure & 0x3f);
    str[2] = binary_to_base64(raw);

    raw = 0;
    // Bit 5 represents whether the bus is should be powered
    if (state->bus_is_powered)
        raw |= 0b00100000;
    // Bit 4 represents whether errors have been detected
    if (state->any_errors_detected)
        raw |= 0b00010000;
    // Bits 3-0 are the top bits 13-10 of the bus battery voltage
    raw |= (state->bus_battery_voltage_mv >> 10) & 0xf;
    str[3] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 9-4 of the bus battery voltage
    raw = (state->bus_battery_voltage_mv >> 4) & 0x3f ;
    str[4] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-2 hold bits 3-0 of the bus battery voltage
    raw = (state->bus_battery_voltage_mv & 0xf) << 2;
    // Bits 1-0 hold bits 13-12 of the vent battery voltage
    raw |= (state->vent_battery_voltage_mv >> 12) & 0x3;
    str[5] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 11-6 of the vent battery voltage
    raw = (state->vent_battery_voltage_mv >> 6) & 0x3f;
    str[6] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 5-0 of the vent battery voltage
    raw = (state->vent_battery_voltage_mv & 0x3f);
    str[7] = binary_to_base64(raw);

    str[8] = '\0';

    return true;
}

bool deserialize_state(system_state *state, const char *str)
{
    if (state == NULL)
        return false;
    if (str == NULL)
        return false;

    uint8_t raw = base64_to_binary(str[0]);

    // Bits 5-2 represent the number of boards connected
    state->num_boards_connected = (raw & 0b00111100) >> 2;
    // Bits 1-0 represent injector valve state
    state->injector_valve_state = raw & 0x3;

    raw = base64_to_binary(str[1]);

    // Bits 5-4 represent vent valve state
    state->vent_valve_state = ((raw & 0x30) >> 4);
    // Bit 3-0 are bits 9-6 of tank pressure
    state->tank_pressure = ((uint16_t) (raw & 0xf)) << 6;

    raw = base64_to_binary(str[2]);
    state->tank_pressure |= (raw & 0x3f);

    raw = base64_to_binary(str[3]);
    // Bit 5 represents whether the bus should be powered
    state->bus_is_powered = raw & 0x20;
    // Bit 4 represents whether any errors are active
    state->any_errors_detected = raw & 0x10;
    // Bits 3-0 represent the top 13-10 bits of the bus battery voltage
    state->bus_battery_voltage_mv = (uint16_t) (raw & 0xf) << 10;

    raw = base64_to_binary(str[4]);
    // Bits 5-0 represent bits 9-4 of the bus battery voltage
    state->bus_battery_voltage_mv |= (raw & 0x3f) << 4;

    raw = base64_to_binary(str[5]);
    // Bits 5-2 represent bits 3-0 of the bus battery voltage
    state->bus_battery_voltage_mv |= (raw & 0x3c) >> 2;
    // Bits 1-0 represent bits 13-12 of the vent battery voltage
    state->vent_battery_voltage_mv = (uint16_t) (raw & 0x3) << 12;

    raw = base64_to_binary(str[6]);
    // Bits 5-0 represent bits 11-6 of the vent battery voltage
    state->vent_battery_voltage_mv |= (raw & 0x3f) << 6;

    raw = base64_to_binary(str[7]);
    // Bits 5-0 represent bits 5-0 of the vent battery voltage
    state->vent_battery_voltage_mv |= raw & 0x3f;

    return true;
}

bool serialize_error(const error_t *err, char *str)
{
    if (err == NULL)
        return false;
    if (str == NULL)
        return false;

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

bool deserialize_error(error_t *err, const char *str)
{
    if (err == NULL)
        return false;
    if (str == NULL)
        return false;

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

bool create_gps_message(uint8_t latitude_deg,
                        uint8_t latitude_min,
                        uint8_t latitude_dmin,
                        uint8_t latitude_dir,
                        uint8_t longitude_deg,
                        uint8_t longitude_min,
                        uint8_t longitude_dmin,
                        uint8_t longitude_dir,
                        char *str)
{
    if (str == NULL ||
        (latitude_dir != 'N' && latitude_dir != 'S') ||
        (longitude_dir != 'E' && longitude_dir != 'W')) {
        return false;
    }

    str[0] = GPS_MSG_HEADER;

    str[1] = binary_to_base64((latitude_deg >> 2) & 0x3f);
    str[2] = binary_to_base64(((latitude_deg << 4) & 0x30) |
                              ((latitude_min >> 4) & 0xf));

    str[3] = binary_to_base64(((latitude_min << 2) & 0x3c) |
                              ((latitude_dmin >> 6) & 0x3));

    str[4] = binary_to_base64(latitude_dmin & 0x3f);
    str[5] = binary_to_base64((longitude_deg >> 2) & 0x3f);
    str[6] = binary_to_base64(((longitude_deg << 4) & 0x30) |
                              ((longitude_min >> 4) & 0xf));

    str[7] = binary_to_base64(((longitude_min << 2) & 0x3c) |
                              ((longitude_dmin >> 6) & 0x3));

    str[8] = binary_to_base64(longitude_dmin & 0x3f);
    str[9] = binary_to_base64((latitude_dir == 'N' ? 0x20 : 0) |
                              (longitude_dir == 'E' ? 0x10 : 0));

    // calculate checksum
    str[GPS_MSG_LEN - 1] = '\0';
    char cs = checksum(str);
    str[GPS_MSG_LEN - 1] = cs;

    return true;
}

bool expand_gps_message(uint8_t *latitude_deg,
                        uint8_t *latitude_min,
                        uint8_t *latitude_dmin,
                        uint8_t *latitude_dir,
                        uint8_t *longitude_deg,
                        uint8_t *longitude_min,
                        uint8_t *longitude_dmin,
                        uint8_t *longitude_dir,
                        char *str)
{
    if (str == NULL || str[0] != GPS_MSG_HEADER) {
        return false;
    }

    char actual_checksum = str[GPS_MSG_LEN - 1];
    str[GPS_MSG_LEN - 1] = '\0';
    char expected_checksum = checksum(str);
    if (expected_checksum != actual_checksum) {
        return false;
    }

    // 6 bits per char. So with 6 bytes, it's gonna take 8 characters
    *latitude_deg   = (base64_to_binary(str[1]) << 2) & 0xfc;
    *latitude_deg  |= (base64_to_binary(str[2]) >> 4) & 0x3;

    *latitude_min   = (base64_to_binary(str[2]) << 4) & 0xf0;
    *latitude_min  |= (base64_to_binary(str[3]) >> 2) & 0xf;

    *latitude_dmin  = (base64_to_binary(str[3]) << 6) & 0xc0;
    *latitude_dmin |= (base64_to_binary(str[4]))      & 0x3f;

    *longitude_deg   = (base64_to_binary(str[5]) << 2) & 0xfc;
    *longitude_deg  |= (base64_to_binary(str[6]) >> 4) & 0x3;

    *longitude_min   = (base64_to_binary(str[6]) << 4) & 0xf0;
    *longitude_min  |= (base64_to_binary(str[7]) >> 2) & 0xf;

    *longitude_dmin  = (base64_to_binary(str[7]) << 6) & 0xc0;
    *longitude_dmin |= (base64_to_binary(str[8]))      & 0x3f;

    // last char has lat_dir (1=='N', 0=='S') and lon_dir (1=='E', 0==W)
    *latitude_dir  = (base64_to_binary(str[9]) & 0x20) ? 'N' : 'S';
    *longitude_dir = (base64_to_binary(str[9]) & 0x10) ? 'E' : 'W';

    return true;
}

bool compare_system_states(const system_state *s, const system_state *p)
{
    if (s == NULL)
        return false;
    if (p == NULL)
        return false;

    if (s->num_boards_connected != p->num_boards_connected)
        return false;
    if (s->injector_valve_state != p->injector_valve_state)
        return false;
    if (s->vent_valve_state != p->vent_valve_state)
        return false;
    if (s->bus_is_powered != p->bus_is_powered)
        return false;
    if (s->any_errors_detected != p->any_errors_detected)
        return false;

    return true;
}

bool create_state_command(char *cmd, const system_state *state)
{
    if (cmd == NULL)
        return false;
    if (state == NULL)
        return false;

    char serialized[SERIALIZED_OUTPUT_LEN];
    if (!serialize_state(state, serialized))
        return false;

    // Do not copy the null terminator of the serialized string
    memcpy(cmd + 1, serialized, SERIALIZED_OUTPUT_LEN - 1);

    // Message start indicator
    cmd[0] = STATE_COMMAND_HEADER;
    // Checksum
    cmd[STATE_COMMAND_LEN - 2] = '\0';
    cmd[STATE_COMMAND_LEN - 2] = checksum(serialized);
    // Null terminator
    cmd[STATE_COMMAND_LEN - 1] = 0;

    return true;
}

char checksum(char *cmd)
{
    uint8_t total = 0;
    uint8_t idx = 0;
    while (cmd[idx] != 0) {
        uint8_t curr = (uint8_t) cmd[idx];
        uint8_t odd_sum = 0;
        uint8_t even_sum = 0;
        for (uint8_t i = 0; i < 4; ++i) {
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
