#ifndef RADIO_HANDLER_H_
#define RADIO_HANDLER_H_

#include "message_types.h"
#include <stdint.h>

/*
 * If we haven't gotten any radio commands in the last 10 seconds, go to safe
 * state. The "safe state" here just means "open the vent valve", but we may
 * someday have other safe state stuff
 */
#define TIME_NO_CONTACT_BEFORE_SAFE_STATE 10000

/*
 * Returns the state that the operators on the ground are telling us that the
 * injector valve should be in. If we haven't heard from the operators ever (we
 * just booted up and haven't established radio contact), this function will
 * return VALVE_UNK, because we shouldn't assume that we should start with the
 * injector valve in either state.
 *
 * If we haven't heard from the operator in TIME_NO_CONTACT_BEFORE_SAFE_STATE,
 * this function will continue to spit out whatever value we last got from the
 * operator, because going to safe state does not imply doing anything to the
 * injector valve.
 */
enum VALVE_STATE radio_get_expected_inj_valve_state(void);

/*
 * Returns the state that the operators on the ground are telling us that the
 * vent valve should be in. If we haven't heard from the operators ever (we
 * just booted up and haven't established radio contact), this function will
 * return VALVE_OPEN, because in the presence of no information, we should
 * always open the vent valve.
 *
 * If we haven't heard from the operator in TIME_NO_CONTACT_BEFORE_SAFE_STATE,
 * this function will return VALVE_OPEN, because that's the safest state for the
 * vent valve to be in
 */
enum VALVE_STATE radio_get_expected_vent_valve_state(void);

void radio_handle_input_character(uint8_t c);

#endif