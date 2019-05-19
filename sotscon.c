#include "sotscon.h"
#include "can_common.h"
#include "message_types.h"
#include "pic18_time.h"
#include "error.h"
#include "radio_handler.h"

/* File local macros */

/*
 * We key boards by their unique id. In theory there could be up to 32 of
 * these, but at present there are only 12 defined IDs, so we can have just
 * those.
 */
#define MAX_BOARD_UNIQUE_ID 0x0C

/*
 * We keep track of how many consecutive E_NOMINAL general status messages
 * we've received from each board. If we receive this many E_NOMINAL's in a
 * row, we consider that board to have no errors active (for the purposes of
 * any_errors_active())
 */
#define MAX_CONSECUTIVE_NOMINALS 20

/* Internal data */

/*
 * As we keep more and more data about every board (like whether there are
 * any errors present, or what board type it is, etc), put that in this
 * struct. Whenever a message is received, the proper one of these structs
 * (board[unique_id]) is updated with whatever it contains
 */
static struct {
    bool valid; //if we have ever received a message from the board, this is true
    uint32_t time_last_message_received_ms;
    uint8_t consecutive_nominals; //how many nominal statuses we've received in a row
} boards[MAX_BOARD_UNIQUE_ID + 1]; //+1 because array indexing starts at 0

/*
 * Keep track of the unique ids of the vent and injector boards. If we haven't
 * heard from one of the two boards yet, it's unique ID will be recorded as 0
 */
static uint8_t vent_board_unique_id = 0;
static uint8_t  inj_board_unique_id = 0;

/*
 * Keep track of the valve state of the vent and inj valves. When we don't know
 * the valve state, or if we've gone through a timeout, these will be set to
 * VALVE_UNK
 */
static enum VALVE_STATE vent_valve_state;
static enum VALVE_STATE  inj_valve_state;

/*
 * Keep track of how many boards we've heard from
 */
static uint8_t connected_boards = 0;

/*
 * If we see any error ever, set this to true. If, after that, we receive
 * MAX_CONSECUTIVE_NOMINALS E_NOMINAL messages from that board, mark this false
 */
static bool errors_active = false;

/*
 * Every time we get a pressure reading from sensor, put it here
 */
static uint16_t last_tank_pressure = 0;

/* Private function declarations */
static void update_all_timeouts(void);
static void update_errors_active(void);

/* Public function definitions */
void init_sotscon(void)
{
    /* Mark all boards as invalid */
    uint8_t i;
    for (i = 0; i <= MAX_BOARD_UNIQUE_ID; ++i) {
        boards[i].valid = false;
    }

    /* Set connected boards, unique id's, and valve states */
    connected_boards = 0;
    vent_board_unique_id = 0;
    inj_board_unique_id = 0;
    vent_valve_state = VALVE_UNK;
    inj_valve_state = VALVE_UNK;
}

void handle_incoming_can_message(const can_msg_t *msg)
{
    uint8_t i; //used for MSG_DEBUG_RADIO_CMD, can't declare in case statement
    uint8_t sender_unique_id = get_board_unique_id(msg);
    if (sender_unique_id > MAX_BOARD_UNIQUE_ID) {
        report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
    } else {
        switch (get_message_type(msg)) {
            case MSG_GENERAL_BOARD_STATUS:
                boards[sender_unique_id].valid = true;
                boards[sender_unique_id].time_last_message_received_ms = millis();
                uint8_t error_code = msg->data[3];
                if (error_code == E_NOMINAL) {
                    if (boards[sender_unique_id].consecutive_nominals < MAX_CONSECUTIVE_NOMINALS) {
                        boards[sender_unique_id].consecutive_nominals++;
                    } else {
                        update_errors_active();
                    }
                } else {
                    boards[sender_unique_id].consecutive_nominals = 0;
                    errors_active = true;
                    report_error(sender_unique_id,
                                 error_code,
                                 msg->data[4],
                                 msg->data[5],
                                 msg->data[6],
                                 msg->data[7]);
                }
                break;

            /* Update our idea of the last vent valve state */
            case MSG_VENT_VALVE_STATUS:
                //we've now heard from it, so it exists
                boards[sender_unique_id].valid = true;
                //update the last time we heard from it
                boards[sender_unique_id].time_last_message_received_ms = millis();

                //validate byte 3 (valve state), and if it's ok remember it
                if (sender_unique_id != vent_board_unique_id &&
                    vent_board_unique_id != 0) {
                    //this is very very bad. You cannot have 2 vent boards
                    report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
                } else if (msg->data[3] != VALVE_OPEN ||
                           msg->data[3] != VALVE_CLOSED ||
                           msg->data[3] != VALVE_UNK) {
                    //this is also bad, this is not a valid valve_state
                    report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
                } else {
                    //yay, we know the state now
                    vent_valve_state = msg->data[3];
                }
                break;

            /* Update our idea of the last inj valve state */
            case MSG_INJ_VALVE_STATUS:
                boards[sender_unique_id].valid = true;
                boards[sender_unique_id].time_last_message_received_ms = millis();

                //validate byte 3 (valve state), and if it's ok remember it
                if (sender_unique_id != inj_board_unique_id &&
                    inj_board_unique_id != 0) {
                    //this is very very bad. You cannot have 2 vent boards
                    report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
                } else if (msg->data[3] != VALVE_OPEN ||
                           msg->data[3] != VALVE_CLOSED ||
                           msg->data[3] != VALVE_UNK) {
                    //this is also bad, this is not a valid valve_state
                    report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
                } else {
                    //yay, we know the state now
                    inj_valve_state = msg->data[3];
                }
                break;

            /* Handle this message by updating last_tank_pressure, then updating last
             * time we've heard from that board
             */
            case MSG_SENSOR_ANALOG:
                if (msg->data[2] == SENSOR_PRESSURE) {
                    // we have a pressure, update the pressure
                    last_tank_pressure = ((uint16_t) msg->data[3] << 8) | msg->data[4];
                }
                boards[sender_unique_id].valid = true;
                boards[sender_unique_id].time_last_message_received_ms = millis();
                break;
            /* Handle these messages by updating the last time since we've
               heard from the sender, but otherwise take no action */
            case MSG_VENT_VALVE_CMD:
            case MSG_INJ_VALVE_CMD:
            case MSG_SENSOR_ACC:
            case MSG_SENSOR_GYRO:
            case MSG_SENSOR_MAG:
            case MSG_GENERAL_CMD:
                boards[sender_unique_id].valid = true;
                boards[sender_unique_id].time_last_message_received_ms = millis();
                break;
            /* We do not handle these message types in any way, not even to
               update time_last_message_received_ms */
            case MSG_DEBUG_PRINTF:
            case MSG_DEBUG_MSG:
            case MSG_LEDS_ON:
            case MSG_LEDS_OFF:
                break;

            /* We treat all the bytes in the debug_radio_cmd message as though
               we just received them from the radio. ie we hand them to the
               radio handler */
            case MSG_DEBUG_RADIO_CMD:
                for (i = 0; i < msg->data_len; ++i) {
                    radio_handle_input_character(msg->data[i]);
                }
                break;

            /* We have provided cases for every message type in the spreadsheet,
               so if we get to the default case, that either means that one of
               the boards is sending a SID that it shouldn't be, or that we added
               a message type and haven't updated this code. Either way, report
               it as an error and move on */
            default:
                report_error(BOARD_UNIQUE_ID, E_ILLEGAL_CAN_MSG, 0, 0, 0, 0);
                break;
        }
    }
}

enum VALVE_STATE current_vent_valve_position(void)
{
    update_all_timeouts();
    return vent_valve_state;
}

enum VALVE_STATE current_inj_valve_position(void)
{
    update_all_timeouts();
    return inj_valve_state;
}

uint8_t current_num_boards_connected(void)
{
    update_all_timeouts();
    return connected_boards;
}

bool any_errors_active(void)
{
    return errors_active;
}

uint16_t current_tank_pressure(void)
{
    if (last_tank_pressure > 999) {
        return 999;
    } else {
        return last_tank_pressure;
    }
}

/* Private function definitions */
static void update_all_timeouts(void)
{
    uint32_t current_time_ms = millis();
    /*
     * Go through all the boards. If valid bit is set, and we have heard
     * from the board in the last MIN_TIME_BETWEEN_BOARD_HEARTBEAT_MS
     * milliseconds, count it in the connected boards.
     */
    connected_boards = 0;
    uint8_t i;
    for (i = 1; i <= MAX_BOARD_UNIQUE_ID; ++i) {
        if (boards[i].valid) {
            if ((current_time_ms - boards[i].time_last_message_received_ms) <
                MIN_TIME_BETWEEN_BOARD_HEARTBEAT_MS) {
                connected_boards++;
            } else {
                //mark the board as invalid and report it as maybe dead
                boards[i].valid = false;
                report_error(BOARD_UNIQUE_ID, E_BOARD_FEARED_DEAD, i, 0, 0, 0);
            }
        }
    }

    /*
     * Check if we heard from the injector board recently enough. Set
     * `inj_valve_state` and `vent_valve_state` based on that.
     */
    if (vent_board_unique_id == 0 || (!boards[vent_board_unique_id].valid) ||
        (boards[vent_board_unique_id].time_last_message_received_ms - current_time_ms)
        >= MIN_TIME_BETWEEN_VALVE_UPDATE_MS) {
        //mark the vent valve state as invalid
        vent_valve_state = VALVE_UNK;
    }
    if (inj_board_unique_id == 0 || (!boards[inj_board_unique_id].valid) ||
        (boards[inj_board_unique_id].time_last_message_received_ms - current_time_ms) >=
        MIN_TIME_BETWEEN_VALVE_UPDATE_MS) {
        //mark the inj valve state as invalid
        inj_valve_state = VALVE_UNK;
    }
}

/*
 * Loop through all the boards. If any of them have received fewer than
 * MAX_CONSECUTIVE_NOMINALS E_NOMINAL messages, set errors_active to true,
 * because that board still has an error active. If all boards have received
 * that many nominal messages, set errors_active to false
 */
static void update_errors_active(void)
{
    /*
     * errors_active is set true whenever we receive an error message. Therefore,
     * if errors_active is false, we've never received an error message or all
     * boards have received enough nominal messages to clear the errors. Either
     * way, there's no point in checking all the boards
     */
    if (errors_active == false) {
        return;
    }
    uint8_t i;
    for (i = 1; i < MAX_BOARD_UNIQUE_ID; ++i) {
        if (boards[i].valid &&
            (boards[i].consecutive_nominals < MAX_CONSECUTIVE_NOMINALS)) {
            errors_active = true;
            return;
        }
    }
    errors_active = false;
}
