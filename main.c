#include "config.h"
#include "init.h"
#include "platform.h"

int main() {
    //initialization functions
    init_pins();
    init_oscillator();

    //program loop
    while(1) {
        //turn on the red LED, turn off white and blue
        LED_1_ON();
        LED_2_OFF();
        LED_3_OFF();
        //cut power to the bus
        BUS_POWER_OFF();
        //wait for 10 seconds
        for(int i = 0; i < 100; ++i)
            //__delay_ms gets angry if you pass it large arguments
            //hence 10 __delay_ms(100), and not one __delay_ms(10000)
            __delay_ms(100);
        //turn off the red led, turn on white and blue
        LED_1_OFF();
        LED_2_ON();
        LED_3_ON();
        //power the bus back on
        BUS_POWER_ON();
        //wait for 10 more seconds
        for(int i= 0; i < 100; ++i)
            __delay_ms(100);
    }

    //unreachable
    return 0;
}
