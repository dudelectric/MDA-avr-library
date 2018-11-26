#include "Timer0.h"
#include <avr/interrupt.h>

volatile uint32_t millis_prv = 0;

void Timer0_init(){
	TCCR0 = _BV(CS01) | _BV(CS00);
	TIFR &= ~TOV0;
	TCNT0 = 0x0;
	TIMSK = _BV(TOIE0);
	sei();
}

ISR(TIMER0_OVF_vect){
	// reset the counter (overflow is cleared automatically)
	TCNT0 = (uint8_t)(0xFF - ((F_CPU/8)/1000)); // use CLKio/8 prescaler (set CS0n accordingly above)
	millis_prv++;
}

uint32_t Timer0_millis(){
	uint32_t m;
	cli();
	m = millis_prv;
	sei();
	return m;
}
