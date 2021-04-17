#include <stc15f2k60s2.h>
#include "iic.h"
#include "led.h"
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

u8 led_mode=0;
u8 led_index=0;
bit adc_flag=0;
u8 adc_value;
u8 led_pwm=0;
u8 set_mode=4;	//0��Ϩ��ģʽ 1����תģʽ 2��ʱ��
u8 smg_led_set[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
u8 smg_led_pwm[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xbf,0xff};
u8 set_led_num=1;	//��תģʽ���� 
u16 led_interval[5]={0,400,800,1200,800};	//��תʱ��
bit smg_blink_flag;
void main()
{
	led_interval[1]=read_24c02(1)*100;
	led_interval[2]=read_24c02(2)*100;
	led_interval[3]=read_24c02(3)*100;
	led_interval[4]=read_24c02(4)*100;
	Timer0Init();
	All_Init();
	while(1)
	{
		if(set_mode==1)
		{
			if(smg_blink_flag)
			{
				smg_led_set[0]=~0x40;
				smg_led_set[1]=~du[set_led_num];
				smg_led_set[2]=~0x40;
			}
			else
			{
				smg_led_set[0]=0xff;
				smg_led_set[1]=0xff;
				smg_led_set[2]=0xff;				
			}
				
		}
		else
		{
			smg_led_set[0]=~0x40;
			smg_led_set[1]=~du[set_led_num];
			smg_led_set[2]=~0x40;
		}
		if(set_mode==2)
		{
			if(smg_blink_flag)
			{ 
				smg_led_set[4]=~du[led_interval[set_led_num]/1000];
				smg_led_set[5]=~du[led_interval[set_led_num]/100%10];
				smg_led_set[6]=~du[led_interval[set_led_num]/10%10];
				smg_led_set[7]=~du[led_interval[set_led_num]%10];
			}
			else
			{
				smg_led_set[4]=0xff;
				smg_led_set[5]=0xff;
				smg_led_set[6]=0xff;
				smg_led_set[7]=0xff;
			}
				
		}
		else
		{ 
			smg_led_set[4]=~du[led_interval[set_led_num]/1000];
			smg_led_set[5]=~du[led_interval[set_led_num]/100%10];
			smg_led_set[6]=~du[led_interval[set_led_num]/10%10];
			smg_led_set[7]=~du[led_interval[set_led_num]%10];
		}
		if(adc_flag)
		{
			adc_value=read_adc(0x03);
			adc_flag=0;
			led_pwm=adc_value/85+1;//led_pwm = 1 2 3 4
			smg_led_pwm[7]=~du[led_pwm];
		}
		
		switch(btnkey)
		{
			case 1:				//S7
				if(led_mode)
				{
					led_mode=0;
				}
				else
				{
					led_mode=1;
					led_index=0;
				}
				btnkey=0;
				break;
			case 2:				//S6
				set_mode++;
				if(set_mode==3)
				{
					set_mode=0;
					write_24c02(1,led_interval[1]/100);	//дEEPROW
					delayms(10);
					write_24c02(2,led_interval[2]/100);	//дEEPROW
					delayms(10);
					write_24c02(3,led_interval[3]/100);	//дEEPROW
					delayms(10);
					write_24c02(4,led_interval[4]/100);	//дEEPROW
					delayms(10);
				}
				btnkey=0;
				break;
			case 3:				//S5
				if(set_mode==1)
				{
					if(set_led_num<4)
					{
						set_led_num++;
					}
				}
				if(set_mode==2)
				{
					if(led_interval[set_led_num]<1200)
					{
						led_interval[set_led_num]+=100;
					}
				}
				btnkey=0;
				break;
			case 4:				//S4
				if(set_mode==1)
				{
					if(set_led_num>1)
					{
						set_led_num--;
					}
				}
				if(set_mode==2)
				{
					if(led_interval[set_led_num]>400)
					{
						led_interval[set_led_num]-=100;
					}
				}
				btnkey=0;
				break;
			default:
			break;
		}

		if(btn_state==btn_state_3 && set_mode==0)
		{
			set_mode=4;
		}
		if(btn_state==0)
		{
			if(set_mode==4)	set_mode=0;
		}
	}
}

void time0() interrupt 1
{
	static int smg_count=0,key_count=0,i=0,led_count=0,adc_count=0,led_pwm_count=0,smg_blink_count=0;
	smg_count++; key_count++;led_count++;adc_count++;led_pwm_count++;smg_blink_count++;
	if(key_count==8)
	{
		btnkey=scanbtn();
		key_count=0;
	}
	
	if(adc_count==100)
	{
		adc_count=0;
		adc_flag=1;
	}
	
	if(smg_blink_count==800)
	{
		smg_blink_count=0;
		smg_blink_flag=!smg_blink_flag;
	}
	
	if(smg_count==3)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		if(set_mode==0)
		{
			P2=(P2&0x1f)|0xe0;P0=0xff;P2&=0x1f;
		}
		if(set_mode==1 || set_mode==2)
		{
			P2=(P2&0x1f)|0xe0;P0=smg_led_set[i];P2&=0x1f;
		}
		if(set_mode==4)
		{
			P2=(P2&0x1f)|0xe0;P0=smg_led_pwm[i];P2&=0x1f;
		}
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;P0=0xff;
		i++;
		if(i==8) i=0;
	}
	
	if(led_pwm_count < led_pwm*2)
	{
		if(led_mode==1)
		{
			P2=(P2&0x1f)|0x80;P0=~(0x01<<led_index);P2&=0x1f;
			if(led_count>led_interval[1])
			{
				led_index++;
				led_count=0;
				if(led_index==8)
				{
					led_index=0;
					led_mode=2;
				}				
			}
		}
		if(led_mode==2)
		{
			P2=(P2&0x1f)|0x80;P0=~(0x80>>led_index);P2&=0x1f;
			if(led_count>led_interval[2])
			{
				led_index++;
				led_count=0;
				if(led_index==8)
				{
					led_index=0;
					led_mode=3;
				}
			}
		}
		if(led_mode==3)
		{
			P2=(P2&0x1f)|0x80;P0=~((0x01<<led_index)|(0x80>>led_index));P2&=0x1f;
			if(led_count>led_interval[3])
			{
				led_index++;
				led_count=0;
				if(led_index==4)
				{
					led_mode=4;
				}
			}
		}
		if(led_mode==4)
		{
			P2=(P2&0x1f)|0x80;P0=~((0x01<<led_index)|(0x80>>led_index));P2&=0x1f;
			if(led_count>led_interval[4])
			{
				led_index++;
				led_count=0;
				if(led_index==8)
				{
					led_index=0;
					led_mode=1;
				}
			}
		}
	}
	else if(led_pwm_count <10)
	{
		P2=(P2&0x1f)|0x80;P0=0xff;P2&=0x1f;
	}
	else
	{
		led_pwm_count=0;
	}
}