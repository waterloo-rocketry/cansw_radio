#include "radio_handler.h"

#include "pic18_time.h" // remove this

enum VALVE_STATE radio_get_expected_inj_valve_state(void)
{
    /* This function currently just changes the inj valve state from open to
     * closed with a rate of 1Hz. This is for debugging, don't actually launch
     * a rocket with this going
     */
    static uint32_t counter = 0;
    if (millis() - counter > 2000) {
        counter = millis();
    }

    if (millis() - counter < 1000) {
        return VALVE_OPEN;
    } else {
        return VALVE_CLOSED;
    }
}

enum VALVE_STATE radio_get_expected_vent_valve_state(void)
{
    /* This function currently just changes the vent valve state from open to
     * closed with a rate of 3Hz. This is for debugging, don't actually launch
     * a rocket with this going
     */
    static uint32_t counter = 0;
    if (millis() - counter > 666) {
        counter = millis();
    }

    if (millis() - counter < 333) {
        return VALVE_OPEN;
    } else {
        return VALVE_CLOSED;
    }
}

void radio_handle_input_character(uint8_t c)
{
    //TODO
}
