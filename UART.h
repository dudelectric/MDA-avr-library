#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <avr/interrupt.h>

void UART_begin(uint16_t baud);
void UART_attach(void (*userFunc)(void));
void UART_print(const char *str);
void UART_println(const char *str);
void UART_putchar(char ch);
void UART_enable_rx();
void UART_disable_rx();

#endif
