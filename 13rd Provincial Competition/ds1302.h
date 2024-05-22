#ifndef _DS1302_H_
#define _DS1302_H_

void init_ds1302();
unsigned char readsec();
unsigned char readmin();
unsigned char readhour();

#endif