#include <stc15f2k60s2.h>
#include "onewire.h"
#include "iic.h"
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
	TMOD |= 0x04;		//设置定时器模式
	TL0 = 0xff;		//设置定时初值
	TH0 = 0xff;		//设置定时初值
	TR0 = 1;		//定时器0开始计时
	ET0=1;
}

void Timer1Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x40;		//定时器时钟1T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xCD;		//设置定时初值
	TH1 = 0xD4;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	ET1=1;
	EA=1;
}


void Delay5ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 54;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
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
bit key_flag=0;

u16 temp;
u8 smg_temp[8]={0x39};

u16 adc;
u8 adc_buf;
bit adc_flag=0;
u8 smg_adc[8]={0x3e};

u16 count=0;
u8 smg_n55[8]={0x71};

u8 mode=1;

u8 freq_low=0,freq_high=0;
u16 save_freq=0;

u8 temp_low=0,temp_high=0;
u16 save_temp=0;

u16 save_adc=0;

u8 hx_mode=0;

u8 hx_temp[8]={0x76,0x39};
u8 hx_adc[8]={0x76,0x3e};
u8 hx_n55[8]={0x76,0x71};

u8 set_adc[8]={0x73};
u8 S7_mode=0;

u8 set_adcval=1;

void KeyProc(void) 
{
	if(Trg==0x81)		//S7
	{
		mode=0;hx_mode=0;
		S7_mode++;
		if(S7_mode==2)
		{
			S7_mode=0;
			mode=1;
		}
	}
	if(Trg==0x82)		//S6
	{
		if(S7_mode==0)
		{
			mode=0;
			hx_mode++;
			if(hx_mode==4)	hx_mode=1;
		}
		if(S7_mode==1)
		{
			set_adcval++;
			if(set_adcval>50)	set_adcval=1;
		}

	}
	if(Trg==0x84)		//S5
	{
		write_24c02(0x01,temp_high);Delay5ms();
		write_24c02(0x02,temp_low);Delay5ms();
		save_temp = (temp_high<<8)|temp_low;
		
		write_24c02(0x03,adc_buf);Delay5ms();
		save_adc=adc_buf/255.0f*500;
		
		write_24c02(0x04,freq_high);Delay5ms();
		write_24c02(0x05,freq_low);
		save_freq = freq_low|(freq_high<<8);
	}
	if(Trg==0x88)		//S4
	{
		mode++;
		if(mode==4)	mode=1;
	}
	
}

void main()
{
	SysInit();
	Timer0Init();
	Timer1Init();
	
	temp_high=read_24c02(0x01);Delay5ms();
	temp_low=read_24c02(0x02);Delay5ms();
	save_temp = (temp_high<<8)|temp_low;
	
	freq_high=read_24c02(0x04);Delay5ms();
	freq_low=read_24c02(0x05);Delay5ms();
	save_freq = (freq_high<<8)|freq_low;
	
	save_adc=read_24c02(0x03)/255.0f*500;
	
	while(1)
	{
		if(adc_flag)
		{
			adc_buf=read_adc(0x03);
			adc=adc_buf/255.0f*500;
			smg_adc[6]=du[adc/100]|0x80;smg_adc[7]=du[adc/10%10];
			
			temp=(u16)rd_temperature_f();
			smg_temp[4]=du[temp/1000];smg_temp[5]=du[temp/100%10]|0x80;
			smg_temp[6]=du[temp/10%10];smg_temp[7]=du[temp%10];
			temp_low=temp;
			temp_high=temp>>8;
		}
		
		if(key_flag)
		{
			key_flag=0;
			KeyRead();
			KeyProc();
		}
		
		if(hx_mode==1)
		{
			hx_temp[4]=du[save_temp/1000];hx_temp[5]=du[save_temp/100%10]|0x80;
			hx_temp[6]=du[save_temp/10%10];hx_temp[7]=du[save_temp%10];
		}
		
		if(hx_mode==2)
		{
			hx_adc[6]=du[save_adc/100]|0x80;hx_adc[7]=du[save_adc/10%10];
		}
		
		if(hx_mode==3)
		{
			hx_n55[3]=du[save_freq/10000];
			hx_n55[4]=du[save_freq/1000%10];
			hx_n55[5]=du[save_freq/100%10];
			hx_n55[6]=du[save_freq/10%10];
			hx_n55[7]=du[save_freq%10];
		}
		
		if(S7_mode==1)
		{
			set_adc[6]=du[set_adcval/10]|0x80;set_adc[7]=du[set_adcval%10];
		}
	}
}

void time0() interrupt 1
{
	count++;
}

void time1() interrupt 3
{
	static int smg_count=0,key_count=0,i=0,adc_count=0,one_second_count=0;
	smg_count++;key_count++;adc_count++;one_second_count++;
	
	if(adc_count==150)
	{
		adc_count=0;
		adc_flag=1;
	}
	
	if(key_count==7)
	{
		key_flag=1;
		key_count=0;
	}
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_temp[i];P2&=0x1f;
		}
		if(mode==2)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_adc[i];P2&=0x1f;
		}
		if(mode==3)
		{
			P2=(P2&0x1f)|0xe0;P0=~smg_n55[i];P2&=0x1f;
		}
		
		if(hx_mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~hx_temp[i];P2&=0x1f;
		}
		if(hx_mode==2)
		{
			P2=(P2&0x1f)|0xe0;P0=~hx_adc[i];P2&=0x1f;
		}
		if(hx_mode==3)
		{
			P2=(P2&0x1f)|0xe0;P0=~hx_n55[i];P2&=0x1f;
		}
		if(S7_mode==1)
		{
			P2=(P2&0x1f)|0xe0;P0=~set_adc[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;
		i++;
		if(i==8)  i=0;
	}
	
	if(one_second_count==1000)
	{
		one_second_count=0;
		ET0=0;TR0=0;
		freq_low=count;
		freq_high=count>>8;
		smg_n55[3]=du[count/10000];
		smg_n55[4]=du[count/1000%10];
		smg_n55[5]=du[count/100%10];
		smg_n55[6]=du[count/10%10];
		smg_n55[7]=du[count%10];
		count=0;
		ET0=1;TR0=1;
	}
}
