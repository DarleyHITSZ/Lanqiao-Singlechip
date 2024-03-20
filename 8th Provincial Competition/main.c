#include <STC15F2K60S2.H>
#include "onewire.h"
#include "smg.h"

sbit s4=P3^3;
sbit s5=P3^2;
sbit s6=P3^1;
sbit s7=P3^0;
unsigned char cnt=0, led_cnt=0, temp, dp_mode=1, hour=23, min=59, sec=0, set_hour=0, set_min=0, set_sec=0, time_op=0, smg_on, led_on=0, led_stat=0xff;
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
void servicet0() interrupt 1
{
	++cnt;
	++led_cnt;
	if(led_cnt%4==0)
	{
		if(set_hour==hour&&set_min==min&&set_sec==sec&&led_on==0)
		{
			led_on=1;
			led_cnt=0;
		}
		if(led_on==1&&led_cnt<100)
		{
			led_stat=(led_stat|0x01)&((~led_stat)|0xfe);
			latch(4,led_stat);
		}
		if(led_on==1&&led_cnt==100)
		{
			led_on=0;
			led_stat=0xff;
			latch(4,led_stat);
		}
	}
	if(dp_mode==3&&cnt%5==0)
	{
		temp=readtemp();
	}
	if(cnt%20==0)
	{
		++sec;
		if(sec==60) 
		{
			sec=0;
			++min;
			if(min==60)
			{
				min=0;
				++hour;
				if(hour==24)
					hour=0;
			}
		}
	}
	if(time_op!=0&&cnt%20==0)
		smg_on=smg_on?0:1;
	if(cnt==240)
		cnt=0;
	if(led_cnt==100)
		led_cnt=0;
}
void display()
{
	switch(dp_mode)
	{
		case 1:
			if(time_op==1&&smg_on==0)
			{
				smg(1,12);
				smg(2,12);
			}
			else
			{
				smg(1,hour/10);
				smg(2,hour%10);
			}
			smg(3,11);
			if(time_op==2&&smg_on==0)
			{
				smg(4,12);
				smg(5,12);
			}
			else
			{
				smg(4,min/10);
				smg(5,min%10);
			}
			smg(6,11);
			if(time_op==3&&smg_on==0)
			{
				smg(7,12);
				smg(8,12);
			}
			else
			{
				smg(7,sec/10);
				smg(8,sec%10);
			}
			break;
		case 2:
			if(time_op==1&&smg_on==0)
			{
				smg(1,12);
				smg(2,12);
			}
			else
			{
				smg(1,set_hour/10);
				smg(2,set_hour%10);
			}
			smg(3,11);
			if(time_op==2&&smg_on==0)
			{
				smg(4,12);
				smg(5,12);
			}
			else
			{
				smg(4,set_min/10);
				smg(5,set_min%10);
			}
			smg(6,11);
			if(time_op==3&&smg_on==0)
			{
				smg(7,12);
				smg(8,12);
			}
			else
			{
				smg(7,set_sec/10);
				smg(8,set_sec%10);
			}
			break;
		case 3:
			smg(6,temp/10);
			smg(7,temp%10);
			smg(8,10);
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
	if(s7==0&&dp_mode==1)
	{
		Delay10ms();
		if(s7==0)
		{
			if(time_op<3) 
				++time_op;
			else
				time_op=0;
			while(s7==0)
				display();
		}
	}
	if(s6==0&&dp_mode!=3)
	{
		Delay10ms();
		if(s6==0)
		{
			if(dp_mode==1&&time_op==0)
			{
				dp_mode=2;
				time_op=1;
			}
			else
				if(time_op<3) 
					++time_op;
				else
				{
					dp_mode=1;
					time_op=0;
				}
			while(s6==0)
				display();
		}
	}
	if(s5==0&&dp_mode!=3&&time_op!=0)
	{
		Delay10ms();
		if(s5==0)
		{
			if(dp_mode==1)
			{
					switch(time_op)
					{
						case 1:if(hour<=22) ++hour;break;
						case 2:if(min<=58) ++min;break;
						case 3:if(sec<=58) ++sec;break;
					}
			}
			else
			{ 
					switch(time_op)
					{
						case 1:if(set_hour<=22) ++set_hour;break;
						case 2:if(set_min<=58) ++set_min;break;
						case 3:if(set_sec<=58) ++set_sec;break;
					}
			}
			while(s5==0)
				display();
		}
	}
	if(s4==0&&dp_mode!=3)
	{
		Delay10ms();
		if(s4==0)
		{
			if(time_op==0&&dp_mode==1)
			{
				dp_mode=3;
			}
			else
				if(time_op!=0&&dp_mode==1)
				{
					switch(time_op)
					{
						case 1:if(hour>=1) --hour;break;
						case 2:if(min>=1) --min;break;
						case 3:if(sec>=1) --sec;break;
					}
				}
				else
					if(time_op!=0&&dp_mode==2)
					{
						switch(time_op)
						{
							case 1:if(set_hour>=1) --set_hour;break;
							case 2:if(set_min>=1) --set_min;break;
							case 3:if(set_sec>=1) --set_sec;break;
						}
					}
			while(s4==0)
				display();
			dp_mode=1;
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
	init_timer0();
	while(1)
	{
		scan();
		display();
	}
}