#include <STC15F2K60S2.H>
#include "onewire.h"
#include "smg.h"

sbit s4=P3^3; sbit s5=P3^2; sbit s6=P3^1; sbit s7=P3^0;
unsigned char cnt0=0, cnt1=0, cntd=0, set_cntd=0, temp, wk_mode=1, led_stat=0xff,last_led=0xff, pwm=0, dp_mode=1;
void init_timer()
{
	TMOD=0x02;
	AUXR&=0x3f;
	EA=1;
	ET0=1;
	ET1=1;
	TH0=255-100;
	TL0=255-100;
	TH1=(65535-50000)/256;
	TL1=(65535-50000)%256;
	TR0=1;
	TR1=1;
}
void servicet0() interrupt 1
{
	++cnt0;
	if(cnt0<pwm&&cntd>0)
	{
		P34=1;
		//led_stat&=0x7f;
	}
	else
	{
		P34=0;
		//led_stat=(led_stat&0x7f)|0x80;
	}
	/*if(led_stat!=last_led)
	{
		latch(4,led_stat);
		last_led=led_stat;
	}*/
	if(cnt0==10)
		cnt0=0;
}
void servicet1() interrupt 3
{
	++cnt1;
	if(cnt1==20&&cntd>0)
	{
		--cntd;
	}
	if(cnt1%10==0&&dp_mode==2)
	{
		temp=readtemp();
	}
	if(cnt1==20)
		cnt1=0;
}
void led()
{
	if(cntd>0)
	{
		if(wk_mode==1)
			led_stat&=0xfe;
		else
			led_stat=(led_stat&0xfe)|0x01;
		if(wk_mode==2)
			led_stat&=0xfd;
		else
			led_stat=(led_stat&0xfd)|0x02;
		if(wk_mode==3)
			led_stat&=0xfb;
		else
			led_stat=(led_stat&0xfb)|0x04;
	}
	else
		led_stat=0xff;
	if(led_stat!=last_led)
	{
		latch(4,led_stat);
		last_led=led_stat;
	}
}
void display()
{
	led();
	switch(dp_mode)
	{
		case 1:
			smg(1,11);
			smg(2,wk_mode);
			smg(3,11);
			smg(5,cntd/1000);
			smg(6,(cntd/100)%10);
			smg(7,(cntd/10)%10);
			smg(8,cntd%10);
			break;
		case 2:
			smg(1,11);
			smg(2,4);
			smg(3,11);
			smg(6,temp/10);
			smg(7,temp%10);
			smg(8,10);
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
	if(s4==0)
	{
		Delay10ms();
		if(s4==0)
		{
			if(wk_mode<3) 
				++wk_mode;
			else
				wk_mode=1;
			switch(wk_mode)
			{
				case 1:pwm=2;break;
				case 2:pwm=3;break;
				case 3:pwm=7;break;
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
			if(set_cntd<120)
				set_cntd+=60;
			else
				set_cntd=0;
			cntd=set_cntd;
			while(s5==0)
				display();
		}
	}
	if(s6==0)
	{
		Delay10ms();
		if(s6==0)
		{
			cntd=0;
			while(s6==0)
				display();
		}
	}
	if(s7==0)
	{
		Delay10ms();
		if(s7==0)
		{
			if(dp_mode==1)
				dp_mode=2;
			else
				dp_mode=1;
			while(s7==0)
				display();
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
	init_timer();
	while(1)
	{
		scan();
		display();
	}
}