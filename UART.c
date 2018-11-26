#include "UART.h"

#ifndef F_CPU
  #define F_CPU 16000000UL
#endif

void(*usart_rxc_vect)(void);

void UART_begin(uint16_t baud) {
  #define USART_BAUDRATE baud
  #define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)	
	UBRRH = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UCSRB = ((1<<TXEN)|(1<<RXEN) | (1<<RXCIE));
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes
}
void UART_attach(void (*userFunc)(void)) {
	usart_rxc_vect = userFunc;
}
void UART_print(const char *str) {
	while(*str){
    UART_putchar(*str);
    str++;
  } 
}
void UART_println(const char *str) {
  while(*str){
    UART_putchar(*str);
    str++;
  }
  UART_putchar('\n');
}
void UART_putchar(char ch){
	while(bit_is_clear(UCSRA,UDRE)){}
	UDR = ch;
}
void UART_enable_rx(){
	UCSRB |= (1 << RXCIE); // Enable the USART Recieve Complete interrupt (USART_RXC)
	UCSRB |= (1 << RXEN); // Enable the USART Reciever
}
void UART_disable_rx(){
	UCSRB &= ~(1 << RXCIE); // Disable the USART Recieve Complete interrupt (USART_RXC)
	UCSRB &= ~(1 << RXEN); // Disable the USART Reciever
}
ISR(USART_RXC_vect){
	usart_rxc_vect();
}