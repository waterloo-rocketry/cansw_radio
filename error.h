#ifndef RADIO_ERROR_H_
#define RADIO_ERROR_H_

#include <stdint.h>
#include "message_types.h"

void report_error(uint8_t board_id,
                  enum BOARD_STATUS error_type,
                  uint8_t byte4, uint8_t byte5,
                  uint8_t byte6, uint8_t byte7);
#endif
