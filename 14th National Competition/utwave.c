#include <STC15F2K60S2.H>
#include <intrins.h>

sbit TX=P1^0;
sbit RX=P1^1;

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
void init_utwave()
{
	TX=1;
	Delay12us();
	TX=0;
}

unsigned int time_utwave()
{
	unsigned int time;
	TH1=0;
	TL1=0;
	init_utwave();
	TR1=1;
	while(RX==1&&TF1==0);
	TR1=0;
	if(TF1==0)
	{
		time=TH1;
		time=(time<<8)|TL1;
		return time;
	}
	else
	{
		TF1=0;
		return 0;
	}
}