#include "bus_power.h"
#include "can.h"
#include "pic18f26k83_can.h"
#include "message_types.h"
#include "pic18_time.h"
#include "can_common.h"
#include <xc.h>

/*
 * State transition diagram for this module:
 *
 *                               call trigger_bus_powerup
 *                             UNPOWERED ------> STARTING_UP
 *                               /|\                 |
 * after BUS_SHUTDOWN_WARNING_MS  |                  | after BUS_POWERUP_TIME_MS
 *                                |                 \|/
 *                             SHUTDOWN <------- POWERED
 *                               call trigger_bus_shutdown
 *
 * BUS_POWERUP_TIME_MS is to give the other boards time to initialize
 * everything. The goal is that we can receive messages while in any
 * state except UNPOWERED, but we won't send messages (except for the
 * shutdown warnings in SHUTDOWN) in any state except POWERED.
 *
 * The only transition not shown in this diagram is from STARTING_UP
 * to SHUTDOWN: if trigger_bus_shutdown is called while we're waiting
 * for BUS_POWERUP_TIME_MS, we immediately switch to SHUTDOWN state,
 * send the warning message, and wait the BUS_SHUTDOWN_WARNING_MS.
 *
 * While in SHUTDOWN state, we ignore calls to trigger_bus_powerup.
 */

static enum {
    BUS_UNPOWERED,
    BUS_STARTING_UP,
    BUS_POWERED,
    BUS_SHUTDOWN,
} state;
static uint32_t time_last_state_transition;

static void power_bus(void)
{
    LATA2 = 1;
}

static void depower_bus(void)
{
    LATA2 = 0;
}

void init_bus_power(void)
{
    //pins are initialized in init.c, we don't need to do that here
    state = BUS_UNPOWERED;
    time_last_state_transition = 0;
    depower_bus();
}

void bus_power_heartbeat(void)
{
    //handle state transitions. Don't send CAN messages or drive any pins,
    //all that is handled in the trigger_*() functions
    switch (state) {
        case BUS_UNPOWERED:
        case BUS_POWERED:
            //do nothing
            break;
        case BUS_STARTING_UP:
            if (millis() - time_last_state_transition >= BUS_POWERUP_TIME_MS) {
                state = BUS_POWERED;
                time_last_state_transition = millis();
            }
            break;
        case BUS_SHUTDOWN:
            if (millis() - time_last_state_transition >= BUS_SHUTDOWN_WARNING_MS) {
                state = BUS_UNPOWERED;
                time_last_state_transition = millis();
                depower_bus();
            }
            break;
        default:
            //we forgot to handle a case. This is bad. TODO, handle it
            break;
    }
}

bool is_bus_powered(void)
{
    return (state == BUS_POWERED);
}

bool is_bus_shutting_down(void)
{
    return (state == BUS_SHUTDOWN);
}

void trigger_bus_shutdown(void)
{
    switch (state) {
        case BUS_UNPOWERED:
        case BUS_SHUTDOWN: //repeated call, do nothing
            break;
        case BUS_POWERED:
        case BUS_STARTING_UP:
            // in both cases, send the warning message and begin shutdown
            // do not depower the bus, that happens in the heartbeat
            state = BUS_SHUTDOWN;
            time_last_state_transition = millis();

            can_msg_t power_down_warning;
            build_general_cmd_msg(micros(),
                                  BUS_DOWN_WARNING,
                                  &power_down_warning);
            can_send(&power_down_warning, 0);
            break;
        default:
            //unhandled case. TODO, handle it
            break;
    }
}

void trigger_bus_powerup(void)
{
    switch (state) {
        case BUS_POWERED:
        case BUS_STARTING_UP: //repeated command, do nothing
            break;
        case BUS_SHUTDOWN: //ignore this command
            break;
        case BUS_UNPOWERED:
            power_bus();
            state = BUS_STARTING_UP;
            time_last_state_transition = millis();
            break;
        default:
            //unhandled. TODO, make it not that
            break;
    }
}
