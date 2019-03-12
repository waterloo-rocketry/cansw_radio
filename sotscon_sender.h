#ifndef SHARK_OF_THE_SKY_CONTROL_SENDER_H_
#define SHARK_OF_THE_SKY_CONTROL_SENDER_H_

/*
 * Only send 10 valve commands per second. Note that this is not splitting
 * bandwidth between vent and injector, this module will send two valve commands
 * (one injector and one vent) every 100ms, if both are in the wrong state
 */
#define MIN_TIME_BETWEEN_VALVE_CMD_MS 100

/*
 * This function checks whether the vent valve and injector valve are in the
 * expected open/close state (based on radio_get_expected_*_valve_state and
 * current_*_vlave_position), and if they are not, sends a MSG_*_VALVE_CMD over
 * the CANbus. This function handles the rate limiting of only sending a
 * MSG_VENT_VALVE_CMD every MIN_TIME_BETWEEN_VALVE_CMD_MS milliseconds
 */
void sotscon_heartbeat(void);

#endif
