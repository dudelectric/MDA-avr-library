#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "wsled.h"
#include "Timer0.h"
#include "config.h"

//vars
rgb_color colors[LED_COUNT];
uint16_t K = 0;
uint16_t L = 0;
uint32_t previousMillis;
bool blnk_state = true;
static volatile uint8_t prev_mode;
uint8_t ws_mode;

void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count)
{
	// Set the pin to be an output driving low.
	LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
	LED_STRIP_DDR |= (1<<LED_STRIP_PIN);

	cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
	while(count--)
	{
		// Send a color to the LED
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		asm volatile(
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0\n"
		"rcall send_led_strip_byte%=\n"  // Send red component.
		"ld __tmp_reg__, -%a0\n"
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"rcall send_led_strip_byte%=\n"  // Send blue component.
		"rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

		// send_led_strip_byte subroutine:  Sends a byte to the LED
		"send_led_strip_byte%=:\n"
		"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
		"ret\n"

		"send_led_strip_bit%=:\n"
		#if F_CPU == 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif
		"sbi %2, %3\n"                           // Drive the line high.

		#if F_CPU != 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif

		#if F_CPU == 16000000
		"nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU != 8000000
		#error "Unsupported F_CPU"
		#endif

		"brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

		#if F_CPU == 8000000
		"nop\n" "nop\n"
		#elif F_CPU == 16000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n"
		#endif

		"brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

		"ret\n"
		"led_strip_asm_end%=: "
		: "=b" (colors)
		: "0" (colors),         // %a0 points to the next color to display
		"I" (_SFR_IO_ADDR(LED_STRIP_PORT)),   // %2 is the port register (e.g. PORTC)
		"I" (LED_STRIP_PIN)     // %3 is the pin number (0-8)
		);
	}
	sei();          // Re-enable interrupts now that we are done.
	_delay_us(80);  // Send the reset signal.
}
rgb_color ws_wheel(uint8_t WheelPos) {
	WheelPos = 255 - WheelPos;
	if(WheelPos < 85) {
		//return Color(255 - WheelPos * 3, 0, WheelPos * 3);
		return (rgb_color){255 - WheelPos * 3, 0, WheelPos * 3};
	}
	if(WheelPos < 170) {
		WheelPos -= 85;
		return (rgb_color){0, WheelPos * 3, 255 - WheelPos * 3};
	}
	WheelPos -= 170;
	return (rgb_color){WheelPos * 3, 255 - WheelPos * 3, 0};
}
void ws_clear(){
	for (register uint8_t c = 0; c < LED_COUNT; c++){
		colors[c] = (rgb_color){0, 0, 0};
	}
	led_strip_write(colors, LED_COUNT);
}
void ws_set_mode(uint8_t _mode){
	prev_mode = ws_mode;
	ws_mode = _mode;
	ws_clear();
	K = 0;
	L = 0;
	previousMillis = 0;
}
void ws_rainbow() {
	if(K < LED_COUNT){
		for(uint8_t i = 0; i < LED_COUNT; i++)
		{
			colors[i] = ws_wheel(((i * 256 / LED_COUNT) + L) & 255);
		}
		K++;
		if(K == LED_COUNT){
			K = 0;
		}
		uint32_t currentMillis = Timer0_millis();
		if((currentMillis - previousMillis > 8) && (L < 256 * 5)){
			previousMillis = currentMillis;
			led_strip_write(colors, LED_COUNT);
			L++;
			if(L == 256 * 5){
				L = 0;
			}
		}
	}
}
void ws_blinker(){
	if(K < 12){
		uint32_t currentMillis = Timer0_millis();
		if(currentMillis - previousMillis > 100){
			previousMillis = currentMillis;
			if(blnk_state){
				for(uint8_t register i = 0; i < LED_COUNT; i++)
				{
					colors[i] = (rgb_color){0, 0, 0};
				}
			}
			else{
				for(uint8_t register i = 0; i < LED_COUNT; i++)
				{
					colors[i] = (rgb_color){255, 255, 255};
				}
			}
			led_strip_write(colors, LED_COUNT);
			blnk_state = !blnk_state;
			K++;
			if(K == 12){
				ws_set_mode(prev_mode);
			}
		}
	}
}
void ws_runner(){
	uint32_t currentMillis = Timer0_millis();
	if(currentMillis - previousMillis > 100){
		previousMillis = currentMillis;
		if(blnk_state){
			for(uint8_t register i = 0; i < LED_COUNT; i++)
			{
				if(i%2){
					colors[i] = (rgb_color){0, 0, 255};
					}else{
					colors[i] = (rgb_color){0, 0, 0};
				}
			}
		}
		else{
			for(uint8_t register i = 0; i < LED_COUNT; i++)
			{
				if(i%2){
					colors[i] = (rgb_color){0, 0, 0};
					}else{
					colors[i] = (rgb_color){0, 0, 255};
				}
			}
		}
		led_strip_write(colors, LED_COUNT);
		blnk_state = !blnk_state;
	}
}
void ws_update(){
	switch(ws_mode) {
		
		case 1:
		ws_rainbow();
		break;
		
		case 2:
		ws_runner();
		break;
		
		case 3:
		ws_blinker();
		break;
	}
}

uint8_t ws_get_mode(){
	return ws_mode;
}