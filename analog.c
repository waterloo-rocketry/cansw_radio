#include "analog.h"
#include "platform.h"
#include <xc.h>

//global variables. Until we have a better way of handling analog
//values that we read (putting them out over CAN or something), just
//store them here
uint16_t analog_last_batt_voltage = 0;
uint16_t analog_last_batt_current = 0;
uint16_t analog_last_bus_current = 0;

//the states for the adc state machine thing
static enum {
    NONE,
    READING_BATT_VOLTAGE,
    READING_BATT_CURRENT,
    READING_BUS_CURRENT
} state;

/*
 * Takes in a value from a channel. Deals with it. Note that this
 * function runs in an isr context, so it's possible to introduce race
 * conditions here if you're not careful. So.... be careful fucking
 * with this.
 */
void analog_report_value(uint16_t value, uint8_t channel) {
    switch(state) {
    case READING_BATT_VOLTAGE:
        //confirm that channel is the battery voltage channel
        if(channel != ANALOG_CH_BATT_VOLTAGE) {
            //this shouldn't be possible, so infinite loop to mark
            //that someday we'll have better error reporting than
            //infinite loops, but i don't know when
            while(1) {}
        }

        //here is where we would report the value over CAN or
        //something, but we don't have any way to do that. So... TODO
        analog_last_batt_voltage = value;

        //transition to the next state. After reading the battery
        //voltage, we switch to reading the battery current, and then
        //to reading the bus current.
        state = READING_BATT_CURRENT;

        //set up the next reading. Set the analog channel and hit go
        ADPCH = ANALOG_CH_BATT_CURRENT;
        ADCON0bits.GO = 1;

        break;
    case READING_BATT_CURRENT:
        //confirm that channel is the battery current channel
        if(channel != ANALOG_CH_BATT_CURRENT) {
            while(1) {}
        }

        analog_last_batt_current = value;

        state = READING_BUS_CURRENT;

        ADPCH = ANALOG_CH_BUS_CURRENT;
        ADCON0bits.GO = 1;

        break;
    case READING_BUS_CURRENT:
        if(channel != ANALOG_CH_BUS_CURRENT) {
            while(1) {}
        }

        analog_last_bus_current = value;

        state = NONE;

        break;
    case NONE:
        //this should never happen. This function should never get
        //called unless we're in a state that isn't NONE.
        while(1) {}
        break;
    default:
        //also an unreachable state
        while(1) {}
    }
}

/*
 * starts the process of reading all of the analog values that we care
 * about returns true if successful, false otherwise. The only way
 * this will return false is if we're currently in the process of
 * reading all the values and a request comes in
 */
bool analog_read_all_values() {
    if(state == NONE) {
        //this is the only time that this function is allowed to write
        //to the state variable
        state = READING_BATT_VOLTAGE;
        //start the analog conversion process.
        // set channel to ANA3, the battery voltage channel
        ADPCH = ANALOG_CH_BATT_VOLTAGE;
        // click go.
        ADCON0bits.GO = 1;
        return true;
    } else {
        return false;
    }
}
