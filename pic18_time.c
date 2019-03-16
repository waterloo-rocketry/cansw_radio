#include "pic18_time.h"
#include <xc.h>

#define MILLIS_INCREMENT 0
#define MILLIS_REMAINDER 64
#define MILLIS_REMAINDER_CAP 125

static uint32_t millis_counter = 0;

//max value for this counter is (125-1)+64, so we can use 8 bits
static uint8_t internal_count = 0;

uint32_t millis(void)
{
    return millis_counter;
}

/*
 * we can read the value of TMR0L to get more precision on the time count. That
 * register increments at 500kHz, up to a max value of 255. The internal_count
 * variable maps 255 of the register counts into 64, which is approximately a
 * division by 4. So the microseconds count is approximately
 *
 * millis_counter * 1000 + (TMR0L + internal_count * 4)
 *
 * We don't really care about how perfectly accurate this count is, but it does
 * have to be monotonically increasing. Which is why it doesn't really matter
 * that the max value of (TMR0L + internal_count * 4) is 751, not 999. We know
 * that this formula monotonically increases, because if TMR0L overflows, the
 * parenthetical part of the equation goes from (255 + internal_count * 4) to
 * ((internal_count+64) * 4) which equals 256 + internal_count*4, which is
 * bigger. If internal count overflows, we go from millis_counter * 1000 +
 * (...) to (millis_counter+1) * 1000 + (whatever).  The max value of whatever
 * is 751, so the extra millis counter must be make it higher
 *
 * There still is the problem that we could read TMR0L, then an interrupt could
 * fire (TMR0L overflows), then we read internal_count. So we read TMR0L before
 * the overflow, and internal_count after, so both are too high. This breaks
 * the whole monotinicity thing we were trying to establish. Instead, we just
 * disable the timer while we're reading the three values. The core frequency
 * is running at several megahertz, and this timer is incrementing at 500kHz,
 * so worst case is we miss one tick, and our timer counter loses 2
 * microseconds of time. In the average case (since it should take far less
 * than half a microsecond to read three values and write two) we won't lose
 * any time. That's probably fine. The other option is to read the three values
 * then check for overflow (read internal_count, then TMR0L, then
 * millis_counter, then internal_count again), but that's more trouble than I
 * think it's worth
 */
uint32_t micros(void)
{
    //TODO: maybe instead of disabling the timer, disable the interrupt. That
    //will reduce the number of missing microseconds by a factor of 256
    T0CON0bits.EN = 0;
    uint8_t timer_val = TMR0L;
    uint8_t internal_val = internal_count;
    uint32_t millis_val = millis_counter;
    T0CON0bits.EN = 1;

    return millis_val * 1000 + timer_val + ((uint32_t) internal_val) * 4;
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
    millis_counter += MILLIS_INCREMENT;
    internal_count += MILLIS_REMAINDER;
    if (internal_count > MILLIS_REMAINDER_CAP) {
        internal_count -= MILLIS_REMAINDER_CAP;
        millis_counter++;
    }
}
