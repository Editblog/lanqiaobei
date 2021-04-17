#include <stc15f2k60s2.h>
#include "ds1302.h"
#include "onewire.h"
#include "iic.h"
#include "led.h"
#define u8 unsigned char
#define u16 unsigned int 
	
sbit relay=P0^4;
sbit buzzer=P0^6;

u8 code du[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 code wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x20;		//设置定时初值
	TH0 = 0xD1;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}


#define KEY P3
#define NO_KEY		0xff	
#define KEY_STATE0  0   	
#define KEY_STATE1  1  		
#define KEY_STATE2  2 
u8 key_val;
u8 Key_Scan()
{
	static unsigned char key_state=KEY_STATE0; 
	u8 key_value=0,key_temp;				   
	u8 key1,key2;

	P30=0;P31=0;P32=0;P33=0;P34=1;P35=1;P42=1;P44=1;
	if(P44==0)	key1=0x70;
	if(P42==0)	key1=0xb0;
	if(P35==0)	key1=0xd0;
	if(P34==0)	key1=0xe0;
	if((P34==1)&&(P35==1)&&(P42==1)&&(P44==1))	key1=0xf0;

	P30=1;P31=1;P32=1;P33=1;P34=0;P35=0;P42=0;P44=0;
	if(P30==0)	key2=0x0e;
	if(P31==0)	key2=0x0d;
	if(P32==0)	key2=0x0b;
	if(P33==0)	key2=0x07;
	if((P30==1)&&(P31==1)&&(P32==1)&&(P33==1))	key2=0x0f;
	key_temp=key1|key2;

	switch(key_state)                             
	{
		case KEY_STATE0:				
			if(key_temp!=NO_KEY)
			{
				key_state=KEY_STATE1; 	   
			}
			break;

		case KEY_STATE1:				
			if(key_temp==NO_KEY)		
			{
				key_state=KEY_STATE0;	
			}
			else						
			{
				switch(key_temp)                             
				{
					case 0x77: key_value=4;break;
					case 0x7b: key_value=5;break;
					case 0x7d: key_value=6;break;
					case 0x7e: key_value=7;break;

					case 0xb7: key_value=8;break;
					case 0xbb: key_value=9;break;
					case 0xbd: key_value=10;break;
					case 0xbe: key_value=11;break;

					case 0xd7: key_value=12;break;
					case 0xdb: key_value=13;break;
					case 0xdd: key_value=14;break;
					case 0xde: key_value=15;break;

					case 0xe7: key_value=16;break;
					case 0xeb: key_value=17;break;
					case 0xed: key_value=18;break;
					case 0xee: key_value=19;break;	
				}
				key_state=KEY_STATE2;
			}
			break;

		case KEY_STATE2:			
			if(key_temp==NO_KEY)
			{
				key_state=KEY_STATE0;
			}
			break;
	}
	return key_value;
}



void SysInit()
{
	P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
	
	P2=(P2&0x1f)|0xa0;relay=0;buzzer=0;P2&=0x1f;
	
	P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
	
	P2=(P2&0x1f)|0xe0;P0=0xff;P2&=0x1f;
}

u8 smg_time[8];
u8 shi,fen,miao;
u8 smg_temp[8]={0x39,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 smg_adc[8]={0x79,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u16 temp=80;
u16 adc_val;
bit adc_flag;
bit adc_01=0;

u8 set_mode[8]={0x73,0x06,0x00,0x00,0x00,0x00,0x00,0x00};	//设置模式

u8 mode=0;	//切换界面
u8 shuju_mode=0;	
u8 canshu_mode=0;

char set_temp=25;
char set_led=4;
char set_time=17;

char set_buf_temp=25;
char set_buf_led=4;
char set_buf_time=17;
u8 test;

bit led_on_flag=0;
bit led_off_flag=0;

u16 led_count1=0,led_count0=0;	//判断亮暗led
void main()
{
	Timer0Init();
	SysInit();
	set_sfm(16,59,50);
	while(1)
	{
		EA=0;
		miao=Read_Ds1302_Byte(0x81);
		fen=Read_Ds1302_Byte(0x83);
		shi=Read_Ds1302_Byte(0x85);
		EA=1;
		
		smg_time[0]=du[shi/16];
		smg_time[1]=du[shi%16];
		smg_time[2]=0x40;
		smg_time[3]=du[fen/16];
		smg_time[4]=du[fen%16];
		smg_time[5]=0x40;
		smg_time[6]=du[miao/16];
		smg_time[7]=du[miao%16];
		
		//test=(shi/16)*10+(shi%16);//十进制时间
		
		if(adc_flag)
		{
			adc_flag=0;
			temp=rd_temperature_f();
			smg_temp[5]=du[temp/100];
			smg_temp[6]=du[temp/10%10]|0x80;
			smg_temp[7]=du[temp%10];
			
			adc_val=(read_adc(0x01)/255.0f*500);
			smg_adc[2]=du[adc_val/100]|0x80;
			smg_adc[3]=du[adc_val/10%10];
			smg_adc[4]=du[adc_val%10];
			
			if(adc_val > 80)
			{
				adc_01=1;
			}
			else
			{
				adc_01=0;
			}
			smg_adc[7]=du[adc_01];
		}
			
		switch(key_val)
		{
			case 4:	
				mode++;
				if(mode==2)
				{
					mode=0;
					set_time=set_buf_time;
					set_temp=set_buf_temp;
					set_led=set_buf_led;
					shuju_mode=0;
				}
				if(mode==1)		//参数界面
				{
					canshu_mode=0;
					set_mode[6]=du[set_time/10];
					set_mode[7]=du[set_time/10];
				}
				key_val=0;
				break;
			case 5:
				if(mode==1)		//参数界面
				{
					canshu_mode++;
					if(canshu_mode==3)
					{
						canshu_mode=0;
					}
					if(canshu_mode==0)
					{
						set_mode[1]=du[1];
						set_mode[6]=du[set_time/10];
						set_mode[7]=du[set_time%10];
					}
					if(canshu_mode==1)
					{
						set_mode[1]=du[2];
						set_mode[6]=du[set_temp/10];
						set_mode[7]=du[set_temp%10];
					}
					if(canshu_mode==2)
					{
						set_mode[1]=du[3];
						set_mode[6]=0x00;
						set_mode[7]=du[set_led];
					}
				}
				else			//数据界面
				{
					shuju_mode++;
					if(shuju_mode==3)
					{
						shuju_mode=0;
					}
				}
				key_val=0;
				break;
			case 8:			//减
				if(mode==1)
				{
					if(canshu_mode==0)
					{
						set_buf_time--;
						if(set_buf_time<0)
						{
							set_buf_time=0;
						}
						set_mode[6]=du[set_buf_time/10];
						set_mode[7]=du[set_buf_time%10];
					}
					if(canshu_mode==1)
					{
						set_buf_temp--;
						if(set_buf_temp<0)
						{
							set_buf_temp=0;
						}
						set_mode[6]=du[set_buf_temp/10];
						set_mode[7]=du[set_buf_temp%10];
					}
					if(canshu_mode==2)
					{
						set_buf_led--;
						if(set_buf_led<4)
						{
							set_buf_led=4;
						}
						set_mode[7]=du[set_buf_led];
					}
					
				}
				key_val=0;
				break;
			case 9:			//加
				if(mode==1)
				{
					if(canshu_mode==0)
					{
						set_buf_time++;
						if(set_buf_time>23)
						{
							set_buf_time=23;
						}
						set_mode[6]=du[set_buf_time/10];
						set_mode[7]=du[set_buf_time%10];
					}
					if(canshu_mode==1)
					{
						set_buf_temp++;
						if(set_temp>99)
						{
							set_buf_temp=99;
						}
						set_mode[6]=du[set_buf_temp/10];
						set_mode[7]=du[set_buf_temp%10];
					}
					if(canshu_mode==2)
					{
						set_buf_led++;
						if(set_buf_led>8)
						{
							set_buf_led=8;
						}
						set_mode[7]=du[set_buf_led];
					}
					
				}
				key_val=0;
				break;
			default:
			break;
		}
		
		/*刷新模式时间初值*/
		if(canshu_mode==0 && mode==1)
		{
			set_mode[1]=du[1];
			set_mode[6]=du[set_buf_time/10];
			set_mode[7]=du[set_buf_time%10];
		}
		
		/*判断小时参数*/
		test=(shi/16)*10+(shi%16);	//当前时间
		if(set_time>8)
		{
			if(test>=set_time || test<8)
			{
				led_on(1);
			}
			else
			{
				led_off(1);
			}
		}
		if(set_time<8)
		{
			if(test>=set_time && test<8)
			{
				led_on(1);
			}
			else
			{
				led_off(1);
			}
		}
		
		/*判断温度参数*/
		if(temp/10 < set_temp)
		{
			led_on(2);
		}
		else
		{
			led_off(2);
		}
		
		EA=0;		//防止LED闪烁
		/*判断LED*/
		if(set_led==4) led_on(4); else led_off(4);
		if(set_led==5) led_on(5); else led_off(5);
		if(set_led==6) led_on(6); else led_off(6);
		if(set_led==7) led_on(7); else led_off(7);
		if(set_led==8) led_on(8); else led_off(8);
		
		/*判断暗亮状态*/
		if(adc_01==1)
		{
			if(led_count1==3000 && led_count0==0)
			{
				led_on(3);
			}
			else
			{
				led_off(3);
			}
			
		}
		else
		{
			if(led_count0==3000 && led_count1==0)
			{
				led_off(3);
			}
			else
			{
				led_on(3);
			}
		}
		EA=1;
	}
}

void time0() interrupt 1
{
	static unsigned int smg_count=0,key_count=0,i=0,adc_count=0;
	smg_count++;key_count++;adc_count++;
	if(key_count==10)
	{
		key_val=Key_Scan();
		key_count=0;
	}
	
	if(adc_count==300)
	{
		adc_count=0;
		adc_flag=1;
	}
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(mode==0)		//数据界面
		{
			if(shuju_mode==0)
			{
				P2=(P2&0x1f)|0xe0;P0=~smg_time[i];P2&=0x1f;
			}
			if(shuju_mode==1)
			{
				P2=(P2&0x1f)|0xe0;P0=~smg_temp[i];P2&=0x1f;
			}
			if(shuju_mode==2)
			{
				P2=(P2&0x1f)|0xe0;P0=~smg_adc[i];P2&=0x1f;
			}
		}
		else			//参数界面
		{
			P2=(P2&0x1f)|0xe0;P0=~set_mode[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
	
	if(adc_01==1)
	{
		led_count1++;
		if(led_count1>3000)	led_count1=3000;
	}else led_count1=0;
	
	if(adc_01==0)
	{
		led_count0++;
		if(led_count0>3000)	led_count0=3000;
	}else led_count0=0;
}




