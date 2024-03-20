#ifndef _DS1302_H_
#define _DS1302_H_

void init_ds1302();
unsigned char read_ds1302_sec();
unsigned char read_ds1302_min();
unsigned char read_ds1302_hour();
void Write_Ds1302_Byte( unsigned char address,unsigned char dat)

#endif