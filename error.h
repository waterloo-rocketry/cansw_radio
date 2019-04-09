#ifndef RADIO_ERROR_H_
#define RADIO_ERROR_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h> // For memcpy
#include "message_types.h"

// how long the char array you pass to get_next_serialized_error needs to be
#define ERROR_COMMAND_LENGTH 8

// How many error messages can be buffered at once
#define ERROR_MESSAGE_RING_BUFFER_SIZE 16

typedef struct {
    uint8_t board_id;
    enum BOARD_STATUS err_type;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
} error_t;

void report_error(uint8_t board_id,
                  enum BOARD_STATUS error_type,
                  uint8_t byte4, uint8_t byte5,
                  uint8_t byte6, uint8_t byte7);

bool get_next_serialized_error(char *output);

#endif
