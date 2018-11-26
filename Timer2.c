#include <avr/interrupt.h>
#include "Timer2.h"

volatile unsigned long tot_overflow_t2;
unsigned long timer2_msecs;
volatile unsigned int timer2_tcnt2;
void (*timer2_func)(void);

void Timer2_set(unsigned long ms, void(*f)(void)){
	float prescaler = 0.0;
	
	TIMSK &= ~(1<<TOIE2);//enable timer2 interrupt
	TCCR2 &= ~((1<<WGM21) | (1<<WGM20));//ctc operation
	TIMSK &= ~(1<<OCIE2);//enable output compare interrupt timer2
	ASSR &= ~(1<<AS2);

	TCCR2 |= (1<<CS22);//set prescaler 64
	TCCR2 &= ~((1<<CS21) | (1<<CS20));
	prescaler = 64.0;

	timer2_tcnt2 = 256 - (int)((float)F_CPU * 0.001 / prescaler);

	if (ms == 0)
	timer2_msecs = 1;
	else
	timer2_msecs = ms;

	timer2_func = f;
}

void Timer2_start() {
	tot_overflow_t2 = 0;

	TCNT2 = timer2_tcnt2;
	TIMSK |= (1<<TOIE2);//enable timer2 overflow interrupt
}

void Timer2_stop() {
	TIMSK &= ~(1<<TOIE2);//disable timer2 overflow interrupt
}

void Timer2_overflow() {
	tot_overflow_t2 ++;
	
	if (tot_overflow_t2 >= timer2_msecs) {
		tot_overflow_t2 = 0;
		timer2_func();
	}
}

ISR(TIMER2_OVF_vect) {
	TCNT2 = timer2_tcnt2;
	Timer2_overflow();
}


