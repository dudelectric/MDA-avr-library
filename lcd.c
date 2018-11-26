//http://github.com/dudelectric
//2017

#include <util/delay.h>
#include <avr/io.h>
#include "lcd.h"

#define LCD_DISPLAY_CLEAR		0x01
#define LCD_CURSOR_HOME			0x02
#define LCD_ENTRY_MODE_SET      0x04
#define LCD_ENTRY_INCREMENT     (0x02 | LCD_ENTRY_MODE_SET)
#define LCD_ENTRY_DECREMENT     (0 | LCD_ENTRY_MODE_SET)
#define LCD_ENTRY_SHIFT         (0x01 | LCD_ENTRY_MODE_SET)
#define LCD_ENTRY_NOTSHIFT      (0 | LCD_ENTRY_MODE_SET)
#define LCD_DISPLAY_ON_OFF      0x08
#define LCD_DISPLAY_ON          (0x04 | LCD_DISPLAY_ON_OFF)
#define LCD_DISPLAY_OFF         (0 | LCD_DISPLAY_ON_OFF)
#define LCD_CURSOR_ON           (0x02 | LCD_DISPLAY_ON_OFF)
#define LCD_CURSOR_OFF          (0 | LCD_DISPLAY_ON_OFF)
#define LCD_POSITION_BLINK      (0x01 | LCD_DISPLAY_ON_OFF)
#define LCD_POSITION_NOTBLINK   (0 | LCD_DISPLAY_ON_OFF)
#define LCD_CURSOR_DISPLAY_SHIFT   0x10
#define LCD_DISPLAY_SHIFT          (0x08 | LCD_CURSOR_DISPLAY_SHIFT)
#define LCD_CURSOR_SHIFT           (0 | LCD_CURSOR_DISPLAY_SHIFT)
#define LCD_RIGHT_SHIFT            (0x04 | LCD_CURSOR_DISPLAY_SHIFT)
#define LCD_LEFT_SHIFT             (0 | LCD_CURSOR_DISPLAY_SHIFT)
#define LCD_FUNCTION_SET		0x28
#define LCD_8BIT				(0x10 | LCD_FUNCTION_SET)
#define LCD_4BIT				(0 | LCD_FUNCTION_SET)
#define LCD_CGRAM_ADDR_SET		0x40
#define LCD_DDRAM_ADDR_SET		0x80

#define RS_on()    RS_PORT |= _BV(RS_PIN)
#define EN_on()    EN_PORT |= _BV(EN_PIN)
#define D4_on()    D4_PORT |= _BV(D4_PIN)
#define D5_on()    D5_PORT |= _BV(D5_PIN)
#define D6_on()    D6_PORT |= _BV(D6_PIN)
#define D7_on()    D6_PORT |= _BV(D7_PIN)
#define EN_off()   EN_PORT &= ~_BV(EN_PIN)
#define RS_off()   RS_PORT &= ~_BV(RS_PIN) 
#define D4_off()   D4_PORT &= ~_BV(D4_PIN)
#define D5_off()   D5_PORT &= ~_BV(D5_PIN)
#define D6_off()   D6_PORT &= ~_BV(D6_PIN)
#define D7_off()   D6_PORT &= ~_BV(D7_PIN)

#define LCD_DLY_PLUS    5
#define nible_hi(byte)   (byte >> 4)
#define nible_lo(byte)   (byte & 0x0F)
#define ir_select()      RS_off()
#define dr_select()      RS_on()
//******************************************
#define ddr(port)	_SFR_IO8(_SFR_IO_ADDR(port)-1)
static void lcd_write_nible(uint8_t nib){
	EN_on();
	if(nib & _BV(0)){

		D4_on();
	}
	else{
		D4_off();
	}
	if(nib & _BV(1)){
		D5_on();
	}
	else{
		D5_off();
	}
	if(nib & _BV(2)){
		D6_on();
	}
	else{
		D6_off();
	}
	if(nib & _BV(3)){
		D7_on();
	}
	else{
		D7_off();
	}

	EN_off();
	_delay_us(40);
}
static void lcd_write_byte(uint8_t B){
	lcd_write_nible(nible_hi(B));
	lcd_write_nible(nible_lo(B));
}
static void lcd_write_ir_nible(uint8_t IR){
	ir_select();
	lcd_write_nible(IR);
}
static void lcd_write_dr_byte(uint8_t DR){//data
	dr_select();
	lcd_write_byte(DR);
}
static void lcd_write_ir_byte(uint8_t IR){//instruction
	ir_select();
	lcd_write_byte(IR);
	if(IR == LCD_DISPLAY_CLEAR || LCD_CURSOR_HOME)
	_delay_us(1600);
}
void lcd_init(){
	ddr(RS_PORT) |= _BV(RS_PIN);
	ddr(EN_PORT) |= _BV(EN_PIN);
	ddr(D4_PORT) |= _BV(D4_PIN);
	ddr(D5_PORT) |= _BV(D5_PIN);
	ddr(D6_PORT) |= _BV(D6_PIN);
	ddr(D7_PORT) |= _BV(D7_PIN);

	_delay_us(1500+LCD_DLY_PLUS);

	lcd_write_ir_nible( nible_hi(LCD_8BIT) );
	_delay_us(4100+LCD_DLY_PLUS);
	lcd_write_ir_nible( nible_hi(LCD_8BIT) );
	_delay_us(100+LCD_DLY_PLUS);
	lcd_write_ir_nible( nible_hi(LCD_8BIT) );

	lcd_write_ir_nible( nible_hi(LCD_4BIT) );
	lcd_write_byte(LCD_4BIT);
	lcd_write_byte(LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_POSITION_NOTBLINK);
	lcd_write_ir_byte(LCD_ENTRY_INCREMENT);
}
void lcd_putch(const char CHR){
	lcd_write_dr_byte(CHR);
}
void lcd_putstr(const char* str){

	while(*str){
		lcd_putch(*str++);
	}
}
#define col_num  20
#define lcd_row_addr(row)  ( row%2? (row>>1)*col_num+0x40: (row>>1)*col_num )
void lcd_gotoxy(uint8_t row, uint8_t col){
	row--;
	col--;
	lcd_write_ir_byte(LCD_DDRAM_ADDR_SET | lcd_row_addr(row)|col);
}
void lcd_display_clear(){
	lcd_write_ir_byte(LCD_DISPLAY_CLEAR);
}

void lcd_home(){
	lcd_write_ir_byte(LCD_CURSOR_HOME);
}

void lcd_cursor_off_blink_off(){
	lcd_write_ir_byte(LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_POSITION_NOTBLINK);
}

void lcd_cursor_on_blink_on(){
	lcd_write_ir_byte(LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_POSITION_BLINK);
}

















