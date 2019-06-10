#include "analog.h"
#include "platform.h"
#include "pic18_time.h"
#include "message_types.h"
#include "error.h"
#include <xc.h>

//global variables. Until we have a better way of handling analog
//values that we read (putting them out over CAN or something), just
//store them here
static uint16_t analog_last_batt_voltage = 0;
static uint16_t analog_last_batt_current = 0;
static uint16_t analog_last_bus_current = 0;

//private function declarations
static void check_analog_values(void);

uint16_t analog_get_vin_mv(void)
{
    /*
     * Multiplication factor is empirically determined
     */
    return analog_last_batt_voltage * 4;
}

uint16_t analog_get_ibatt_ma(void)
{
    /*
     * the ADC is reading a signal that has been amplified 100x. The sense
     * resistors are 150mOhm. Therefore, 1mA = 150uV * 100 = 15mV.
     *
     * one ADC read is 1mV (12 bit gives 4096 possible numbers, and we're
     * comparing to a voltage of 4.096V. Therefore, to go from
     * analog_last_batt_current to battery current in mA, we have to divide by
     * 15.
     *
     * Anecdotally, I ran this in debug mode and found that we were getting
     * anywhere between 14 and 24 mA, and then I ran it in normal mode and
     * measured the current draw, and got abotu 22mA (jumping up to 27 mA when
     * an LED turned on, as you'd expect). So I'm calling this division thing
     * pretty accurate.
     */
    return analog_last_batt_current / 15;
}

uint16_t analog_get_ibus_ma(void)
{
    /*
     * Same logic as above. Divide by 15.
     */
    return analog_last_bus_current / 15;
}

//the states for the adc state machine thing
static enum {
    NONE,
    READING_BATT_VOLTAGE,
    READING_BATT_CURRENT,
    READING_BUS_CURRENT
} state;


void analog_report_value(uint16_t value, uint8_t channel)
{
    switch (state) {
        case READING_BATT_VOLTAGE:
            //confirm that channel is the battery voltage channel
            if (channel != ANALOG_CH_BATT_VOLTAGE) {
                //this shouldn't be possible, so infinite loop to mark
                //that someday we'll have better error reporting than
                //infinite loops, but i don't know when
                while (1) {}
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
            if (channel != ANALOG_CH_BATT_CURRENT) {
                while (1) {}
            }

            analog_last_batt_current = value;

            state = READING_BUS_CURRENT;

            ADPCH = ANALOG_CH_BUS_CURRENT;
            ADCON0bits.GO = 1;

            break;
        case READING_BUS_CURRENT:
            if (channel != ANALOG_CH_BUS_CURRENT) {
                while (1) {}
            }

            analog_last_bus_current = value;

            state = NONE;

            break;
        case NONE:
            //this should never happen. This function should never get
            //called unless we're in a state that isn't NONE.
            while (1) {}
            break;
        default:
            //also an unreachable state
            while (1) {}
    }
}

bool analog_read_all_values()
{
    if (state == NONE) {
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

void analog_heartbeat(void)
{
    static uint32_t last_read_ms = 0;
    if (millis() - last_read_ms > 100) {
        last_read_ms = millis();
        analog_read_all_values();
        if (last_read_ms > 0) {
            //we've already done one conversion, so check that the values for
            //all the analog signals are within range
            check_analog_values();
        }
    }
}

/*
 * Check the input current, output current, and input voltage. If any of them
 * are out of the expected range, report an error.
 *
 * Current expected values:
 * Input voltage: 10V < Vin < 14V
 * Input current: Iin < 2A
 * Output current: Iout < 2A
 *
 * Don't report an error on each of these values more than once every second,
 * just so we don't overwhelm the bus or the radio
 */
static void check_analog_values(void)
{
    uint16_t vin = analog_get_vin_mv();
    uint16_t iin = analog_get_ibatt_ma();
    uint16_t iout = analog_get_ibus_ma();

    static uint32_t ms_last_vin_error = 0;
    static uint32_t ms_last_iin_error = 0;
    static uint32_t ms_last_iout_error = 0;

    if (vin < 10000) {
        if (millis() - ms_last_vin_error > 5000) {
            ms_last_vin_error = millis();
            report_error(BOARD_UNIQUE_ID,
                         E_BATT_UNDER_VOLTAGE,
                         vin >> 8,
                         vin & 0xff,
                         0, 0);
        }
    } else if (vin > 14000) {
        if (millis() - ms_last_vin_error > 5000) {
            ms_last_vin_error = millis();
            report_error(BOARD_UNIQUE_ID,
                         E_BATT_OVER_VOLTAGE,
                         vin >> 8,
                         vin & 0xff,
                         0, 0);
        }
    }

    if (iin > 2000) {
        if (millis() - ms_last_iin_error > 5000) {
            ms_last_iin_error = millis();
            report_error(BOARD_UNIQUE_ID,
                         E_BATT_OVER_CURRENT,
                         iin >> 8,
                         iin & 0xff,
                         0, 0);
        }
    }

    if (iout > 2000) {
        if (millis() - ms_last_iout_error > 5000) {
            ms_last_iout_error = millis();
            report_error(BOARD_UNIQUE_ID,
                         E_BUS_OVER_CURRENT,
                         iout >> 8,
                         iout & 0xff,
                         0, 0);
        }
    }
}
