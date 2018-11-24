#ifndef ANALOG_H_
#define ANALOG_H_

#include <stdbool.h>
#include <stdint.h>

void analog_report_value(uint16_t value, uint8_t channel);
bool analog_read_all_values(void);

#endif //ANALOG_H_
