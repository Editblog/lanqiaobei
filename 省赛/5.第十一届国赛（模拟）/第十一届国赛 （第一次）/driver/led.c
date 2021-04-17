#include "led.h"


void led_off(unsigned char Led){
	P2 = (P2 & 0x1f) | 0x80;
	switch(Led){
		case L1:
			P00 = 1;
			break;
		case L2:
			P01 = 1;
			break;
		case L3:
			P02 = 1;
			break;
		case L4:
			P03 = 1;
			break;
		case L5:
			P04 = 1;
			break;
		case L6:
			P05 = 1;
			break;
		case L7:
			P06 = 1;
			break;
		case L8:
			P07 = 1;
			break;
	}
	P2 = (P2 & 0x1f);
}

void led_on(unsigned char Led){
	P2 = (P2 & 0x1f) | 0x80;
	switch(Led){
		case L1:
			P00 = 0;
			break;
		case L2:
			P01 = 0;
			break;
		case L3:
			P02 = 0;
			break;
		case L4:
			P03 = 0;
			break;
		case L5:
			P04 = 0;
			break;
		case L6:
			P05 = 0;
			break;
		case L7:
			P06 = 0;
			break;
		case L8:
			P07 = 0;
			break;
	}
	P2 = (P2 & 0x1f);
}
