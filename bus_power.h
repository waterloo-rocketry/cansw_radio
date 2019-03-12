#ifndef BUS_POWER_H_
#define BUS_POWER_H_

#include <stdbool.h>

/*
 * The whole goal of this module is to decide whether we should be providing
 * power to the bus, and do so if we're supposed to be.
 */

// After we decide to shut down the bus, wait this many ms in order to give
// the other boards on the bus time to finish spinning down
#define BUS_SHUTDOWN_WARNING_MS 10000

// How much time we should give the other nodes to initialize themselves before
// we start flooding them with CAN messages
#define BUS_POWERUP_TIME_MS 3000

/*
 * Call this function at the beginning of runtime. It initializes local variables
 */
void init_bus_power(void);

/*
 * Call this function every loop through the program code. This function will
 * be the one that removes power from the CAN bus after a shutdown wait
 */
void bus_power_heartbeat(void);

/*
 * Returns true if the bus is currently powered. If this function returns true,
 * then the other boards on the bus are running. During a shutdown time (in the
 * BUS_SHUTDOWN_WARNING_MS milliseconds between triggering a shutdown and when
 * the power is actually removed), this function will return false
 */
bool is_bus_powered(void);

/*
 * Returns true if we're currently in a shutdown time (the
 * BUS_SHUTDOWN_WARNING_MS milliseconds between triggering a shutdown and when
 * the power is actually removed), this function will return true
 */
bool is_bus_shutting_down(void);

/*
 * Triggers a bus shutdown. This will cause a MSG_GENERAL_CMD to be sent over
 * the CAN bus, in order to warn the other boards of a shutdown. After
 * BUS_SHUTDOWN_WARNING_MS milliseconds, the power will be removed from the bus
 */
void trigger_bus_shutdown(void);

/*
 * Triggers a bus powerup. Will apply power to the bus, in order to wake up the
 * other boards
 */
void trigger_bus_powerup(void);

#endif
