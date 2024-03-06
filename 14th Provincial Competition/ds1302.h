#ifndef _DS1302_H_
#define _DS1302_H_

void init_ds1302();
unsigned char hour_read();
unsigned char min_read();
unsigned char sec_read();

#endif