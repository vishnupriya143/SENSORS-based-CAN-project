#include<LPC21xx.h>
//#include"4bitcode.h"
typedef unsigned char u8;
typedef signed char   s8;
typedef unsigned short int  u16;
typedef signed short   int  s16;
typedef unsigned int  u32;
typedef signed   int  s32;
typedef float f32;
typedef double f64;
f32 t;
#define CS 7

// SPCR Bits Setting 
#define Mode_0     0x00  // CPOL 0 CPHA 0
#define Mode_1     0x08  // CPOL 0 CPHA 1
#define Mode_2     0x10  // CPOL 1 CPHA 0
#define Mode_3     0x18  // CPOL 1 CPHA 1
#define MSTR_BIT   5     // SPI0 as Master 
#define LSBF_BIT   6     // default MSB first,if set LSB first
#define SPIE_BIT   7     //SPI Interrupt Enable Bit

// SPSR bits
#define SPIF_BIT   7    // Data Transfer Completion Flag

//SPINT bit
#define SPIINTF_BIT 0   //SPI Interrupt Flag Bit

#define SETBIT(WORD,BITPOS)         (WORD|=1<<BITPOS)
#define CLRBIT(WORD,BITPOS)         (WORD&=~(1<<BITPOS))
#define CPLBIT(WORD,BITPOS)         (WORD^=(1<<BITPOS))
#define WRITEBIT(WORD,BITPOS,BIT)   (BIT ? SETBIT(WORD,BITPOS): CLRBIT(WORD,BITPOS))
#define READBIT(WORD,BITPOS)        ((WORD>>BITPOS)&1)

void Init_SPI0(void);
u8 SPI0(u8 data);
f32 Read_ADC_MCP3204(u8 channelNo);

void Init_SPI0(void)
{
	
	PINSEL0 |=0X00001500; 
  S0SPCCR  = 150;// to set 100kbps(The SPI rate may be 
	               //calculated as: PCLK rate / SPI_RATE
	S0SPCR  = (1<<MSTR_BIT|Mode_3); //spi module in master mode,
                                  //CPOL =1,CCPHA = 1. MSB first
	IODIR0 |= 1<<7;
}

u8 SPI0(u8 data)
{
   u8 stat;
   stat = S0SPSR;    //clear SPIF 
   S0SPDR = data;   // load spi tx reg
   while(READBIT(S0SPSR,SPIF_BIT)==0); // wait for transmission to complete
   return S0SPDR;    // read data from SPI data reg, place into buffer 
}

f32 Read_ADC_MCP3204(u8 channelNo)
{
  u8 hByte,lByte;
  u32 adcVal=0;
   
  //select/activate chip 
  CLRBIT(IOPIN0,CS);
	//delay_ms(100);
	
  SPI0(0x06);
  hByte = SPI0(channelNo<<6);
  lByte = SPI0(0x00);
	
	//de-select/de-activate chp
	SETBIT(IOSET0,CS);
	//delay_ms(100);
  adcVal=((hByte&0x0f)<<8)|lByte;
  return ((adcVal*3.3)/4096);
}




