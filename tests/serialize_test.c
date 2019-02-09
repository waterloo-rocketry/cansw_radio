#include "serialize.h"
#include <stdio.h>

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
    // test that comparing two identical states returns true
    system_state s = {
        .num_boards_connected = 2,
        .injector_valve_open = true,
        .vent_valve_open = false,
        .running_self_test = false,
        .any_errors_detected = true,
    };
    system_state p = s;
    UNIT_TEST( compare_system_states(&s, &p), "Compare identical system_state's");

    //test that comparing two different states returns false
    p.num_boards_connected++;
    p.any_errors_detected = false;
    UNIT_TEST( !compare_system_states(&s, &p), "Compare different system_state's");
    
    //test that we can serialize s, deserialize that into p, and then they should be equal
    char serialized_output[SERIALIZED_OUTPUT_LEN];
    UNIT_TEST(serialize_state(&s, serialized_output), "Serializing valid state returns true");
    UNIT_TEST(deserialize_state(&p, serialized_output), "Deserializing vaid output returns true");
    UNIT_TEST(compare_system_states(&s, &p), "Serializing and deserializing results in identical states");

    //test that passing deserialize an empty string causes it to return false
    UNIT_TEST(deserialize_state(&p, ""), "Deserializing empty string returns false");

    //test that passing a null pointer to serialize_state causes it to return false
    UNIT_TEST(!serialize_state(&s, NULL), "Passing serialize_state a null output pointer");
    UNIT_TEST(!serialize_state(NULL, serialized_output), "Passing serialize_state a null input pointer");

    //test that passing a null pointer to deserialize_state causes it to return false
    UNIT_TEST(!deserialize_state(&p, NULL), "Passing deserialize_state a null input pointer");
    UNIT_TEST(!deserialize_state(NULL, serialized_output), "Passing deserialize_state a null output pointer");

    printf("Test Results: %i tests, %i passed, %i %sfailed%s\n", total_tests, total_tests - failing_tests, failing_tests, failing_tests ? COLOR_RED : COLOR_GREEN, COLOR_NONE);
}
