#include <stc15f2k60s2.h>
#include "ds1302.h"
#include "onewire.h"
#define u8 unsigned char
#define u16 unsigned int
	
u8 code du[]={ 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
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
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

#define SetKeyBoard(x) P4 = (x>>3) | (x>>4);P3 = x 
#define GetKeyBoard() ((P4&0x10)<<3) | ((P4&0x04)<<4) | (P3&0x3F)
unsigned char Trg; 
unsigned char Cont; 
void KeyRead( void ) 
{ 
    unsigned char ReadData; 
    SetKeyBoard(0x0f);
    ReadData=GetKeyBoard(); 
	SetKeyBoard(0xf0);
    ReadData=(ReadData | GetKeyBoard())^0xff; 
    Trg  = ReadData & (ReadData ^ Cont); 
    Cont = ReadData; 
} 

u8 shi,fen,miao;
u8 smg_time[8];

u8 temp;
u8 smg_temperature[8];
u8 display_mode=1;	//1时钟显示   2 设置时间界面	3 设置闹钟界面	  4温度显示界面
u8 smg_set_time[8];	//用于设置时间
u8 set_time_index=0;
u8 save_shi,save_fen,save_miao;

bit blink_flag=0;
bit key_flag=0;
bit led_blink_flag=0;

u8 alarm_shi=0,alarm_fen=0,alarm_miao=0;
u8 led_blink_time=0;
u16 led_blink_count=0;

u8 smg_set_alarm[8]={0x3F,0x3F,0x40,0x3F,0x3F,0x40,0x3F,0x3F};
void KeyProc(void) 
{ 
    if (Trg)  // 如果有按下 
    {
		led_blink_flag=0;
		led_blink_time=0;
		led_blink_count=0;
		P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
	}
	if(Trg==0x81)//S7
	{
		if(display_mode==1)
		{
			display_mode=2;	//进入时间设置界面
		}
		if(display_mode==2)
		{
			if(set_time_index==0)
			{
				save_shi=(shi/16)*10+shi%16;
				save_fen=(fen/16)*10+fen%16;
				save_miao=(miao/16)*10+miao%16;
				smg_set_time[0]=du[save_shi/10];smg_set_time[1]=du[save_shi%10];smg_set_time[2]=0x40;
				smg_set_time[3]=du[save_fen/10];smg_set_time[4]=du[save_fen%10];smg_set_time[5]=0x40;
				smg_set_time[6]=du[save_miao/10];smg_set_time[7]=du[save_miao%10];
			}
			set_time_index++;
			if(set_time_index==4)	//按到第四次返回时间显示界面，设置时间
			{
				set_time_index=0;
				display_mode=1;
			}
			set_srf(save_shi,save_fen,save_miao);
		}
	}
	if(Trg==0x82)//S6
	{
		if(display_mode==1)	
		{
			display_mode=3;	//设置闹钟
		}
		if(display_mode==3)
		{
			set_time_index++;
			if(set_time_index==4)	//按到第四次返回时间显示界面，设置闹钟
			{
				set_time_index=0;
				display_mode=1;
			}
		}
	}
	if(Trg==0x84)//S5
	{
		if(display_mode==2)		//设置时间界面
		{
			if(set_time_index==1)
			{
				if(save_shi<23)
				{
					save_shi++;
				}
			}
			if(set_time_index==2)
			{
				if(save_fen<59)
				{
					save_fen++;
				}
			}
			if(set_time_index==3)
			{
				if(save_miao<59)
				{
					save_miao++;
				}
			}
		}
		
		if(display_mode==3)		//设置闹钟界面
		{
			if(set_time_index==1)
			{
				if(alarm_shi<23)
				{
					alarm_shi++;
				}
			}
			if(set_time_index==2)
			{
				if(alarm_fen<59)
				{
					alarm_fen++;
				}
			}
			if(set_time_index==3)
			{
				if(alarm_miao<59)
				{
					alarm_miao++;
				}
			}
		}
	}
	if(Trg==0x88)//S4
	{
		if(display_mode==1)	//时间显示界面
		{
			display_mode=4;
		}
		
		if(display_mode==2)		//设置时间界面
		{
			if(set_time_index==1)
			{
				if(save_shi>0)
				{
					save_shi--;
				}
			}
			if(set_time_index==2)
			{
				if(save_fen>0)
				{
					save_fen--;
				}
			}
			if(set_time_index==3)
			{
				if(save_miao>0)
				{
					save_miao--;
				}
			}
		}
		
		if(display_mode==3)		//设置闹钟界面
		{
			if(set_time_index==1)
			{
				if(alarm_shi>0)
				{
					alarm_shi--;
				}
			}
			if(set_time_index==2)
			{
				if(alarm_fen>0)
				{
					alarm_fen--;
				}
			}
			if(set_time_index==3)
			{
				if(alarm_miao>0)
				{
					alarm_miao--;
				}
			}
		}
	}
	
    if (Cont)  // 如果按键被按着不放 
    { 
		
    }
	
    if (Trg ==0 & Cont==0)  //按键放开  
    { 
       if(display_mode==4)	//如果是温度界面
		{
			display_mode=1; //切换时间界面
		}
    } 
} 

void main()
{
	SysInit();
	set_srf(23,59,50);
	Timer0Init();
	smg_set_alarm[0]=du[alarm_shi/10];smg_set_alarm[0]=du[alarm_shi%10];
	smg_set_alarm[3]=du[alarm_fen/10];smg_set_alarm[4]=du[alarm_fen%10];
	smg_set_alarm[6]=du[alarm_miao/10];smg_set_alarm[7]=du[alarm_miao%10];
	while(1)
	{
		EA=0;
		miao=Read_Ds1302_Byte(0x81);
		fen=Read_Ds1302_Byte(0x83);
		shi=Read_Ds1302_Byte(0x85);
		EA=1;
		if(display_mode==1)
		{
			smg_time[0]=du[shi/16];smg_time[1]=du[shi%16];smg_time[2]=0x40;
			smg_time[3]=du[fen/16];smg_time[4]=du[fen%16];smg_time[5]=0x40;
			smg_time[6]=du[miao/16];smg_time[7]=du[miao%16];
		}
		if(display_mode==2)	//时钟设置界面
		{
			if(blink_flag)
			{
				if(set_time_index==1)	//如果第一次·按下S7
				{
					smg_set_time[0]=du[save_shi/10];smg_set_time[1]=du[save_shi%10];
				}
				if(set_time_index==2)	//如果第二次·按下S7
				{
					smg_set_time[3]=du[save_fen/10];smg_set_time[4]=du[save_fen%10];
				}
				if(set_time_index==3)	//如果第三次·按下S7
				{
					smg_set_time[6]=du[save_miao/10];smg_set_time[7]=du[save_miao%10];
				}
			}
			else
			{
				if(set_time_index==1)	//如果第一次·按下S7
				{
					smg_set_time[0]=0x00;smg_set_time[1]=0x00;
				}
				else
				{
					smg_set_time[0]=du[save_shi/10];smg_set_time[1]=du[save_shi%10];
				}
				if(set_time_index==2)	//如果第二次·按下S7
				{
					smg_set_time[3]=0x00;smg_set_time[4]=0x00;
				}
				else
				{
					smg_set_time[3]=du[save_fen/10];smg_set_time[4]=du[save_fen%10];
				}
				if(set_time_index==3)	//如果第三次·按下S7
				{
					smg_set_time[6]=0x00;smg_set_time[7]=0x00;
				}
				else
				{
					smg_set_time[6]=du[save_miao/10];smg_set_time[7]=du[save_miao%10];
				}
			}
		}
		
		if(display_mode==3)	//闹钟设置界面
		{
			if(blink_flag)
			{
				if(set_time_index==1)	//如果第一次·按下S7
				{
					smg_set_alarm[0]=du[alarm_shi/10];smg_set_alarm[1]=du[alarm_shi%10];
				}
				if(set_time_index==2)	//如果第二次·按下S7
				{
					smg_set_alarm[3]=du[alarm_fen/10];smg_set_alarm[4]=du[alarm_fen%10];
				}
				if(set_time_index==3)	//如果第三次·按下S7
				{
					smg_set_alarm[6]=du[alarm_miao/10];smg_set_alarm[7]=du[alarm_miao%10];
				}
			}
			else
			{
				if(set_time_index==1)	//如果第一次·按下S7
				{
					smg_set_alarm[0]=0x00;smg_set_alarm[1]=0x00;
				}
				else
				{
					smg_set_alarm[0]=du[alarm_shi/10];smg_set_alarm[1]=du[alarm_shi%10];
				}
				if(set_time_index==2)	//如果第二次·按下S7
				{
					smg_set_alarm[3]=0x00;smg_set_alarm[4]=0x00;
				}
				else
				{
					smg_set_alarm[3]=du[alarm_fen/10];smg_set_alarm[4]=du[alarm_fen%10];
				}
				if(set_time_index==3)	//如果第三次·按下S7
				{
					smg_set_alarm[6]=0x00;smg_set_alarm[7]=0x00;
				}
				else
				{
					smg_set_alarm[6]=du[alarm_miao/10];smg_set_alarm[7]=du[alarm_miao%10];
				}
			}
		}
		
		
		temp=(u8)rd_temperature_f();
		smg_temperature[5]=du[temp/10];smg_temperature[6]=du[temp%10];smg_temperature[7]=0x39;
		
		if(key_flag)
		{
			key_flag=0;
			KeyRead();
			KeyProc();
		}
		
		if((shi==alarm_shi)&&(fen==alarm_fen)&&(miao==alarm_miao))
		{
			if(display_mode==1)
			{
				led_blink_flag=1;
			}
		}
	}
}


void time0() interrupt 1
{
	static int smg_count=0,key_count=0,i=0,blink_count=0;
	smg_count++;key_count++;blink_count++;
	if(key_count==10)
	{
		key_count=0;
		key_flag=1;
	}
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(display_mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_time[i];P2&=0x1f;
		}
		if(display_mode==2)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_set_time[i];P2&=0x1f;
		}
		if(display_mode==3)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_set_alarm[i];P2&=0x1f;
		}
		if(display_mode==4)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_temperature[i];P2&=0x1f;
		}
		
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8)  i=0;
	}
	
	if(blink_count==1000)
	{
		blink_count=0;
		blink_flag=!blink_flag;
	}
	
	if(led_blink_flag)
	{
		led_blink_count++;
		if(led_blink_count<200)
		{
			P2=(P2&0x1f)|0x80;P0=~0x01;P2&=0x1f;
		}
		else
		{
			if(led_blink_count<400)
			{
				P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
			}
			else
			{
				led_blink_time++;
				led_blink_count=0;
				if(led_blink_time==12)
				{
					led_blink_time=0;
					led_blink_flag=0;
				}
			}
		}
		
	}
	
}