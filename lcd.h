#include "config.h"

#ifndef LCD_H
#define LCD_H

void lcd_init();
void lcd_putch(const char CHR);
void lcd_putstr(const char* STR);
void lcd_gotoxy(uint8_t row, uint8_t col);
void lcd_display_clear();
void lcd_home();
void lcd_cursor_off_blink_off();
void lcd_cursor_on_blink_on();

#endif