#include <stc15f2k60s2.h>
#include "ds1302.h"
#include "iic.h"
#define u8 unsigned char
#define u16 unsigned int
sbit relay=P0^4;
sbit buzzer=P0^6;
	
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
	P0 =  0X00;   //预送数据我
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

u8 time_display[8];
u8 set_display[8]={0x40,0x40,0x00,0x00,0x00,0x00};
u8 shi,fen,miao;
u8 humidity=99;
u8 mode;	//mode=0 自动  mode=1 手动
u8 humidity_th=50;
bit buzzer_flag,relay_flag;
bit buzzer_beep=1;	//让蜂鸣器响或不响
bit set_flag=0;
void main()
{
	All_Init();
	humidity_th=read_24c02(0x01);
	set_sfm(8,30,50);
	Timer0Init();
	while(1)
	{
		EA=0;
		miao=Read_Ds1302_Byte(0x81);
		fen=Read_Ds1302_Byte(0x83);
		shi=Read_Ds1302_Byte(0x85);
		EA=1;
		time_display[0]=du[shi/16];
		time_display[1]=du[shi%16];
		time_display[2]=0x40;
		time_display[3]=du[fen/16];
		time_display[4]=du[fen%16];
		time_display[5]=0x00;
		
		EA=0;
		humidity=(u8)(read_adc(0x03)/2.57f);
		EA=1;
		time_display[6]=du[humidity/10];
		time_display[7]=du[humidity%10];
		
		set_display[6]=du[humidity_th/10];
		set_display[7]=du[humidity_th%10];
		
		if(mode==1)		//手动
		{
			P0=~0x02;P2=(P2&0x1f)|0x80;P0=~0x02;P2&=0x1f;
			if(humidity<humidity_th)
			{
				if(buzzer_beep)
				{
					buzzer_flag=1;
				}
				else
				{
					buzzer_flag=0;
				}
			}
			else
			{
				buzzer_flag=0;
			}
		}
		else			//自动
		{
			P0=~0x01;P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;
			buzzer_flag=0;
			if(humidity<humidity_th)
			{
				relay_flag=1;
			}
			else
			{
				relay_flag=0;
			}
		}
		
		switch(btnkey)
		{
			case 1:				//S7
				mode++;
				if(mode==2)
				{
					mode=0;
				}
				btnkey=0;
				break;
			case 2:				//S6
				if(mode==1)
				{
					buzzer_beep=~buzzer_beep;
				}
				else
				{
					set_flag=~set_flag;
					if(set_flag==0)
					{
						write_24c02(0x01,humidity_th);
					}
				}
				
				btnkey=0;
				break;
			case 3:				//S5
				if(mode==1)
				{
					relay_flag=1;
				}
				else
				{
					if(set_flag)
					{
						humidity_th++;
					}
				}
				btnkey=0;
				break;
			case 4:				//S4
				if(mode==1)
				{
					relay_flag=0;
				}
				else
				{
					if(set_flag)
					{
						humidity_th--;
					}
				}
				btnkey=0;
				break;
			default:
			break;
		}
		if(relay_flag==0 && buzzer_flag==0)
		{
			relay=0;buzzer=0;P2=(P2&0x1f)|0xa0;relay=0;buzzer=0;P2&=0x1f;
		}
		if(relay_flag==1 && buzzer_flag==0)
		{
			relay=1;buzzer=0;P2=(P2&0x1f)|0xa0;relay=1;buzzer=0;P2&=0x1f;
		}
		if(relay_flag==0 && buzzer_flag==1)
		{
			relay=0;buzzer=1;P2=(P2&0x1f)|0xa0;relay=0;buzzer=1;P2&=0x1f;
		}
		if(relay_flag==1 && buzzer_flag==1)
		{
			relay=1;buzzer=1;P2=(P2&0x1f)|0xa0;relay=1;buzzer=1;P2&=0x1f;
		}
	}
}

void time0() interrupt 1
{
	static int smg_count=0,key_count=0,i=0;
	smg_count++; key_count++;
	if(key_count==8)
	{
		btnkey=scanbtn();
		key_count=0;
	}
	
	if(smg_count==1)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(set_flag)
		{
			P2=(P2&0x1f)|0xe0;P0=~set_display[i];P2&=0x1f;
		}
		else
		{
			P2=(P2&0x1f)|0xe0;P0=~time_display[i];P2&=0x1f;
		}
		
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;
		i++;
		if(i==8) i=0;
	}
}