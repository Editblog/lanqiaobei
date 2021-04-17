#include <stc15f2k60s2.h>
#include "onewire.h"
#define u8 unsigned char 
#define u16 unsigned int
	
u8 code du[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 code wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
sbit relay=P0^4;
sbit buzzer=P0^6;
void SysInit()
{
	P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
	P2=(P2&0x1f)|0xa0;relay=0;buzzer=0;P2&=0x1f;
	P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
	P2=(P2&0x1f)|0xe0;P0=0xff;P2&=0x1f;
}


void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xAE;		//设置定时初值
	TH0 = 0xFB;		//设置定时初值
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
#define LONG_KEY_TIME 300 //LONG_KEY_TIME*10MS = 3S
#define SINGLE_KEY_TIME 3 //SINGLE_KEY_TIME*10MS = 30MS
char btnkey;
char scanbtn(void)
{
	static char btn_state = 0;
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

u8 work_display[8];
u8 mode=1;
u16 work_time;
u8 S5_count=0;
bit output_flag=1;
u8 temp_mode=0;
u8 temp_display[8]={0x40,0x66,0x40,0x00,0x00,0x00,0x00,0x39};
u8 temperature;
void main()
{
	SysInit();
	Timer0Init();
	while(1)
	{
		work_display[0]=0x40;work_display[1]=du[mode];work_display[2]=0x40;
		work_display[3]=0x00;work_display[4]=du[0];work_display[5]=du[work_time/100];
		work_display[6]=du[work_time/10%10];work_display[7]=du[work_time%10];
		
		if(temp_mode)
		{
			temperature=rd_temperature_f();
			temp_display[5]=du[temperature/10];temp_display[6]=du[temperature%10];
		}
		
		if(work_time==0)
		{
			output_flag=0;
		}
		else
		{
			output_flag=1;
		}
		
		switch(btnkey)
		{
			case 1:		//S7
				temp_mode++;
				if(temp_mode==2)	temp_mode=0;
				btnkey=0xff;
				break;
			case 2:		//S6
				work_time=0;
				output_flag=0;
				btnkey=0xff;
				break;
			case 3:		//S5
				S5_count++;
				if(S5_count==3)	
				{	
					S5_count=0;
					work_time=0;
				}
				if(S5_count==1) work_time=60;
				if(S5_count==2) work_time=120;
				btnkey=0xff;
				break;
			case 4:		//S4
				mode++;
				if(mode==4)
				{
					mode=1;
				}
				btnkey=0xff;
				break;
		}	
		if(mode==1)
		{
			P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;
		}
		if(mode==2)
		{
			P2=(P2&0x1f)|0x80;P0=~0x02;P2&=0x1f;
		}
		if(mode==3)
		{
			P2=(P2&0x1f)|0x80;P0=~0x04;P2&=0x1f;
		}
	}
}

void time0() interrupt 1	//100us
{
	static smg_count=0,key_count=0,i=0,pwm_count=0,time_count=0;
	smg_count++; key_count++;pwm_count++;time_count++;
	
	if(key_count==100)
	{
		btnkey=scanbtn();
		key_count=0;
	}
	
	if(smg_count==20)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(temp_mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~temp_display[i];P2&=0x1f;
		}
		else
		{
			P2=(P2&0x1f)|0xe0;P0=~work_display[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
	
	if(time_count==10000)
	{
		time_count=0;
		if(work_time>0) work_time--;
	}
	
	if(output_flag)
	{
		if(mode==1)
		{
			if(pwm_count==8)
			{
				P34=1;
			}
			if(pwm_count==10)
			{
				P34=0;
				pwm_count=0;
			}
		}
		if(mode==2)
		{
			if(pwm_count==7)
			{
				P34=1;
			}
			if(pwm_count==10)
			{
				P34=0;
				pwm_count=0;
			}
		}
		if(mode==3)
		{
			if(pwm_count==3)
			{
				P34=1;
			}
			if(pwm_count==10)
			{
				P34=0;
				pwm_count=0;
			}
		}
	}
	else
	{
		P34=0;
		pwm_count=0;
		time_count=0;
	}
}

