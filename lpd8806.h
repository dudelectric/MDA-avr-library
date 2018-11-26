#ifndef LPD8806_H
#define LPD8806_H

#include <stdint.h>

	void lpd_begin(uint8_t n);
	void lpd_update();
	void lpd_set_color(uint8_t _r, uint8_t _g, uint8_t _b);
	void lpd_set_mode(uint8_t _mode);
	void lpd_clear();
	void lpd_blinker();

#endif