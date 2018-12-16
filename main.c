#include "config.h"
#include "init.h"
#include "platform.h"
#include "analog.h"
#include "uart.h"

#include <string.h>

int main() {
    //initialization functions
    init_pins();
    init_oscillator();
    init_adc();
    init_interrupts();
    init_uart();

    LED_3_OFF();

    //program loop
    while(1) {
        if(uart_byte_available()) {
            char c = uart_read_byte();
            switch(c) {
            case '1':
                LED_1_ON();
                LED_2_OFF();
                LED_3_OFF();
                break;
            case '2':
                LED_1_OFF();
                LED_2_ON();
                LED_3_OFF();
                break;
            case '3':
                LED_1_OFF();
                LED_2_OFF();
                LED_3_ON();
                break;
            default:
                LED_1_OFF();
                LED_2_OFF();
                LED_3_OFF();
                break;
            }
        }
    }

    //unreachable
    return 0;
}
