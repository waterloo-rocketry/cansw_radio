#ifndef SHARK_OF_THE_SKY_CONTROL_H_
#define SHARK_OF_THE_SKY_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>
#include "canlib/can.h"
#include "canlib/message_types.h" //defines VALVE_STATE enum

/*
 * If we do not receive a MSG_VENT_VALVE_STATUS from the vent board every
 * this many milliseconds (or MSG_INJ_VALVE_STATUS from injector), we will
 * consider the state of that valve to be unknown
 */
#define MIN_TIME_BETWEEN_VALVE_UPDATE_MS 3000
/*
 * If we do not receive any message from a given board every this many
 * milliseconds, we will consider that board to be dead. We will issue an
 * error, and we will decrement our count of boards
 */
#define MIN_TIME_BETWEEN_BOARD_HEARTBEAT_MS 3000

/*
 * Call this function at bootup, and never again. It will reset all of the
 * internal data. The reason this function needs to exist is that I can't
 * statically initialize our internal data with microchip's compiler
 */
void init_sotscon(void);

/*
 * Call this function every time we receive a CAN message over the bus.
 * This will update our current knowledge of the rocket state. This function
 * is not designed to be thread safe or particularly efficient, so it
 * shouldn't be called from an ISR.
 *
 * TODO, add a buffering layer for CAN messages so that we don't have to call
 * this from an ISR
 */
void handle_incoming_can_message(const can_msg_t *msg);

/*
 * If we have received a VENT_VALVE_STATUS message in the last
 * MIN_TIME_BETWEEN_VALVE_UPDATE_MS, this function will return either
 * VALVE_OPEN or VALVE_CLOSED, depending on, you know, whether it's open
 * or closed. If we haven't received that status message in the correct
 * amount of time, this function will return VALVE_UNK.
 */
enum VALVE_STATE current_vent_valve_position(void);

/*
 * Same idea as current_vent_valve_position. Returns open, closed, or unk
 * if we haven't received a status message in MIN_TIME_BETWEEN_VALVE_UPDATE_MS
 */
enum VALVE_STATE current_inj_valve_position(void);

/*
 * Returns the number of unique BOARD_UNIQUE_IDs that this code module has
 * seen in the incoming CAN messages. Note that this does not always mean it
 * will know how many boards are present, since if a board has not issued
 * any CAN messages, this module will cannot know that it exists.
 *
 * If we have not received any CAN messages from a board in the last
 * MIN_TIME_BETWEEN_BORAD_HEARTBEAT_MS milliseconds, that board will not be
 * counted in the value that this function returns.
 */
uint8_t current_num_boards_connected(void);

/*
 * Returns true if any boards that are connected to the CAN bus are believed
 * to have any active errors. An error is defined as active if the board that
 * sent it has not sent MAX_CONSECUTIVE_NOMINALS E_NOMINAL messages (the
 * "everything is ok" message) since sending it.
 *
 * So if the vent board sends a "battery voltage low" error message, this
 * function will return true until the vent board has sent MAX_CONSECUTIVE_NOMINALS
 * "Everything is find" messages.
 */
bool any_errors_active(void);

#endif
