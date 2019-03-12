#ifndef PIC18_TIME_H
#define PIC18_TIME_H

#include <stdint.h>

/*
 * Returns the number of milliseconds that have happened since the
 * microcontroller woke up.
 */
uint32_t millis(void);

/*
 * Returns the number of microseconds since we woke up
 */
uint32_t micros(void);

/*
 * Interrupt handler for timer 0 interrupt. Do not call from application code
 */
void timer0_handle_interrupt(void);

#endif
