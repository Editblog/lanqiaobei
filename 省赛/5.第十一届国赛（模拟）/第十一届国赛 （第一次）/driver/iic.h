#ifndef _IIC_H
#define _IIC_H

#define uchar unsigned char
#define uint unsigned int 
	
void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 

uchar read_adc(uchar add);
void write_24c02(uchar add,uchar date);
uchar read_24c02(uchar add);
#endif