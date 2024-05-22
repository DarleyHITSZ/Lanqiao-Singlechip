#include <STC15F2K60S2.H>
#include "iic.h"
#include "smg.h"
#include "ulwave.h"

sbit s4=P3^3;sbit s5=P3^2;sbit s6=P3^1;sbit s7=P3^0;
unsigned int set_f=9000, now_f, now_d, cnt_f, cnt, t_press; 
unsigned char now_wet, set_wet=40, set_d=60, old_DA_output, Y5_stat=0x00, old_Y5_stat, old_led_stat, led_stat=0xff, relay_stat=0, relay_times;
unsigned char dp_mode=1, dw=0 , set_dp_mode=1;

void init_timer()
{
	TMOD=0x06;
	TH0=255;
	TL0=255;
	T2H=(65535-200)/256;
	T2L=(65535-200)%256;
	EA=1;
	ET0=1;
	IE2=0x04;
	AUXR=AUXR&0x23|0x10;
	TR0=1;
}
void servicet0() interrupt 1
{
	++cnt_f;
}
void led()
{
	if(now_f>set_f) led_stat&=0xf7;
	else led_stat=led_stat&0xf7|0x08;
	if(now_wet>set_wet) led_stat&=0xef;
	else led_stat=led_stat&0xef|0x10;
	if(now_d>set_d) led_stat&=0xdf;
	else led_stat=led_stat&0xdf|0x20;
	switch(dp_mode)
	{
		case 1:
			led_stat&=0xfe;
			led_stat=led_stat&0xfb|0x04;
			break;
		case 2:
			led_stat&=0xfd;
			led_stat=led_stat&0xfe|0x01;
			break;
		case 3:
			led_stat&=0xfb;
			led_stat=led_stat&0xfd|0x02;
			break;
		case 4:
			if(set_dp_mode==1)
				led_stat=(led_stat|0x01)&((~led_stat)|0xfe);
			else
				led_stat=led_stat&0xfe|0x01;
			if(set_dp_mode==2)
				led_stat=(led_stat|0x02)&((~led_stat)|0xfd);
			else
				led_stat=led_stat&0xfd|0x02;
			if(set_dp_mode==3)
				led_stat=(led_stat|0x04)&((~led_stat)|0xfb);
			else
				led_stat=led_stat&0xfb|0x04;
			break;
	}
	if(led_stat!=old_led_stat)
	{
		latch(4,led_stat);
		old_led_stat=led_stat;
	}
}
void servicet2() interrupt 12
{
	unsigned char DA_output;
	if(cnt<0xffffffff) ++cnt;
	else cnt=0;
	if(t_press<6000) ++t_press;
	else t_press=0;
	if(now_f>set_f)  //0.1ms
	{
		if(cnt_f%5<=3)
			Y5_stat=Y5_stat&0xdf|0x20;
		else
			Y5_stat&=0xdf;
	}
	else
	{
		if(cnt_f%5<=3)
			Y5_stat&=0xdf;
		else
			Y5_stat=Y5_stat&0xdf|0x20;
	}	
	if(cnt%500==0) //0.1s
		led();
	if(cnt%1000==0) //200ms
	{
		if(now_d>set_d)
		{
			if(relay_stat==0)
			{
				++relay_times;
				AT_write(relay_times);
			}
			relay_stat=1;
			Y5_stat=Y5_stat&0xef|0x10;
		}
		else
		{
			relay_stat=0;
			Y5_stat=Y5_stat&0xef;
		}
		now_wet=1.0*AD_read()/51*20;
		if(now_wet<=set_wet) 
			DA_output=1*51;
		if(now_wet>set_wet&&now_wet<80)
			DA_output=(now_wet*4.0/(80-set_wet)+(80.0-5*set_wet)/(80-set_wet))*51.0;
		if(now_wet>=80)
			DA_output=5*51;
		if(DA_output!=old_DA_output)
		{
			DA_write(DA_output);
			old_DA_output=DA_output;
		}
	}	
	if(Y5_stat!=old_Y5_stat)
	{
		latch(5,Y5_stat);
		old_Y5_stat=Y5_stat;
	}
	if(cnt%2500==0) //500ms
	{
		now_f=cnt_f*2;
		cnt_f=0;
		now_d=data_utwave();
	}
}
void display()
{
	switch(dp_mode)
	{
		case 1://Hz
			smg(1,11);
			if(dw==0)
			{
				if(now_f>=100000) smg(3,now_f/100000);
				if(now_f>=10000) smg(4,(now_f/10000)%10);
				if(now_f>=1000) smg(5,(now_f/1000)%10);
				if(now_f>=100) smg(6,(now_f/100)%10);
				if(now_f>=10) smg(7,(now_f/10)%10);
				smg(8,now_f%10);
			}
			else
			{
				if(now_f>=100000) smg(5,now_f/100000);
				if(now_f>=10000) smg(6,(now_f/10000)%10);
				smg_p(7,(now_f/1000)%10);
				smg(8,(now_f/100)%10);
			}
			break;
		case 2://wet
			smg(1,12);
			if(now_wet>=10) smg(7,now_wet/10);
			smg(8,now_wet%10);
			break;
		case 3://distance
			smg(1,10);
			if(dw==0)
			{
				if(now_d>=100) smg(6,now_d/100);
				if(now_d>=10) smg(7,(now_d/10)%10);
				smg(8,now_d%10);
			}
			else
			{
				smg_p(6,now_d/100);
				smg(7,(now_d/10)%10);
				smg(8,now_d%10);
			}
			break;
		case 4:
			smg(1,13);
			if(set_dp_mode==1)
			{
				smg(2,1);
				if(set_f>=10000) smg(6,set_f/10000);
				smg_p(7,(set_f/1000)%10);
				smg(8,(set_f/100)%10);
			}
			if(set_dp_mode==2)
			{
				smg(2,2);
				smg(7,set_wet/10);
				smg(8,set_wet%10);
			}
			if(set_dp_mode==3)
			{
				smg(2,3);
				smg_p(7,set_d/100);
				smg(8,(set_d/10)%10);
			}
			break;
		}
}
void check()
{
	if(s4==0)
	{
		if(dp_mode<4) ++dp_mode;
		else dp_mode=1;
		set_dp_mode=1;
		dw=0;
		while(s4==0)
			display();
	}
	if(s5==0&&dp_mode==4)
	{
		if(set_dp_mode<3) ++set_dp_mode;
		else set_dp_mode=1;
		while(s5==0)
			display();
	}
	if(s6==0&&(dp_mode==4||dp_mode==3))
	{
		if(dp_mode==4)
		{
			if(set_dp_mode==1)
			{
				if(set_f<12000) set_f+=500;
				else set_f=1000;
			}
			if(set_dp_mode==2)
			{
				if(set_wet<60) set_wet+=10;
				else set_wet=10;
			}
			if(set_dp_mode==3)
			{
				if(set_d<120) set_d+=10;
				else set_d=10;
			}
		}
		else
			dw=dw ? 0 : 1 ;
		while(s6==0)
			display();
	}
	if(s7==0&&dp_mode!=3)
	{
		if(dp_mode==4)
		{
			if(set_dp_mode==1)
			{
				if(set_f>1000) set_f-=500;
				else set_f=12000;
			}
			if(set_dp_mode==2)
			{
				if(set_wet>10) set_wet-=10;
				else set_wet=60;
			}
			if(set_dp_mode==3)
			{
				if(set_d>10) set_d-=10;
				else set_d=120;
			}
		}
		else
			dw=dw ? 0 : 1 ;
		t_press=0;
		while(s7==0)
			display();
		if(t_press>=5000&&dp_mode==2)
		{
			relay_times=0;
			AT_write(0);
		}
	}
}
void init_set()
{
	relay_times=AT_read();
	latch(4,0xff);
	latch(5,0x00);
}
void main()
{
	init_set();
	init_timer();
	while(1)
	{
		display();
		check();
	}
}