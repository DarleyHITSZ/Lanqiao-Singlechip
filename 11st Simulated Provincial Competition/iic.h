#ifndef _IIC_H
#define _IIC_H_

void AT_write(unsigned char addr,unsigned char dat);
unsigned char AT_read(unsigned char addr);

#endif