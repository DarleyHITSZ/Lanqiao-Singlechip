#include <STC15F2K60S2.H>
#include "iic.h"
#include "smg.h"

unsigned char pwd[6]={8,8,8,8,8,8}, now_pwd[6]=0;
unsigned char now_pwd_ws=0, dp_mode=0, count_t=0, i, led_stat=0xff;
sbit r1=P3^0; sbit r2=P3^1; sbit r3=P3^2; sbit r4=P3^3;
sbit c1=P4^4; sbit c2=P4^2; sbit c3=P3^5; sbit c4=P3^4;
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
void init_stat()
{
	latch(4,0xff);
	latch(5,0x00);
	latch(7,0xff);
}
void servicet0() interrupt 1
{
	unsigned char judge;
	if(dp_mode==1||dp_mode==2) ++count_t;
	if(dp_mode==1&&now_pwd_ws==6&&(led_stat&0x01)==0x01)
	{
		judge=1;
		for(i=0;i<6;i++)
			if(pwd[i]!=now_pwd[i])
			{
				judge=0;
				break;
			}
		if(judge)
		{
			latch(5,0x10);
			now_pwd_ws=0;
			dp_mode=2;
		}
		else
		{
			led_stat&=0xfe;
		}
		count_t=0;
	}
	if(count_t==100)
	{
		if((led_stat&0x01)==0x00)
		{
			now_pwd_ws=0;
			dp_mode=0;
			led_stat=(led_stat&0xfe)|0x01;	
		}
		if(dp_mode==2)
		{			
			dp_mode=0;
			now_pwd_ws=0;	
		}
	}	
	if(count_t==0xff)
		count_t=0;
}
void led()
{
	if(dp_mode==1)
		led_stat=led_stat&0xbf;
	else
		led_stat=(led_stat&0xbf)|0x40;
	if(dp_mode==3)
		led_stat=led_stat&0x7f;
	else
		led_stat=(led_stat&0x7f)|0x80;
	latch(4,led_stat);
}
void display()
{
	led();
	switch(dp_mode)
	{
		case 0:
			init_stat();
			break;
		case 1:
			smg(1,10);
			for(i=8;i>(8-now_pwd_ws);i--)
			{
				smg(i,now_pwd[i-9+now_pwd_ws]);
			}
			break;
		case 2:
			smg(1,0);
			smg(5,0);
			smg(6,12);
			smg(7,13);
			smg(8,14);
			break;
		case 3:
			smg(1,11);
			for(i=8;i>(8-now_pwd_ws);i--)
			{
				smg(i,now_pwd[i-9+now_pwd_ws]);
			}
			break;
	}
	latch(7,0xff);
}
void setpwd()
{
	for(i=0;i<6;i++)
		pwd[i]=now_pwd[i];
	for(i=0;i<6;i++)
		AT_write(i,now_pwd[i]);
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
	if((dp_mode==1||dp_mode==3)&&now_pwd_ws<6)
	{
		r1=0;
		r2=r3=r4=c1=c2=c3=c4=1;
		if(c1==0)
		{
			Delay10ms();
			if(c1==0)
			{
				now_pwd[(++now_pwd_ws)-1]=0;
				while(c1==0)
					display();
			}
		}
		if(c2==0)
		{
			Delay10ms();
			if(c2==0)
			{
				now_pwd[(++now_pwd_ws)-1]=1;
				while(c2==0)
					display();
			}
		}
		if(c3==0)
		{
			Delay10ms();
			if(c3==0)
			{
				now_pwd[(++now_pwd_ws)-1]=2;
				while(c3==0)
					display();
			}
		}
		if(c4==0)
		{
			Delay10ms();
			if(c4==0)
			{
				now_pwd[(++now_pwd_ws)-1]=3;
				while(c4==0)
					display();
			}
		}
		
		r2=0;
		r1=r3=r4=c1=c2=c3=c4=1;
		if(c1==0)
		{
			Delay10ms();
			if(c1==0)
			{
				now_pwd[(++now_pwd_ws)-1]=4;
				while(c1==0)
					display();
			}
		}
		if(c2==0)
		{
			Delay10ms();
			if(c2==0)
			{
				now_pwd[(++now_pwd_ws)-1]=5;
				while(c2==0)
					display();
			}
		}
		if(c3==0)
		{
			Delay10ms();
			if(c3==0)
			{
				now_pwd[(++now_pwd_ws)-1]=6;
				while(c3==0)
					display();
			}
		}
		if(c4==0)
		{
			Delay10ms();
			if(c4==0)
			{
				now_pwd[(++now_pwd_ws)-1]=7;
				while(c4==0)
					display();
			}
		}
		
		r3=0;
		r1=r2=r4=c1=c2=c3=c4=1;
		if(c1==0)
		{
			Delay10ms();
			if(c1==0)
			{
				now_pwd[(++now_pwd_ws)-1]=8;
				while(c1==0)
					display();
			}
		}
		if(c2==0)
		{
			Delay10ms();
			if(c2==0)
			{
				now_pwd[(++now_pwd_ws)-1]=9;
				while(c2==0)
					display();
			}
		}
	}
	
	r4=0;
	r1=r2=r3=c1=c2=c3=c4=1;
	if(c2==0&&(dp_mode==1||dp_mode==3)&&((led_stat&0x01)==0x01))
	{
		Delay10ms();
		if(c2==0)
		{
			now_pwd_ws=0;
			while(c2==0)
				display();
		}
	}
	if(c3==0&&(dp_mode==2||dp_mode==3))
	{
		Delay10ms();
		if(c3==0)
		{
			if(dp_mode==2)
				dp_mode=3;
			if(now_pwd_ws==6&&dp_mode==3)
			{
				setpwd();
				now_pwd_ws=0;
				dp_mode=2;
				count_t=0;
			}
			while(c3==0)
				display();
		}
	}
	if(c4==0&&dp_mode==0)
	{
		Delay10ms();
		if(c4==0)
		{
			dp_mode=1;
			while(c4==0)
				display();
		}
	}
}
void init_pwd()
{
	if(AT_read(0x08)==0)
	{
		for(i=0;i<6;i++)
			AT_write(i,pwd[i]);
		AT_write(0x08,0x01);
	}
	else
	{
		for(i=0;i<6;i++)
			pwd[i]=AT_read(i);
	}
}
void main()
{
	init_stat();
	init_timer0();
	init_pwd();
	while(1)
	{
			scan();
			display();
	}
}