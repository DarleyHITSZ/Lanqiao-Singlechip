#include <STC15F2K60S2.H>
#include "iic.h"
#include "smg.h"

unsigned char cnt, vdac ,vrb2, dp_mode=1, out_mode=0, led_stat=0xff, ledon=1, smgon=1;
unsigned int cnf, fre;
sbit s4=P3^3;
sbit s5=P3^2;
sbit s6=P3^1;
sbit s7=P3^0;

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
	TR0=1;
	TR1=1;
}
void servicet0() interrupt 1
{
	++cnf;
}
void servicet1() interrupt 3
{
	++cnt;
	if(!(cnt%10))
	{
		vrb2=AD_read();
		if(!out_mode)
		{
			vdac=2*51;
		}
		else
		{
			vdac=vrb2;
		}
		DA_write(vdac);
	}
	if(!(cnt%20))
	{
		fre=cnf;
		cnf=0;
	}
	if(cnt==0xff)
		cnt=0;
}
void led()
{
	led_stat=0xff;
	if(ledon)
	{
		if(dp_mode)
			led_stat&=0xfe;
		else
			led_stat&=0xfd;
		if((vrb2>=1.5*51&&vrb2<2.5*51)||vrb2>=3.5*51)
			led_stat&=0xfb;
		if((fre>=1000&&fre<5000)||fre>=10000)
			led_stat&=0xf7;
		if(out_mode)
			led_stat&=0xef;
	}
	latch(4,led_stat);
}
void display()
{
	unsigned int temp;
	float temp1; 
	led();
	if(smgon)
	{
		switch(dp_mode)
		{
			case 0:
				smg(1,10);
				if(fre>=100000) smg(3,fre/100000);
				if(fre>=10000) smg(4,(fre/10000)%10);
				if(fre>=1000) smg(5,(fre/1000)%10);
				if(fre>=100) smg(6,(fre/100)%10);
				if(fre>=10) smg(7,(fre/10)%10);
				smg(8,fre%10);
				break;
			case 1:
				temp1=vrb2/51.0;
				temp=temp1*100;
				smg(1,11);
				smg_p(6,temp/100);
				smg(7,(temp/10)%10);
				smg(8,temp%10);
				break;
		}
	}
	else
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
	if(!s4)
	{
		Delay10ms();
		if(!s4)
		{
			dp_mode=(dp_mode)?0:1;
			while(!s4)
				display();
		}
	}
	if(!s5)
	{
		Delay10ms();
		if(!s5)
		{
			out_mode=(out_mode)?0:1;
			while(!s5)
				display();
		}
	}
	if(!s6)
	{
		Delay10ms();
		if(!s6)
		{
			ledon=(ledon)?0:1;
			while(!s6)
				display();
		}
	}
	if(!s7)
	{
		Delay10ms();
		if(!s7)
		{
			smgon=(smgon)?0:1;
			while(!s7)
				display();
		}
	}
}	
void set()
{
	latch(5,0x00);
	latch(4,0xff);
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