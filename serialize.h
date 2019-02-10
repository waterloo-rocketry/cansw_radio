#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * This macro defines how long (in bytes) a string must be in order to
 * hold a serialized system_state. It includes the null terminator, so
 * a serialized system state should have SERIALIZED_OUTPUT_LEN - 1 ascii
 * characters in it
 */
#define SERIALIZED_OUTPUT_LEN 3

/*
 * This type contains all of the information that needs to be shared between
 * the operator on the ground and the CAN system in the rocket.
 */
typedef struct {
    uint8_t num_boards_connected;
    bool injector_valve_open;
    bool vent_valve_open;
    bool running_self_test;
    bool any_errors_detected;
} system_state;

/*
 * This function takes a system_state and serializes it into ASCII text that
 * can be sent over the radio. It will return true if it was able to
 * successfully serialize the state, false otherwise. Examples of how it can
 * return false is passing it a NULL pointer as either argument
 */
bool serialize_state(const system_state *state, char *str);

/*
 * This function takes a string which was generated by the serialize_state
 * function and converts it back into a system_state. Returns true if it
 * was successfully able to deserialize a state, false otherwise
 */
bool deserialize_state(system_state *state, const char *str);

/*
 * Returns true if the two system states passed to it are equal (returns
 * false if either of them are NULL). Note that in C you're not just allowed
 * to do (*s == *p), you have to individually compare each element in the
 * struct, due to data representation reasons
 */
bool compare_system_states(const system_state *s, const system_state *p);

#endif
