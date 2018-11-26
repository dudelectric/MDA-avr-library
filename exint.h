#include "config.h"

#ifndef EXTINT_H
#define EXTINT_H

#define LOW       0
#define CHANGE    1
#define FALLING   2
#define RISING    3

void exint_attach(uint8_t interruptNum, void (*userFunc)(void), uint8_t mode); 
void exint_detach(uint8_t interruptNum);

#endif
