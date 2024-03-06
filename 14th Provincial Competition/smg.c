#include <STC15F2K60S2.H>

code unsigned char table[] = 
{
0xc0, //0
0xf9, //1
0xa4, //2
0xb0, //3
0x99, //4
0x92, //5
0x82, //6
0xf8, //7
0x80, //8
0x90, //9
0x88, //A 10
0xc6, //C 11
0x86, //E 12
0x8e, //F 13
0x8c, //P 14
0x89, //H 15	
0xbf, //- 16
};
void Delay500us()		//@12.000MHz
{
	unsigned char i, j;

	i = 6;
	j = 211;
	do
	{
		while (--j);
	} while (--i);
}

void delay_smg(unsigned char t)
{
	while(t--);
}
void latch(unsigned char yn,unsigned char value)
{
		P0=value;
		P2=(P2&0x1f)|(yn<<5);
		P2=P2&0x1f;
}
void smg(unsigned char pos,unsigned char value)
{
	latch(6,0x01<<(pos-1));
	latch(7,table[value]);
	Delay500us();
}
void smg_p(unsigned char pos,unsigned char value)
{
	latch(6,0x01<<(pos-1));
	latch(7,table[value]-0x80);
	Delay500us();
}
