#include "error.h"

static char err_msg_ring_buf[ERROR_MESSAGE_RING_BUFFER_SIZE]
    [ERROR_COMMAND_LENGTH];
static uint8_t err_msg_buf_size = 0;
static uint8_t err_msg_buf_write = 0;
static uint8_t err_msg_buf_read = 0;

void report_error(uint8_t board_id,
                  enum BOARD_STATUS error_type,
                  uint8_t byte4, uint8_t byte5,
                  uint8_t byte6, uint8_t byte7)
{

    error_t err;
    err.board_id = board_id;
    err.err_type = error_type;
    err.byte4 = byte4;
    err.byte5 = byte5;
    err.byte6 = byte6;
    err.byte7 = byte7;

    char *serialized_error = err_msg_ring_buf[err_msg_buf_write];
    serialize_error(&err, serialized_error);
    ++err_msg_buf_size;
    ++err_msg_buf_write;
}

bool get_next_serialized_error(char *output)
{
    if(err_msg_ring_buf == NULL) return false;
    if(err_msg_buf_size == 0) return false;

    char *serialized_error = err_msg_ring_buf[err_msg_buf_read];
    memcpy(output, serialized_error, ERROR_COMMAND_LENGTH);

    ++err_msg_buf_read;
    --err_msg_buf_size;

    return true;
}
