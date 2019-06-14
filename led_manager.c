#include "led_manager.h"
#include "platform.h"
#include "bus_power.h"
#include "sotscon.h"
#include "pic18_time.h"
#include "radio_handler.h"
#include <stdint.h>
#include <stdbool.h>

static uint32_t time_last_blink_started = 0;
static bool vent_valve_desired_open, inj_valve_desired_open, bus_powered;

void init_led_manager(void)
{
    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();

    time_last_blink_started = millis();
    if (radio_get_expected_vent_valve_state() == VALVE_OPEN) {
        vent_valve_desired_open = true;
    }
    if (radio_get_expected_inj_valve_state() == VALVE_OPEN) {
        inj_valve_desired_open = true;
    }
    bus_powered = is_bus_powered();
}

void led_manager_heartbeat(void)
{
    // everything has a frequency of 0.5 Hz
    if (millis() - time_last_blink_started > 2000) {
        time_last_blink_started = millis();

        // only decide these once per cycle to save function calls
        vent_valve_desired_open = (radio_get_expected_vent_valve_state() == VALVE_OPEN);
        inj_valve_desired_open = (radio_get_expected_inj_valve_state() == VALVE_OPEN);
        bus_powered = is_bus_powered();
    }

    // from time 0 to 100 ms:
    // LED_1 is on
    // LED_2 is on if we want the vent valve open
    // LED_3 is on if we want the injector valve open
    if (millis() - time_last_blink_started < 100) {
        LED_1_ON();
        if (vent_valve_desired_open) {
            LED_2_ON();
        }
        if (inj_valve_desired_open) {
            LED_3_ON();
        }
    }

    // from time 100 ms to 300ms, everything is off
    else if (millis() - time_last_blink_started < 300) {
        LED_1_OFF();
        LED_2_OFF();
        LED_3_OFF();
    }

    // from time 300 to 400ms:
    // LED_1 is on if the bus is powered
    else if (millis() - time_last_blink_started < 400) {
        if (bus_powered) {
            LED_1_ON();
        }
    }

    // from time 400ms to 2000ms, everything is off
    else {
        LED_1_OFF();
        LED_2_OFF();
        LED_3_OFF();
    }
}
