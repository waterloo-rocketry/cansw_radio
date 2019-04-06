#ifndef INIT_H_
#define	INIT_H_

//Set up IO pin registers
void init_pins(void);

//Set up proper oscillator registers
void init_oscillator(void);

//set up the ADC
void init_adc(void);

//set up timer 0, which is used for millis() function
void init_timer0(void);

//set up all the interrupt related registers
void init_interrupts(void);

#endif	/* INIT_H */

