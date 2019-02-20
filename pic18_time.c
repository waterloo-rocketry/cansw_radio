#include "pic18_time.h"

static uint32_t millis_counter = 0;

#define MILLIS_INCREMENT 21
#define MILLIS_REMAINDER 2536
#define MILLIS_REMAINDER_CAP 3000

uint32_t millis(void)
{
    return millis_counter;
}

/*
 * At present, this function gets called every time that the 16 bit timer0
 * overflows. Since that timer ticks at a rate of Fosc/4 (3 MHz), this
 * function is going to be called every 21.8453 milliseconds. What we're going to
 * do is increment the millis count by 21. This runs into a problem wherein we lose
 * those 845.3 microseconds. Which is an issue. So what we do is increment an
 * internal counter by 8453, and whenever that counter is more than 10000, we
 * increment the millisecond counter by 1 and decrement the internal counter by
 * 10000
 */
void timer0_handle_interrupt()
{
    //max value for this counter is 2*8453, so we can use 16 bits
    static uint16_t internal_count = 0;
    //TODO, make these magic numbers calculated from XTAL_FREQ
    millis_counter += MILLIS_INCREMENT;
    internal_count += MILLIS_REMAINDER;
    if (internal_count > MILLIS_REMAINDER_CAP) {
        internal_count -= MILLIS_REMAINDER_CAP;
        millis_counter++;
    }
}
