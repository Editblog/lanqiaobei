#include <stc15f2k60s2.h>
#include "intrins.h"
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"
#define u8 unsigned char
#define u16 unsigned int
	
u8 code du[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 code wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
sbit TX = P1^0;
sbit RX = P1^1;
void All_Init(void)
{
	//LED
    P2 &= 0X1F;   //关573
	P0 =  0XFF;   //预送数据
	P2 |= 0X80;   //开相应573
	P0 =  0XFF;   //送数据
	P2 &= 0X1F;   //关573
	
	//蜂鸣器
    P2 &= 0X1F;   //关573
	P0 =  0X00;   //预送数据
	P2 |= 0XA0;   //开相应573
	P0 =  0X00;   //送数据
	P2 &= 0X1F;   //关573
	
	//数码管位选
	P2 &= 0X1F;   //关573
	P0 =  0X00;   //预送数据
	P2 |= 0XC0;   //开相应573
	P0 =  0X00;   //送数据
	P2 &= 0X1F;   //关573
	
	//数码管段选
	P2 &= 0X1F;   //关573
	P0 =  0XFF;   //预送数据
	P2 |= 0XE0;   //开相应573
	P0 =  0XFF;   //送数据
	P2 &= 0X1F;   //关573
}

void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

#define state_0 0 // 消抖
#define state_1 1 //按键是否按下
#define state_2 2 //按键弹起
u8 key_num = 0xff;
void key_scanf(void)
{
	static u8 stats = 0;
	u8 key_x,key_y=0;
	switch(stats)
	{
		case state_0:
			P3 = 0x0f;P42=0;P44=0;
			if(P3!=0x0f)
				stats = state_1;
			break;
		case state_1:
			if(!P30|!P31|!P32|!P33)
			{
				if(!P30) key_y = 4;
				if(!P31) key_y = 3;
				if(!P32) key_y = 2;
				if(!P33) key_y = 1;
			}
			P3 = 0xf0;P42=1;P44=1;
			if(!P34|!P35|!P42|!P44)
			{
				if(!P34) key_x = 4;
				if(!P35) key_x = 3;
				if(!P42) key_x = 2;
				if(!P44) key_x = 1;
				key_num = key_x*4 +key_y -5;
				stats = state_2;
			}
			else
				stats = state_0;
			break;
		case state_2:
			P3 = 0x0f;P42=0;P44=0;
			if(P3==0x0f)
				stats = state_0;
			break;
	}
}



void Delay12us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 30;
	while (--i);
}

void send_wave(void)
{
	unsigned char i = 1;
	do
	{
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
	while(i--);
}
u8 smg_time[8];
u8 smg_adc[8];
u8 shi,fen,miao;
u8 adc1,adc3;
bit adc_flag=0;
u8 AT24C02=0;
u8 smg_test[8];
u8 temp=0;

u16 distance,time;
u8 smg_distance[8];
bit sonic_flag;

u8 mode=0;
void main()
{
	All_Init();
	set_srf(23,59,55);
	
	AT24C02=read_24c02(0x01);
	smg_test[0]=du[AT24C02/10];
	smg_test[1]=du[AT24C02%10];
	write_24c02(0x01,++AT24C02);
	Timer0Init();
	
	write_adc(255);
	
	while(1)
	{
		EA=0;
		miao=Read_Ds1302_Byte(0x81);
		fen=Read_Ds1302_Byte(0x83);
		shi=Read_Ds1302_Byte(0x85);
		EA=1;
		smg_time[0]=du[shi/16];smg_time[1]=du[shi%16];smg_time[2]=0x40;
		smg_time[3]=du[fen/16];smg_time[4]=du[fen%16];smg_time[5]=0x40;
		smg_time[6]=du[miao/16];smg_time[7]=du[miao%16];
		
		if(adc_flag)
		{
			adc_flag=0;
			adc1=read_adc(0x01);
			adc3=read_adc(0x03);
			smg_adc[0]=du[adc1/100];smg_adc[1]=du[adc1/10%10];smg_adc[2]=du[adc1%10];
			smg_adc[5]=du[adc3/100];smg_adc[6]=du[adc3/10%10];smg_adc[7]=du[adc3%10]; 
			write_adc(adc3);
		}
		
		temp= (unsigned char)rd_temperature_f();
		smg_test[6]=du[temp/10];
		smg_test[7]=du[temp%10];
		
		switch(key_num)
		{
			case 0:				
				mode++;
				if(mode==4)	mode=0;
				key_num=0xff;
				break;
			case 1:				
				mode=1;
				key_num=0xff;
				break;
			case 2:				
				mode=2;
				key_num=0xff;
				break;
			case 3:				
				mode=3;
				key_num=0xff;
				break;
			case 4:				
				mode=0;
				key_num=0xff;
				break;
			default:
			break;
		}
		
		if(sonic_flag)
		{
			sonic_flag = 0;
			send_wave(); 
			TR1 = 1;
			while((RX == 1) && (TF1 == 0));
			TR1 = 0;
			if(TF1 == 1)
			{
				TF1 = 0;
			}
			else
			{
				time = TH1;
				time <<= 8;
				time |= TL1;
				distance = (unsigned int)(time*0.018);	
			}
			TH1 = 0;
			TL1 = 0;
						
			smg_distance[5]=du[distance/100]|0x80;
			smg_distance[6]=du[distance%100/10];
			smg_distance[7]=du[distance%10];
		}
	}
}

void time0() interrupt 1
{
	static int smg_count=0,key_count=0,i=0,adc_count=0,sonic_count=0;
	smg_count++; key_count++;adc_count++;sonic_count++;
	if(key_count==8)
	{
		key_scanf();
		key_count=0;
	}
	
	if(adc_count==100)
	{
		adc_count=0;
		adc_flag=1;
	}
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(mode==0)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_time[i];P2&=0x1f;
		}
		else if(mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_adc[i];P2&=0x1f;
		}
		else if(mode==2)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_test[i];P2&=0x1f;
		}
		else if(mode==3)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_distance[i];P2&=0x1f;
		}
		
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
	
	if(sonic_count==50)
	{
		sonic_count=0;
		sonic_flag=1;
	}
}