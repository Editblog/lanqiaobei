#include "led.h"
#include "intrins.h"

void delayus(unsigned int n)		//@11.0592MHz
{
	do{
		_nop_();
		_nop_();
		_nop_();
	}while(n--);
}

void delayms(unsigned int n)		//@11.0592MHz
{
	unsigned char i, j;
	do{
		_nop_();
		_nop_();
		_nop_();
		i = 11;
		j = 190;
		do
		{
			while (--j);
		} while (--i);
	}while(n--);
}


