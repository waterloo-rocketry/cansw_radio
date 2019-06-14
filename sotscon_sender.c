#include "sotscon_sender.h"
#include "message_types.h"
#include "can_common.h"
#include "can_tx_buffer.h"
#include "pic18f26k83_can.h"
#include "radio_handler.h"
#include "sotscon.h"
#include "pic18_time.h"
#include "error.h"

/* Private function declarations */

// the rl stands for rate limiting
static void rl_send_inj_valve_cmd(enum VALVE_STATE s);

/* Public function definitions */

void sotscon_heartbeat(void)
{
    enum VALVE_STATE current_inj = current_inj_valve_position();

    enum VALVE_STATE desired_inj = radio_get_expected_inj_valve_state();

    if (current_inj != desired_inj) {
        rl_send_inj_valve_cmd(desired_inj);
    }
}


/* Private function definitions */


static uint32_t time_last_inj_valve_cmd = 0;
static void rl_send_inj_valve_cmd(enum VALVE_STATE s)
{
    if ((millis() - time_last_inj_valve_cmd) < MIN_TIME_BETWEEN_VALVE_CMD_MS) {
        return;
    }
    can_msg_t valve_cmd;
    uint8_t can_msg_data = s; //only one byte of data needed for valve commands
    if (!build_valve_cmd_msg(micros(),
                             s,
                             MSG_INJ_VALVE_CMD,
                             &valve_cmd)) {
        report_error(BOARD_UNIQUE_ID, E_SEGFAULT, 0, 0, 0, 0);
    } else {
        txb_enqueue(&valve_cmd);
    }

    time_last_inj_valve_cmd = millis();
}
