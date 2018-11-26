#include "lpd8806.h"
#include "Timer0.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>//memset
#include <stdlib.h>//malloc
#include <stdbool.h>//bool

//proto
void lpd_updateLength(uint16_t n);
void lpd_startBitbang();

uint16_t numLEDs, numuint8_ts;
uint8_t *pixels;
uint8_t lpd_mode;
uint8_t lpd_red;
uint8_t lpd_green;
uint8_t lpd_blue;


void lpd_begin(uint8_t n) {
	pixels = 0;
	lpd_updateLength(n);
	lpd_startBitbang();
	lpd_mode = 1;
}
void lpd_startBitbang() {
	lpd_di_pin_init();//DDRC |= _BV(3);
	lpd_ci_pin_init();//DDRC |= _BV(2);
	lpd_di_pin_low();//PORTC &= ~_BV(3); // Data is held low throughout (latch = 0)
	for(uint16_t i=((numLEDs+31)/32)*8; i>0; i--) {
		lpd_ci_pin_high();//PORTC |=  _BV(2);
		lpd_ci_pin_low();//PORTC &= ~_BV(2);
	}
}
void lpd_updateLength(uint16_t n) {
	uint8_t  latchuint8_ts;
	uint16_t datauint8_ts, totaluint8_ts;

	numLEDs = numuint8_ts = 0;
	if(pixels) free(pixels); // Free existing data (if any)

	datauint8_ts  = n * 3;
	latchuint8_ts = (n + 31) / 32;
	totaluint8_ts = datauint8_ts + latchuint8_ts;
	if((pixels = (uint8_t *)malloc(totaluint8_ts))) { // Alloc new data
		numLEDs  = n;
		numuint8_ts = totaluint8_ts;
		memset( pixels           , 0x80, datauint8_ts);  // Init to RGB 'off' state
		memset(&pixels[datauint8_ts], 0   , latchuint8_ts); // Clear latch uint8_ts
	}
}
uint16_t lpd_numPixels(void) {
	return numLEDs;
}
void lpd_show(void) {
	uint8_t  *ptr = pixels;
	uint16_t i    = numuint8_ts;
	uint8_t p, bit;

	while(i--) {
		p = *ptr++;
		for(bit=0x80; bit; bit >>= 1) {
			if(p & bit) lpd_di_pin_high();//PORTC |=  _BV(3);
			else        lpd_di_pin_low();//PORTC &= ~_BV(3);
			lpd_ci_pin_high();//PORTC |=  _BV(2);
			lpd_ci_pin_low();//PORTC &= ~_BV(2);
		}
	}
}
uint32_t lpd_Color(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)(g | 0x80) << 16) |
	((uint32_t)(r | 0x80) <<  8) |
	b | 0x80 ;
}
void lpd_setPixelColor2(uint16_t n, uint32_t c) {
	if(n < numLEDs) {
		uint8_t *p = &pixels[n * 3];
		*p++ = (c >> 16) | 0x80;
		*p++ = (c >>  8) | 0x80;
		*p++ =  c        | 0x80;
	}
}

////////application layer 
uint32_t lpd_wheel(uint16_t WheelPos){
	uint8_t r, g, b;
	switch(WheelPos / 128)
	{
		case 0:
		r = 127 - WheelPos % 128;   //Red down
		g = WheelPos % 128;      // Green up
		b = 0;                  //blue off
		break;
		case 1:
		g = 127 - WheelPos % 128;  //green down
		b = WheelPos % 128;      //blue up
		r = 0;                  //red off
		break;
		case 2:
		b = 127 - WheelPos % 128;  //blue down
		r = WheelPos % 128;      //red up
		g = 0;                  //green off
		break;
	}
	return(lpd_Color(r,g,b));
}

uint16_t I = 0;
uint16_t J = 0;
uint32_t previousMillis;
uint8_t rc_waits = 4;
uint8_t r_waits = 10;
bool blink_state = true;
uint8_t prev_mode;

void lpd_rainbowCycle_nb() {
	if(I < lpd_numPixels()){
		lpd_setPixelColor2(I, lpd_wheel( ((I * 384 / lpd_numPixels()) + J) % 384) );
		I++;
		if(I == lpd_numPixels()){
			I = 0;
		}
		uint32_t currentMillis = Timer0_millis();
		if((currentMillis - previousMillis > rc_waits) && (J < 384 * 5)){
			previousMillis = currentMillis;
			lpd_show();
			J++;
			if(J == 384 * 5){
				J = 0;
			}
		}
	}
}
void lpd_rainbow_nb() {
	if(I < lpd_numPixels()){
		lpd_setPixelColor2(I, lpd_wheel( (I + J) % 384));
		I++;
		if(I == lpd_numPixels()){
			I = 0;
		}
		uint32_t currentMillis = Timer0_millis();
		if((currentMillis - previousMillis > r_waits) && (J < 384)){
			previousMillis = currentMillis;
			lpd_show();
			J++;
			if(J == 384){
				J = 0;
			}
		}
	}
}
void lpd_rgb_nb() {
	for(register int i=0; i<lpd_numPixels(); i++) {
		lpd_setPixelColor2(i, lpd_Color(lpd_red, lpd_green, lpd_blue));
		lpd_show();
	}
}
void lpd_clear(){
	for(register int i=0; i<lpd_numPixels(); i++) {
		lpd_setPixelColor2(i, lpd_Color(0, 0, 0));
		lpd_show();
	}
}
void lpd_blinker(){
	if(I < 12){
		uint32_t currentMillis = Timer0_millis();
		if(currentMillis - previousMillis > 100){
			previousMillis = currentMillis;
			if(blink_state){
				for(register uint8_t i=0; i<lpd_numPixels(); i++) {
					lpd_setPixelColor2(i, lpd_Color(255, 255, 255));
				}
			}
			else{
				for(register uint8_t i=0; i<lpd_numPixels(); i++) {
					lpd_setPixelColor2(i, lpd_Color(0, 0, 0));
				}
			}
			lpd_show();	
			blink_state = !blink_state;
			I++;
			if(I == 12){
				lpd_set_mode(prev_mode);
			}
		}
	}
}
void lpd_update(){
	switch(lpd_mode) {

		case 1:
		lpd_rainbowCycle_nb();
		break; 
		
		case 2:
		lpd_rainbow_nb();
		break;
		
		case 3:
		lpd_rgb_nb();
		break;
		
		case 4:
		lpd_blinker();
		break;

	}
}
void lpd_set_color(uint8_t _r, uint8_t _g, uint8_t _b){
	lpd_red = _r;
	lpd_green = _g;
	lpd_blue = _b;
}
void lpd_set_mode(uint8_t _mode){
	prev_mode = lpd_mode;
	lpd_mode = _mode;
	lpd_clear();
	I=0;
	J=0;
	previousMillis = 0;
}
