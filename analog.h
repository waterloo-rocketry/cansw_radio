#ifndef ANALOG_H_
#define ANALOG_H_

/*
 * TODO, comment explaining how this works and how to use it
 */

#include <stdbool.h>
#include <stdint.h>

/*
 * returns the voltage coming in from the battery, in mV
 */
uint16_t analog_get_vin_mv(void);

/*
 * returns the current coming in from the battery, in mA
 */
uint16_t analog_get_ibatt_ma(void);

/*
 * returns the current going out to the bus, in mA
 */
uint16_t analog_get_ibus_ma(void);

/*
 * Takes in a value from a channel. Deals with it. Note that this
 * function runs in an isr context, so it's possible to introduce race
 * conditions here if you're not careful. So.... be careful fucking
 * with this.
 */
void analog_report_value(uint16_t value, uint8_t channel);

/*
 * starts the process of reading all of the analog values that we care
 * about returns true if successful, false otherwise. The only way
 * this will return false is if we're currently in the process of
 * reading all the values and a request comes in
 */
bool analog_read_all_values(void);

/*
 * triggers analog_read_all_values every 100 ms
 */
void analog_heartbeat(void);

#endif //ANALOG_H_
