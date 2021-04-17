#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 

unsigned char read_adc(unsigned char add);
void write_24c02(unsigned char add,unsigned char date);
unsigned char read_24c02(unsigned char date);

void IIC_Delay(unsigned char i);

#endif