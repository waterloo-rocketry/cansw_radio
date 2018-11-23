#ifndef LEDS_H
#define	LEDS_H

#include <xc.h>

//macros to do things
#define LED_1_ON()  do{ LATC1 = 0; }while(0)
#define LED_1_OFF() do{ LATC1 = 1; }while(0)
#define LED_2_ON()  do{ LATA4 = 0; }while(0)
#define LED_2_OFF() do{ LATA4 = 1; }while(0)
#define LED_3_ON()  do{ LATA5 = 0; }while(0)
#define LED_3_OFF() do{ LATA5 = 1; }while(0)

#define BUS_POWER_ON()  do{ LATA2 = 1; }while(0)
#define BUS_POWER_OFF() do{ LATA2 = 0; }while(0)

#endif	/* LEDS_H */

