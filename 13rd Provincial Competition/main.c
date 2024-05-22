#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "onewire.h"
#include "smg.h"

sbit r3=P3^2;sbit r4=P3^3;
sbit c3=P3^5;sbit c4=P3^4;
float now_temp;
unsigned char wk_mode=0, dp_mode=1, hour=7, min=59, sec=0, set_temp=23, cnt=0, blink=0;
unsigned char	relay_stat=0, old_relay_stat=0, led_stat=0xff, old_led_stat=0xff;
void init_timer0()
{
	AUXR&=0x3f;
	TMOD=0x00;
	EA=1;
	ET0=1;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	TR0=1;
}
void relay_check()
{
	if(wk_mode==0)
	{
		if(now_temp>set_temp)
			relay_stat=1;
		else
			relay_stat=0;
	}
	else
	{
		if(min==0&&sec==0)
			relay_stat=1;
		else
			if(min==0&&sec>=1&&sec<=5&&relay_stat==1)
				relay_stat=1;
			else
				relay_stat=0;
	}
	if(old_relay_stat!=relay_stat)
	{
		if(relay_stat==1)
			latch(5,0x10);
		else
			latch(5,0x00);
		old_relay_stat=relay_stat;
	}
}
void servicet0() interrupt 1
{
	if(cnt!=0xff) 
		++cnt;
	else
		cnt=0;
	if(cnt%2==0)
		sec=readsec();
	if(sec==0)
	{
		min=readmin();
		if(min==0)
		{
			hour=readhour();
		}
	}
	if(cnt%20==0)
	{
		now_temp=readtemp();
	}
	relay_check();
	if(relay_stat==0)
		blink=0;
	else
		if(cnt%2==0&&relay_stat==1)
			blink=!blink;
}
void led()
{
	led_stat=0xff;
	if(min==0&&sec>=0&&sec<=5)
		led_stat&=0xfe;
	if(wk_mode==0)
		led_stat&=0xfd;
	if(blink==1)
		led_stat&=0xfb;
	if(led_stat!=old_led_stat)
	{
		latch(4,led_stat);
		old_led_stat=led_stat;
	}
}
void display()
{
	unsigned int stemp;
	led();
	switch(dp_mode)
	{
		case 1:
			stemp=now_temp*10;
			smg(1,10);
			smg(2,1);
			smg(6,stemp/100);
			smg_p(7,(stemp/10)%10);
			smg(8,stemp%10);
			break;
		case 2:
			smg(1,10);
			smg(2,2);
			smg(4,hour/10);
			smg(5,hour%10);
			smg(6,11);
			smg(7,min/10);
			smg(8,min%10);
			break;
		case 3:
			smg(1,10);
			smg(2,3);
			smg(7,set_temp/10);
			smg(8,set_temp%10);
			break;
		case 4:
			smg(1,10);
			smg(2,2);
			smg(4,min/10);
			smg(5,min%10);
			smg(6,11);
			smg(7,sec/10);
			smg(8,sec%10);
			break;
	}
}
void Delay10ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 117;
	j = 184;
	do
	{
		while (--j);
	} while (--i);
}
void scan()
{
	r4=0;
	r3=c3=c4=1;
	if(c3==0)
	{
		Delay10ms();
		if(c3==0)
		{
			if(dp_mode<3) 
				++dp_mode;
			else 
				dp_mode=1;
			while(c3==0)
				display();
		}
	}
	if(c4==0&&dp_mode==3)
	{
		Delay10ms();
		if(c4==0)
		{
			if(set_temp<99)
				++set_temp;
			while(c4==0)
				display();
		}
	}
	r3=0;
	r4=c3=c4=1;
	if(c3==0)
	{
		Delay10ms();
		if(c3==0)
		{
			wk_mode=wk_mode?0:1;
			relay_stat=0;
			while(c3==0)
				display();
		}
	}
	if(c4==0&&dp_mode!=1)
	{
		Delay10ms();
		if(c4==0)
		{
			if(dp_mode==3&&set_temp>10)
				--set_temp;
			if(dp_mode==2)
				dp_mode=4;
			while(c4==0)
				display();
			if(dp_mode==4)
				dp_mode=2;
		}
	}
}
void set()
{
	latch(4,0xff);
	latch(5,0x00);
}
void main()
{
	set();
	readtemp();
	init_timer0();
	init_ds1302();
	while(1)
	{
		display();
		scan();
	}
}