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
    P2 &= 0X1F;   //��573
	P0 =  0XFF;   //Ԥ������
	P2 |= 0X80;   //����Ӧ573
	P0 =  0XFF;   //������
	P2 &= 0X1F;   //��573
	
	//������
    P2 &= 0X1F;   //��573
	P0 =  0X00;   //Ԥ������
	P2 |= 0XA0;   //����Ӧ573
	P0 =  0X00;   //������
	P2 &= 0X1F;   //��573
	
	//�����λѡ
	P2 &= 0X1F;   //��573
	P0 =  0X00;   //Ԥ������
	P2 |= 0XC0;   //����Ӧ573
	P0 =  0X00;   //������
	P2 &= 0X1F;   //��573
	
	//����ܶ�ѡ
	P2 &= 0X1F;   //��573
	P0 =  0XFF;   //Ԥ������
	P2 |= 0XE0;   //����Ӧ573
	P0 =  0XFF;   //������
	P2 &= 0X1F;   //��573
}

void Timer0Init(void)		//1����@11.0592MHz
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xCD;		//���ö�ʱ��ֵ
	TH0 = 0xD4;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
	ET0=1;
	EA=1;
}

//---------------------------�ĸ���������--------------------------------
#define btn_io P3
#define btn_state_0 0	  //�Ƿ���
#define btn_state_1 1 	 //�Ƿ��Ƕ���
#define btn_state_2 2	  //�ж�������Ч������
#define btn_state_3 3	  //�ȴ������ͷ� 
#define btn_mask 0x0f	  //���β���Ҫ��IO
#define LONG_KEY_TIME 100 //LONG_KEY_TIME*10MS = 3S
#define SINGLE_KEY_TIME 3 //SINGLE_KEY_TIME*10MS = 30MS
char btnkey;
static char btn_state = 0;
char scanbtn(void)
{
	static int key_time; //������ʱ����	
	static char key_value=0;
	char btn_press,btn_return = 0;
	btn_press = btn_io & btn_mask;
	switch(btn_state)
	{
		case btn_state_0:
			key_value=0;
			if(btn_press != btn_mask){
				key_time = 0; // ����ʱ��������
				btn_state = btn_state_1;
			}
			break;
		case btn_state_1:
			if(btn_press != btn_mask)
			{
				key_time++; // һ��10ms
				if (key_time >= SINGLE_KEY_TIME) // ����ʱ��Ϊ��SINGLE_KEY_TIME*10ms = 30ms;
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
				btn_state = btn_state_0; // ȥ״̬3���ȴ������ͷ�
			}
			else
			{
				if(btn_press == 0x0e) key_value = 1;
				if(btn_press == 0x0d) key_value = 2;
				if(btn_press == 0x0b) key_value = 3;
				if(btn_press == 0x07) key_value = 4;

				key_time++;

				if (key_time >= LONG_KEY_TIME) // �������ʱ�䳬�� �趨�ĳ���ʱ�䣨LONG_KEY_TIME*10ms=200*10ms=2000ms��, ���ж�Ϊ ����
				{
					btn_return =key_value|0x80;
					key_value=0;
					btn_state = btn_state_3; // ȥ״̬3���ȴ������ͷ�
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

u8 menu_index=0;	//Ĭ�Ͻ��� 
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
		
		//�ɼ�ʱ������ʾ
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
			case 4:				//S4 �ɼ�ʱ�����л�
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