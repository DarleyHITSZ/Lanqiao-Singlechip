#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"
#include "smg.h"

float now_temp;
unsigned char count_t, max_temp=30, min_temp=20, before_max_temp, before_min_temp, op_temp=1, dp_mode=1, led_stat=0xff, fg_judge=1;

sbit s7=P3^0;
sbit s6=P3^1;
sbit s5=P3^2;
sbit s4=P3^3;
void init_timer0()
{
	TMOD=0x00;
	AUXR&=0x3f;
	EA=1;
	ET0=1;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	TR0=1;
}
void display();
void servicet0() interrupt 1
{
	++count_t;
	display();
	if(count_t%5==0)
	{
		if(now_temp>max_temp)
			DA_write(4*51);
		else
			if(now_temp<=max_temp&&now_temp>=min_temp)
				DA_write(3*51);
			else
				DA_write(2*51);
	}
	display();
	if(count_t%8==0)
	{
		now_temp=readtemp();
		display();
	}
	if(count_t==0xff)
		count_t=0;
	display();
}
void led()
{
	led_stat=0xff;
	if(now_temp>max_temp)
		led_stat&=0xfe;
	else
		if(now_temp<=max_temp&&now_temp>=min_temp)
			led_stat&=0xfd;
		else
			led_stat&=0xfb;
	if(fg_judge==0)
		led_stat&=0xf7;
	latch(4,led_stat);
}
void display()
{
	unsigned char stemp;
	led();
	switch(dp_mode)
	{
		case 1:
			stemp=now_temp;
			smg(1,10);
			if(stemp>=10)
				smg(7,stemp/10);
			smg(8,stemp%10);
			break;
		case 2:
			smg(1,11);
			if(max_temp>=10)
				smg(4,max_temp/10);
			smg(5,max_temp%10);
			if(min_temp>=10)
				smg(7,min_temp/10);
			smg(8,min_temp%10);
			break;
	}
	smg(8,12);
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
	if(s4==0)
	{
		Delay10ms();
		if(s4==0)
		{
			if(dp_mode==1)
			{
				dp_mode=2;
				before_max_temp=max_temp;
				before_min_temp=min_temp;
				op_temp=1;
			}
			else
			{
				dp_mode=1;
				if(min_temp>max_temp)
				{
					fg_judge=0;
					max_temp=before_max_temp;
					min_temp=before_min_temp;
				}
				else
					fg_judge=1;
			}
			while(s4==0)
				display();
		}
	}
	if(s5==0)
	{
		Delay10ms();
		if(s5==0)
		{
			if(op_temp==1)
				op_temp=2;
			else
				op_temp=1;
			while(s5==0)
				display();
		}		
	}
	if(s6==0&&dp_mode==2)
	{
		Delay10ms();
		if(s6==0)
		{
			if(op_temp==1&&min_temp<=98)
				++min_temp;
			else
				if(op_temp==2&&max_temp<=98)
					++max_temp;
			while(s6==0)
				display();
		}		
	}
	if(s7==0&&dp_mode==2)
	{
		Delay10ms();
		if(s7==0)
		{
			if(op_temp==1&&min_temp>=1)
				--min_temp;
			else
				if(op_temp==2&&max_temp>=1)
					--max_temp;
			while(s7==0)
				display();
		}		
	}
}
void init_system()
{
	latch(4,0xff);
	latch(5,0x00);
}
void main()
{
	init_system();
	init_timer0();
	while(1)
	{
		scan();
		display();
	}
}