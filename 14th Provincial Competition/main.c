#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"
#include "onewire.h"
#include "smg.h"

sbit r3=P3^2;sbit r4=P3^3;
sbit c1=P4^4;sbit c2=P4^2;
unsigned int count_f;
unsigned char led_stat=0xff, count_t, dp_mode=1, memo_dp_mode, back_mode=1, max_temp, max_wet, now_temp, now_wet, before_temp, before_wet, times=0, hour, min, sec, last_hour, last_min, set_temp=30;
float avr_temp, avr_wet;

void init_timer()
{
	TMOD=0x06;
	AUXR&=0x3f;
	EA=1;
	ET0=1;
	ET1=1;
	TH0=255;
	TL0=255;
	TH1=(65535-50000)/256;
	TL1=(65535-50000)%256;
	TR0=0;
	TR1=1;
}
void get_time()
{
	hour=hour_read();
	min=min_read();
	sec=sec_read();
}
void service_t0() interrupt 1
{
	++count_f;
}
void service_t1() interrupt 3
{
	++count_t;
	if(count_t%10==0&&dp_mode==1)
	{
		get_time();
	}
	if(count_t%5==0&&dp_mode!=4)
	{
		if(AD_read()<20)
		{
			count_f=0;
			TR0=1;
			count_t=0;		
			readtemp();
			memo_dp_mode=dp_mode;	
			dp_mode=4;
			last_hour=hour_read();
			last_min=min_read();			
		}
	}
	if(count_t==20&&dp_mode==4)
	{
		TR0=0;
		before_temp=now_temp;	
		now_temp=readtemp();		
		if(now_temp>max_temp)
			max_temp=now_temp;
		avr_temp=1.0*(now_temp+avr_temp*times)/(times+1);	
		if(count_f<200||count_f>2000)
			now_wet=0;
		else
		{	
			before_wet=now_wet;
			now_wet=0.044*count_f+1.2;
			if(now_wet>max_wet)
				max_wet=now_wet;
			avr_wet=1.0*(now_wet+avr_wet*times)/(times+1);
		}	
	}
	if(count_t==60&&dp_mode==4)
	{
		++times;
		dp_mode=memo_dp_mode;
	}
	if(count_t==0xff)
		count_t=0;
}
void led()
{
	if(dp_mode==1)
		led_stat=led_stat&0xfe;
	else
		led_stat=(led_stat&0xfe)|0x01;
	if(dp_mode==2)
		led_stat=led_stat&0xfd;
	else
		led_stat=(led_stat&0xfd)|0x02;
	if(dp_mode==4)
		led_stat=led_stat&0xfb;
	else
		led_stat=(led_stat&0xfb)|0x04;
	if(now_temp>set_temp)
	{
		if(count_t%2==0)
			led_stat=(led_stat|0x08)&((~led_stat)|0xf7);
	}
	else
		led_stat=(led_stat&0xf7)|0x08;
	if(now_wet==1)
		led_stat=led_stat&0xef;
	else
		led_stat=(led_stat&0xef)|0x10;
	if(times>=2&&before_temp<now_temp&&before_wet<now_wet)
		led_stat=led_stat&0xdf;
	else
		led_stat=(led_stat&0xdf)|0x20;
	latch(4,led_stat);
}
void display()
{
	unsigned int temp;
	led();
	switch(dp_mode)
	{
		case 1:
			smg(1,hour/10);
			smg(2,hour%10);
			smg(3,16);
			smg(4,min/10);
			smg(5,min%10);
			smg(6,16);
			smg(7,sec/10);
			smg(8,sec%10);
			break;
		case 2:
			if(back_mode==1)
			{
				smg(1,11);
				if(times)
				{
					temp=avr_temp*10;			
					smg(3,max_temp/10);
					smg(4,max_temp%10);
					smg(5,16);
					smg(6,temp/100);
					smg_p(7,(temp/10)%10);
					smg(8,temp%10);
				}
			}
			else
				if(back_mode==2)
				{
					smg(1,15);
					if(times)
					{
						temp=avr_wet*10;			
						smg(3,max_wet/10);
						smg(4,max_wet%10);
						smg(5,16);
						smg(6,temp/100);
						smg_p(7,(temp/10)%10);
						smg(8,temp%10);
					}
				}
			else
				if(back_mode==3)
				{
					smg(1,13);
					smg(2,times/10);
					smg(3,times%10);
					if(times)
					{
						smg(4,last_hour/10);
						smg(5,last_hour%10);
						smg(6,16);
						smg(7,last_min/10);
						smg(8,last_min%10);
					}
				}
				break;
		case 3:
			smg(1,14);
			smg(7,set_temp/10);
			smg(8,set_temp%10);
			break;
		case 4:
			smg(1,12);
			smg(4,now_temp/10);
			smg(5,now_temp%10);
			smg(6,16);
			if(now_wet==0)
			{
				smg(7,10);
				smg(8,10);
			}
			else
			{
				smg(7,now_wet/10);
				smg(8,now_wet%10);
			}
			break;			
	}
	latch(7,0xff);
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
	r3=c1=c2=1;
	if(c1==0)
	{
		Delay10ms();
		if(c1==0)
		{
			if(dp_mode<3) 
				++dp_mode;
			else
				dp_mode=1;
			if(dp_mode==2)
				back_mode=1;
			while(c1==0)
				display();
		}
	}
	r3=0;
	r4=c1=c2=1;
	if(c1==0&&dp_mode==2)
	{
		Delay10ms();
		if(c1==0)
		{
			if(back_mode<3) 
				++back_mode;
			else
				back_mode=1;
			while(c1==0)
				display();
		}
	}
	r4=0;
	r3=c1=c2=1;
	if(c2==0&&dp_mode==3)
	{
		Delay10ms();
		if(c2==0)
		{
			++set_temp;
			while(c2==0)
				display();
		}
	}
	r3=0;
	r4=c1=c2=1;
	if(c2==0&&dp_mode==3)
	{
		Delay10ms();
		if(c2==0)
		{
			--set_temp;
			while(c2==0)
				display();
		}
	}
	if(c2==0&&dp_mode==2&&back_mode==3)
	{
			Delay10ms();
			if(c2==0)
			{
				count_t=0;			
				while(c2==0)
				{
					display();
					if(count_t>40)
					{
						now_temp=now_wet=before_temp=before_wet=max_temp=max_wet=times=avr_temp=avr_wet=0;
					}
				}
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
	init_ds1302();
	init_timer();
	while(1)
	{
		if(dp_mode!=4)
			scan();
		display();
	}
}