#include "radio_handler.h"
#include "serialize.h"
#include <stdio.h>

//pic18_time.c depends on xc.h, so we can't use its millis function
//just declare a fake one, we don't really need it for these tests
uint32_t millis(void) { return 0; }

#define COLOR_GREEN "\x1B[32m"
#define COLOR_RED   "\x1B[31m"
#define COLOR_NONE  "\x1B[0m"

static int total_tests = 0;
static int failing_tests = 0;
#define UNIT_TEST(expected_result, description)                                 \
    if( (expected_result) ) {                                                   \
        printf("%sTest Passed:%s %s\n", COLOR_GREEN, COLOR_NONE, description);  \
    } else {                                                                    \
        printf("%sTest Failed:%s %s\n", COLOR_RED, COLOR_NONE, description);    \
        failing_tests++;                                                        \
    }                                                                           \
    total_tests++;

int main() {
    // Test that we can send a state update request, and it will update the valves
    char open_valves_command[STATE_COMMAND_LEN];
    system_state open_both_valves = {
        .injector_valve_open = true,
        .vent_valve_open = true,
        .running_self_test = false,
    };
    create_state_command(open_valves_command, &open_both_valves);
    // pass this state command into the radio handler
    uint8_t i;
    for (i = 0; i < STATE_COMMAND_LEN - 1; ++i) {
        radio_handle_input_character(open_valves_command[i]);
    }
    // make sure that both valves are open
    UNIT_TEST( (radio_get_expected_inj_valve_state() == VALVE_OPEN &&
                radio_get_expected_vent_valve_state() == VALVE_OPEN),
                "open both the valves");

    char close_valves_command[STATE_COMMAND_LEN];
    system_state close_both_valves = {
        .injector_valve_open = false,
        .vent_valve_open = false,
        .running_self_test = false,
    };
    create_state_command(close_valves_command, &close_both_valves);
    for (i = 0; i < STATE_COMMAND_LEN - 1; ++i) {
        radio_handle_input_character(close_valves_command[i]);
    }
    // make sure that both valves are closed
    UNIT_TEST( (radio_get_expected_inj_valve_state() == VALVE_CLOSED &&
                radio_get_expected_vent_valve_state() == VALVE_CLOSED),
                "close both the valves");

    // write half of the close command, then write the full open command
    for (i = 0; i < (STATE_COMMAND_LEN / 2); ++i) {
        radio_handle_input_character(close_valves_command[i]);
    }
    for (i = 0; i < STATE_COMMAND_LEN - 1; ++i) {
        radio_handle_input_character(open_valves_command[i]);
    }
    // this should cause the valves to be open
    UNIT_TEST( (radio_get_expected_inj_valve_state() == VALVE_OPEN &&
                radio_get_expected_vent_valve_state() == VALVE_OPEN),
                "write half the close command, then the open command");

    // change one byte in the close command. This should screw up the CRC
    // check, and that state should not be applied
    close_valves_command[1]++; //change the second byte
    for (i = 0; i < STATE_COMMAND_LEN - 1; ++i) {
        radio_handle_input_character(close_valves_command[i]);
    }
    UNIT_TEST( (radio_get_expected_inj_valve_state() == VALVE_OPEN &&
                radio_get_expected_vent_valve_state() == VALVE_OPEN),
                "changing a byte in the close command doesn't cause CRC to fail");

    printf("%s Test Results: %i tests, %i passed, %i %sfailed%s\n",
            __FILE__,
            total_tests,
            total_tests - failing_tests,
            failing_tests, failing_tests ? COLOR_RED : COLOR_GREEN, COLOR_NONE);
}

