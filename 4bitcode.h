#include<LPC21xx.h>
#include "delay.h"
#define lcd_d 0xf<<20                                                
#define RS 1<<17
#define RW 1<<18						 
#define e 1<<19
void lcd_confi(void);
void lcd_com(unsigned char);
void lcd_data(unsigned char);
void lcd_string(unsigned char*);
//void delay_ms(unsigned int ms);
void lcd_float(float f);
void lcd_integer(int n);			    			      
void lcd_config(void)
{
PINSEL0=0;
IODIR1=lcd_d|RS|e|RW;
lcd_com(0x01);
lcd_com(0x02);
lcd_com(0x0c);
lcd_com(0x28);
lcd_com(0x80);
}
void lcd_com(unsigned char c)
{
IOCLR1=lcd_d;
IOSET1=(c&0xf0)<<16;
IOCLR1=RS;
IOCLR1=RW;
IOSET1=e;
delay_ms(2);
IOCLR1=e;

IOCLR1=lcd_d;
IOSET1=(c&0x0f)<<20;
IOCLR1=RS;
IOCLR1=RW;
IOSET1=e;
delay_ms(2);
IOCLR1=e;
}
void lcd_data(unsigned char d)
{
IOCLR1=lcd_d;
IOSET1=(d&0xf0)<<16;
IOSET1=RS;
IOCLR1=RW;
IOSET1=e;
delay_ms(2);
IOCLR1=e;

IOCLR1=lcd_d;
IOSET1=(d&0x0f)<<20;
IOSET1=RS;
IOCLR1=RW;
IOSET1=e;
delay_ms(2);
IOCLR1=e;
}
void lcd_string(unsigned char*s)
{
 while(*s)
 lcd_data(*s++);
}
void lcd_integer(int n)
{
unsigned char arr[5];
signed int i=0;
if(n==0)
lcd_data('0');
else
{
if(n<0)
{
lcd_data('-');
 n=-n;
}
while(n>0)
{
arr[i++]=n%10;
n=n/10;
}
for(--i;i>=0;i--)
lcd_data(arr[i]+48);
}
}
void lcd_float(float f)
{
int x=f;
lcd_integer(x);
lcd_data('.');
f=(f-x)*100;
lcd_integer(f);
}
