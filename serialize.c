#include "serialize.h"

bool serialize_state(const system_state *state, char *str) {
    if(!state) return false;
    if(!str) return false;

    str[0] = 0b01100001;
    str[1] = 0b01100001;
    str[2] = 0;

    // num_boards_connected will occupy bits 3-5 in char 0.
    // Since we have less than 8 boards, we will only need 3 bits
    // to encode its value.
    str[0] |= (state->num_boards_connected & 0b00000111) << 2;
    if(state->injector_valve_open) str[0] |= 0b00000010;
    if(state->vent_valve_open) str[1] |= 0b00010000;
    if(state->running_self_test) str[1] |= 0b00001000;
    if(state->any_errors_detected) str[1] |= 0b00000100;

    return true;
}

bool deserialize_state(system_state *state, const char *str) {
    if(!state) return false;
    if(!str) return false;

    state->num_boards_connected = (str[0] & 0b00011100) >> 2;
    state->injector_valve_open = str[0] & 0b00000010;
    state->vent_valve_open = str[1] & 0b00010000;
    state->running_self_test = str[1] & 0b00001000;
    state->any_errors_detected = str[1] & 0b00000100;

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
