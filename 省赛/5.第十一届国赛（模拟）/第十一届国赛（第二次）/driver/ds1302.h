#ifndef __DS1302_H
#define __DS1302_H

void Write_Ds1302_Byte(unsigned  char temp);
void Write_Ds1302( unsigned char address,unsigned char dat );
unsigned char Read_Ds1302_Byte ( unsigned char address );

void set_sfm(char shi,char fen,char miao);
#endif
