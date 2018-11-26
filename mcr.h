#include <stdbool.h>
#include <avr/io.h>

#ifndef MCR_H
#define MCR_H

#define DDR_CLS  DDRD 
#define PORT_CLS PORTD
#define PIN_CLS  PIND  
#define BIT_CLS  4
#define cls_is_low() (bit_is_clear(PIN_CLS, BIT_CLS)) 

void mcr_begin(unsigned char track);
void mcr_restart();
void mcr_stop();
bool mcr_available();
short mcr_get_dataKartu();
short mcr_read(char *data, unsigned char size);

#define DATA_BUFFER_LEN  25
char mcr_dataKartu[DATA_BUFFER_LEN];

#endif  /* MCR_H */


