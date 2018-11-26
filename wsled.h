#ifndef WSLED_H
#define WSLED_H

#include <stdbool.h>

//typedef
typedef struct rgb_color
{
	unsigned char green, red, blue;
} rgb_color;

//defines
#define LED_COUNT 10

void ws_set_mode(uint8_t _mode);
void ws_update();
uint8_t ws_get_mode();

#endif