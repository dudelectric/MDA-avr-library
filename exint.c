#include <avr/interrupt.h>
#include "exint.h" 

static void(*int0_isr_func)(void); //ISR calback functions
static void(*int1_isr_func)(void); 


void exint_attach(uint8_t interruptNum, void (*userFunc)(void), uint8_t mode){
	switch (interruptNum) {
		case 0:
			DDR_INTR0 &= ~_BV(BIT_INTR0);// make intr pin as input
			PORT_INTR0 |= _BV(BIT_INTR0); //intr 0 pullup active
			int0_isr_func = userFunc;
			GICR |= (1 << INT0);
			MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
		break;
		case 1:
			DDR_INTR1 &= ~_BV(BIT_INTR1);// make intr pin as input	
			PORT_INTR1 |= _BV(BIT_INTR1); //intr 1 pullup active
			int1_isr_func = userFunc;
			GICR |= (1 << INT1);
			MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
		break;
	}
	sei();
}

void exint_detach(uint8_t interruptNum){
	switch (interruptNum) {
		case 0:
    		int0_isr_func = 0;
			GICR &= ~(1 << INT0);
		break;
		case 1:
			int1_isr_func = 0;
			GICR &= ~(1 << INT1);
		break;
	}
}

#if defined(INT0_vect)
ISR(INT0_vect)
{
	if(int0_isr_func)
	int0_isr_func();
}
#endif

#if defined(INT1_vect)
ISR(INT1_vect)
{
	if(int1_isr_func)
	int1_isr_func();
}
#endif

