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
0xc1, //U 10
0xbf, //- 11
};
void delay_smg(unsigned int t)
{
	while(--t);
}
void latch(unsigned char yn,unsigned char value)
{
	P2=(P2&0x1f)|(yn<<5);
	P0=value;
	P2&=0x1f;
}
void smg(unsigned char pos,unsigned char value)
{
	latch(6,0x01<<(pos-1));
	latch(7,table[value]);
	delay_smg(1000);
}
void smg_p(unsigned char pos,unsigned char value)
{
	latch(6,0x01<<(pos-1));
	latch(7,table[value]-0x80);
	delay_smg(1000);
}