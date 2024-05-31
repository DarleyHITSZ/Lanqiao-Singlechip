#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"
#include "smg.h"
#include "utwave.h"

sbit c1=P4^4;sbit c2=P4^2;
sbit r3=P3^2;sbit r4=P3^3;
double da_low=1;
float now_temp;
int cor_d=0,now_d;
unsigned int v=340, last_d[10], ut_time;
unsigned char i=0, cnt, set_d=40, set_temp=30, led_stat=0xff, old_led_stat=0xff, t_record, t_reset, da_output;
unsigned char dp_mode=1, set_dp_mode=1, dw=0, f_dp_mode=1, record=0, output=0, relay=0, old_relay=0;
void init_timer()
{
	TMOD=0x00;
	AUXR&=0x3f;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	EA=1;
	ET0=1;
	TR0=1;
}
void led();
void servicet0() interrupt 1
{
	if(cnt<0xff) ++cnt;
	else cnt=0;
	if(t_reset<0xff) ++t_reset;
	else t_reset=0;
	now_d=1.0*v*ut_time/2.0*0.0001+cor_d;
	if(cnt%2==0)
	{
		led();
		if(now_d>=set_d-5&&now_d<=set_d+5&&now_temp<=set_temp)
			relay=1; 
		else
			relay=0;
		if(old_relay!=relay)
		{
			if(relay==1) latch(5,0x10);
			else latch(5,0x00);
			old_relay=relay;
		}
	}
	if(cnt%19==0)
	{
		now_temp=readtemp();
	}
	if(cnt%20==0) //1s
	{
		ut_time=time_utwave();
		if(now_d<0) now_d=0;
		if(record==1)
		{
			last_d[i++]=now_d;
		}
	}
	if(record==1)
	{
		++t_record;	
		if(t_record==120)
		{
			i=0;
			record=0;
			t_record=0;
		}	
	}
	if(output==1&&last_d[0]!=0&&cnt%2==0)
	{
		if(last_d[i]<=10)
			da_output=1*51;
		if(last_d[i]>10&&last_d[i]<90)
			da_output=(1.0*(5-da_low)/80*last_d[i]+1.0*(90.0*da_low-50)/80)*51;
		if(last_d[i]>=90)
			da_output=5*51;
		DA_write(da_output);
		++i;
		if(last_d[i]==0)
			output=0;
	}
}
void led()
{
	if(dp_mode==1)
	{
		if(now_d<=255)
			led_stat=~now_d;
		else
			led_stat=0x00;
	}
	if(dp_mode==2)
	{
		led_stat=0x7f;
	}
	if(dp_mode==3)
	{
		led_stat|=0xfe;
		led_stat=(led_stat|0x01)&((~led_stat)|0xfe);
	}
	if(led_stat!=old_led_stat)
	{
		latch(4,led_stat);
		old_led_stat=led_stat;
	}
}
void display()
{
	unsigned int temp;
	switch(dp_mode)
	{
		case 1:
			temp=now_temp*10;
			if(temp>=100) smg(1,temp/100);
			smg_p(2,(temp/10)%10);
			smg(3,temp%10);
			smg(4,12);
			if(dw==0)
			{
				if(now_d>=1000) smg(5,now_d/1000);
				if(now_d>=100)smg(6,(now_d/100)%10);
				if(now_d>=10) smg(7,(now_d/10)%10);
				smg(8,now_d%10);
			}
			else
			{
				if(now_d>=1000) smg(5,now_d/1000);
				smg_p(6,(now_d/100)%10);
				smg(7,(now_d/10)%10);
				smg(8,now_d%10);
			}
			break;
		case 2:
			smg(1,11);
			smg(2,set_dp_mode);
			if(set_dp_mode==1)
			{
				smg(7,set_d/10);
				smg(8,set_d%10);
			}
			else
			{
				smg(7,set_temp/10);
				smg(8,set_temp%10);
			}
			break;
		case 3:
			smg(1,10);
			smg(2,f_dp_mode);
			if(f_dp_mode==1)
			{
				if(cor_d<0)
				{
					temp=-cor_d;
					if(temp>=10)
					{
						smg(6,12);
						smg(7,temp/10);
						smg(8,temp%10);
					}
					else
					{
						smg(7,12);
						smg(8,temp);
					}
				}
				else
				{
					temp=cor_d;
					if(temp>=10) smg(7,temp/10);
					smg(8,temp%10);
				}
			}
			if(f_dp_mode==2)	
			{
				if(v>=1000) smg(5,v/1000);
				if(v>=100) smg(6,(v/100)%10);
				if(v>=10) smg(7,(v/10)%10);
				smg(8,v%10);
			}
			if(f_dp_mode==3)
			{
				temp=da_low*10;
				smg_p(7,temp/10);
				smg(8,temp%10);
			}
			break;
	}
}
void reset()
{
	dp_mode=1;
	dw=0;
	set_d=40;
	set_temp=30;
	cor_d=0;
	v=340;
	da_low=1.0;
}
void check()
{
	if(record==1) return;
	c1=0;
	c2=r3=r4=1;
	if(r4==0) //s4
	{
		if(dp_mode<3) ++dp_mode;
		else dp_mode=1;
		set_dp_mode=1;
		f_dp_mode=1;
		dw=0;
		while(r4==0)
			display();
	}
	if(r3==0) //s5
	{
		if(dp_mode==1)
		{
			dw=dw?0:1;
		}
		if(dp_mode==2)
		{
			if(set_dp_mode<2) ++set_dp_mode;
			else set_dp_mode=1;
		}
		if(dp_mode==3)
		{
			if(f_dp_mode<3) ++f_dp_mode;
			else f_dp_mode=1;
		}
		while(r3==0)
			display();
	}
	c2=0;
	c1=r3=r4=1;
	if(r3==0&&r4==0)
	{
		t_reset=0;
		while(r3==0&&r4==0&&t_reset<=40)
			display();
		if(t_reset>=40)
			reset();
	}
	if(r4==0) //s8
	{
		if(dp_mode==1)
		{
			for(i=0;i<10;i++)
				last_d[i]=0;
			i=0;
			t_record=0;
			record=1;
		}
		if(dp_mode==2)
		{
			if(set_dp_mode==1)
			{
				if(set_d<90)
					set_d+=10;
			}
			else
			{
				if(set_temp<80)
					++set_temp;
			}
		}
		if(dp_mode==3)
		{
			if(f_dp_mode==1)
			{
				if(cor_d<90)
					cor_d+=5;
			}
			if(f_dp_mode==2)
			{
				if(v<9990)
					v+=10;
			}
			if(f_dp_mode==3)
			{
				if(da_low<2.0)
					da_low+=0.1;
			}
		}
		while(r4==0)
			display();
	}
	if(r3==0) //s9
	{
		if(dp_mode==1)
		{
			i=0;
			output=1;
		}
		if(dp_mode==2)
		{
			if(set_dp_mode==1)
			{
				if(set_d>10)
					set_d-=10;
			}
			else
			{
				if(set_temp>0)
					--set_temp;
			}
		}
		if(dp_mode==3)
		{
			if(f_dp_mode==1)
			{
				if(cor_d>-90)
					cor_d-=5;
			}
			if(f_dp_mode==2)
			{
				if(v>10)
					v-=10;
			}
			if(f_dp_mode==3)
			{
				if(da_low>0.2)
					da_low-=0.1;
			}
		}
		while(r3==0)
			display();
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
	init_timer();
	while(1)
	{
		display();
		check();
	}
}