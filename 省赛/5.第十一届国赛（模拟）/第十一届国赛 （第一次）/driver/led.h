#ifndef __LED_H__
#define __LED_H__

#include <STC15F2K60S2.H>

#define L1 1
#define L2 2
#define L3 3
#define L4 4
#define L5 5
#define L6 6
#define L7 7
#define L8 8
void led_off(unsigned char Led);
void led_on(unsigned char Led);
#endif