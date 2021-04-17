#include <stc15f2k60s2.h>
#include "ds1302.h"
#include "onewire.h"
#define u8 unsigned char
#define u16 unsigned int
	
u8 code du[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 code wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

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

//---------------------------四个独立按键--------------------------------
#define btn_io P3
#define btn_state_0 0	  //是否按下
#define btn_state_1 1 	 //是否是抖动
#define btn_state_2 2	  //判定按键有效的种类
#define btn_state_3 3	  //等待按键释放 
#define btn_mask 0x0f	  //屏蔽不需要的IO
#define LONG_KEY_TIME 100 //LONG_KEY_TIME*10MS = 3S
#define SINGLE_KEY_TIME 3 //SINGLE_KEY_TIME*10MS = 30MS
char btnkey;
static char btn_state = 0;
char scanbtn(void)
{
	static int key_time; //按键计时变量	
	static char key_value=0;
	char btn_press,btn_return = 0;
	btn_press = btn_io & btn_mask;
	switch(btn_state)
	{
		case btn_state_0:
			key_value=0;
			if(btn_press != btn_mask){
				key_time = 0; // 清零时间间隔计数
				btn_state = btn_state_1;
			}
			break;
		case btn_state_1:
			if(btn_press != btn_mask)
			{
				key_time++; // 一次10ms
				if (key_time >= SINGLE_KEY_TIME) // 消抖时间为：SINGLE_KEY_TIME*10ms = 30ms;
				{
					btn_state = btn_state_2;
				}
			}
			else
				btn_state = btn_state_0;
			break;
		case btn_state_2:
			if(btn_press == btn_mask)
			{
				btn_return=key_value;
				key_value=0;
				btn_state = btn_state_0; // 去状态3，等待按键释放
			}
			else
			{
				if(btn_press == 0x0e) key_value = 1;
				if(btn_press == 0x0d) key_value = 2;
				if(btn_press == 0x0b) key_value = 3;
				if(btn_press == 0x07) key_value = 4;

				key_time++;

				if (key_time >= LONG_KEY_TIME) // 如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=200*10ms=2000ms）, 则判定为 长按
				{
					btn_return =key_value|0x80;
					key_value=0;
					btn_state = btn_state_3; // 去状态3，等待按键释放
				}
			}
			break;
		case btn_state_3:
			if(btn_press == 0x0f)	btn_state = btn_state_0;
			break;
		default:
		break;
	}
	return 	btn_return;
}

u8 shi,fen,miao;
bit blink_flag=0;
bit temp_flag=0;
bit led_blink_flag=0;

u8 temperature[10];
u8 sample_count=0;

u8 menu1[8],menu2[8],menu3[8];
u8 sample_time[4]={1,5,30,60};
u8 sample_index;
u8 real_sample_time;

u8 menu_index=0;	//默认界面 
u8 display_count=0;
u8 n;

void main()
{
	All_Init();
	set_srf(23,59,50);
	for(n=0;n<10;n++)	rd_temperature_f();
	rd_temperature_f();
	Timer0Init();
	while(1)
	{
		EA=0;
		miao=Read_Ds1302_Byte(0x81);
		fen=Read_Ds1302_Byte(0x83);
		shi=Read_Ds1302_Byte(0x85);
		EA=1;
		
		menu2[0]=du[shi/16];menu2[1]=du[shi%16];
		menu2[3]=du[fen/16];menu2[4]=du[fen%16];
		menu2[6]=du[miao/16];menu2[7]=du[miao%16];
		
		//采集时间间隔显示
		menu1[6]=du[sample_time[sample_index]/10];menu1[7]=du[sample_time[sample_index]%10];menu1[5]=0x40;
		switch(btnkey)
		{
			case 1:				//S7
				if(menu_index==2)
				{
					menu_index=0;
					sample_count=0;
					display_count=0;
				}
				btnkey=0;
				break;
			case 2:				//S6
				if(menu_index==2)
				{
					led_blink_flag=0;
					display_count++;
					if(display_count==10)
					{
						display_count=0;
					}
					P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
				}
				btnkey=0;
				break;
			case 3:				//S5
				if(menu_index==0)
				{
					menu_index=1;
					real_sample_time=sample_time[sample_index];
				}
				btnkey=0;
				break;
			case 4:				//S4 采集时间间隔切换
				if(menu_index==0)
				{
					sample_index++;
					if(sample_index==4)	sample_index=0;
				}
				btnkey=0;
				break;
			default:
			break;
		}
		
		if(temp_flag)
		{
			temp_flag=0;
			if(sample_count<10)
			{
				//EA=0;
				temperature[sample_count]=rd_temperature_f();
				//EA=1;
				sample_count++;
			}
			else
			{
				menu_index=2;
				led_blink_flag=1;
			}
		}
		
		if(menu_index==2)
		{
			menu3[0]=0x40;
			menu3[1]=du[display_count/10];
			menu3[2]=du[display_count%10];
			menu3[3]=0x00;menu3[4]=0x00;menu3[5]=0x40;
			menu3[6]=du[temperature[display_count]/10];
			menu3[7]=du[temperature[display_count]%10];
		}
		
	}
}

void time0() interrupt 1
{
	static int smg_count=0,key_count=0,i=0,blink_count=0,sample_count=0;
	smg_count++; key_count++;blink_count++;
	
	if(blink_count==1000)
	{
		blink_count=0;
		blink_flag=~blink_flag;
		if(blink_flag)
		{
			menu2[2]=0x00;menu2[5]=0x00;
		}
		else
		{
			menu2[2]=0x40;menu2[5]=0x40;
		}
		
		if(led_blink_flag)
		{
			if(blink_flag)
			{
				P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;
			}
			else
			{
				P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
			}
		}
		
	}

	if(key_count==10)
	{
		btnkey=scanbtn();
		key_count=0;
	}
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(menu_index==0)
		{
			P2=(P2&0x1f)|0xe0;P0=~menu1[i];P2&=0x1f;
		}
		if(menu_index==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~menu2[i];P2&=0x1f;
		}
		if(menu_index==2)
		{
			P2=(P2&0x1f)|0xe0;P0=~menu3[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
	if(menu_index==1)
	{
		sample_count++;
		if(sample_count==real_sample_time*1000)	//s->ms
		{
			sample_count=0;
			temp_flag=1;
		}
	}
	
}