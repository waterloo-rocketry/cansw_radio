#ifndef INIT_H_
#define	INIT_H_

//Set up IO pin registers
void init_pins();

//Set up proper oscillator registers
void init_oscillator();

//set up the ADC
void init_adc(void);

//set up all the interrupt related registers
void init_interrupts();

#endif	/* INIT_H */

