#include "error.h"

void report_error(uint8_t board_id,
                  enum BOARD_STATUS error_type,
                  uint8_t byte4, uint8_t byte5,
                  uint8_t byte6, uint8_t byte7)
{
    //do nothing
}

bool get_next_serialized_error(char *output)
{
    return false;
}
