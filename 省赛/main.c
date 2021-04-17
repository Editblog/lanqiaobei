#include <stc15f2k60s2.h>
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

u8 smg_test[8];
void main()
{
	All_Init();
	Timer0Init();
	while(1)
	{
		switch(btnkey)
		{
			case 1:				//S7
				smg_test[0]=du[0];
				btnkey=0;
				break;
			case 2:				//S6
				smg_test[0]=du[1];
				btnkey=0;
				break;
			case 3:				//S5
				smg_test[0]=du[2];
				btnkey=0;
				break;
			case 4:				//S4
				smg_test[0]=du[3];
				btnkey=0;
				break;
			default:
			break;
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
	
	if(smg_count==2)
	{
		smg_count=0;
		P2=(P2&0x1f)|0xc0;P0=0x00;P2&=0x1f;
		P2=(P2&0x1f)|0xe0;P0=~du[i];P2&=0x1f;
		P2=(P2&0x1f)|0xc0;P0=wei[i];P2&=0x1f;
		i++;
		if(i==8) i=0;
	}
}