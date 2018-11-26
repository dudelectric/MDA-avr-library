#ifndef TIMER2_H
#define TIMER2_H

void Timer2_set(unsigned long ms, void(*f)(void));
void Timer2_start();
void Timer2_stop();

#endif

