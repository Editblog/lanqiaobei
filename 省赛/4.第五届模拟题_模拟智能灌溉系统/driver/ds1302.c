/*
  程序说明: DS1302驱动程序
  软件环境: Keil uVision 4.10 
  硬件环境: CT107单片机综合实训平台 8051，12MHz
  日    期: 2011-8-9
*/

#include <reg52.h>
#include <intrins.h>

sbit SCK=P1^7;		
sbit SDA=P2^3;		
sbit RST = P1^3;   // DS1302复位												

void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK=0;
		SDA=temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}

/**
  * @brief  无需更改延时
  * @param  shi,fen,miao
  * @retval None
  * @author dianshe.taobao.com
  */
void set_sfm(char shi,char fen,char miao)
{
	Write_Ds1302_Byte(0x8e,0);							//关闭写保护
	Write_Ds1302_Byte(0x80,(miao/10)*16+miao%10);		//以BCD码写入秒数据
	Write_Ds1302_Byte(0x82,(fen/10)*16+fen%10);			//以BCD码写入分数据
	Write_Ds1302_Byte(0x84,(shi/10)*16+shi%10);			//以BCD码写入时数据
	Write_Ds1302_Byte(0x8e,0x80);						//打开写保护
}
