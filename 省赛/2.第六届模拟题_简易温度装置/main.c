#include <stc15f2k60s2.h>
#include "intrins.h"
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

void Delay500ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 22;
	j = 3;
	k = 227;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


#define KEY P3
#define NO_KEY		0xff	//无按键按下
#define KEY_STATE0  0   	//判断按键按下
#define KEY_STATE1  1  		//确认按键按下
#define KEY_STATE2  2  		//释放按键
unsigned char Key_Scan()
{
	static unsigned char key_state=KEY_STATE0; //定义为静态变量，用于保存每次按键的状态
	u8 key_value=0,key_temp;				   //key_val:返回的键值；key_temp:读取的IO状态
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
		case KEY_STATE0:				//判断按键按下
			if(key_temp!=NO_KEY)
			{
				key_state=KEY_STATE1; 	//有键按下，就转到状态1    
			}
			break;

		case KEY_STATE1:				//经过10ms，再次确认按键按下，用于消抖
			if(key_temp==NO_KEY)		
			{
				key_state=KEY_STATE0;	//如果是抖动，则回到状态0
			}
			else						//如果不是抖动，则返回对应的键值
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

		case KEY_STATE2:			//经过10ms，判断是否释放按键
			if(key_temp==NO_KEY)
			{
				key_state=KEY_STATE0;
			}
			break;
	}
	return key_value;
}


u8 key_value=0;
u8 temperture;
u8 temp_display[8];
u8 set_display[8];
u8 set_temp[8];
u8 temp_num;
u8 temp_max=30;u8 temp_min=20;
u8 key_count=0;
u8 set_count=0;
sbit relay=P0^4;
sbit buzzer=P0^6;
bit error_flag;

void main()
{
	All_Init();
	Timer0Init();
	Delay500ms();
	while(1)
	{
		temperture= rd_temperature_f();
		temp_display[0]=0x40;temp_display[1]=du[temp_num];temp_display[2]=0x40;temp_display[3]=0x00;
		temp_display[4]=0x00;temp_display[5]=0x40;temp_display[6]=du[temperture/10];temp_display[7]=du[temperture%10];
		set_display[0]=0x40;set_display[5]=0x40;
		if(temperture<temp_min)
		{
			temp_num=0;
		}
		else if(temperture<=temp_max)
		{
			temp_num=1;
		}
		else
		{
			temp_num=2;
		}
		if(key_count==2)	key_count=5;
		if(key_count>7)		key_count=8;  
		switch(key_value)            
		{                                              
			case 4: 
				key_count++;
				set_display[key_count]=du[9];
				set_temp[key_count]=9;
				break;
			case 5: 
				key_count++;
				set_display[key_count]=du[6];
				set_temp[key_count]=6;
				break;
			case 6:  
				key_count++;
				set_display[key_count]=du[3];
				set_temp[key_count]=3;
				break;
			case 7:  
				key_count++;
				set_display[key_count]=du[0];
				set_temp[key_count]=0;
				break;
			case 8:  		//设置
				set_count++;
				if(set_count==2)	
				{
					set_count=0;
					temp_max=set_temp[1]*10+set_temp[2];
					temp_min=set_temp[6]*10+set_temp[7];
					if(temp_min>temp_max)
					{
						P2=(P2&0x1f)|0x80;P0=~0x02;P2&=0x1f;
						error_flag=1;
					}
					else
					{
						error_flag=0;
					}
				}
				break;
			case 9:  		
				key_count++;
				set_display[key_count]=du[7];
				set_temp[key_count]=7;
				break;
			case 10: 
				key_count++;
				set_display[key_count]=du[4];
				set_temp[key_count]=4;
				break;
			case 11: 
				key_count++;
				set_display[key_count]=du[1];
				set_temp[key_count]=1;
				break;
			case 12: 		//清除
				key_count=0;
				set_display[1]=0x00;set_display[2]=0x00;set_display[6]=0x00;set_display[7]=0x00;
				break;
			case 13: 
				key_count++;
				set_display[key_count]=du[8];
				set_temp[key_count]=8;
				break;
			case 14: 
				key_count++;
				set_display[key_count]=du[5];
				set_temp[key_count]=5;
				break;
			case 15: 
				key_count++;
				set_display[key_count]=du[2];
				set_temp[key_count]=2;
				break;
			case 16: 
				break;
			case 17: 
				break;
			case 18: 
				break;
			case 19: 
				break;
		}
		if(temp_num==0 || temp_num==1)
		{
			relay=0;buzzer=0;P2=(P2&0x1f)|0xa0;relay=0;buzzer=0;P2&=0x1f;
		}
		if(temp_num==2)
		{
			relay=1;buzzer=0;P2=(P2&0x1f)|0xa0;relay=1;buzzer=0;P2&=0x1f;
		}

	}
}

bit blink_flag;
void time0() interrupt 1
{
	static int smg_count=0,count_key=0,i=0,led_count=0;
	smg_count++; count_key++;led_count++;
	if(count_key==14)
	{
		key_value=Key_Scan();
		count_key=0;
	}
	
	if(error_flag==0)
	{
		if(temp_num==0)
		{
			if(led_count>800)
			{
				led_count=0;
				blink_flag=~blink_flag;
				if(blink_flag)	{P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;}
				else {P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;}
			}
		}
		if(temp_num==1)
		{
			if(led_count>400)
			{
				led_count=0;
				blink_flag=~blink_flag;
				if(blink_flag)	{P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;}
				else {P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;}
			}
		}
		if(temp_num==2)
		{
			if(led_count>200)
			{
				led_count=0;
				blink_flag=~blink_flag;
				if(blink_flag)	{P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;}
				else {P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;}
			}
		}
	}
	
	if(smg_count==3)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(set_count==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~set_display[i];P2&=0x1f;
		}
		else
		{
			P2=(P2&0x1f)|0xe0;P0=~temp_display[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
}