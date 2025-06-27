#include <LPC21xx.H>
#include<string.h>
#include<stdio.h>
#include "delay.h"
#include"uart0.h"
#include "4bitcode.h"  // LCD 4-bit mode driver

typedef unsigned int u32;
typedef unsigned short u16;

// CAN2 message structure
typedef struct {
	u32 id;
	u32 rtr;
	u32 dlc;
	u32 byteA;
} CAN2_MSG;

// Global variables
CAN2_MSG m1;
char str[16];
int t;
u32 adc_value;
float voltage;
u32 distance;
char send_flag = 0;
unsigned int scounter = 0;
// CAN2 initialization
void can2_init(void) {
	PINSEL1 |= 0x00014000;      // P0.23 = RD2, P0.24 = TD2
	VPBDIV = 1;                 // PCLK = 60 MHz
	C2MOD = 0x01;               // Enter reset mode
	AFMR = 0x02;                // Accept all messages
	C2BTR = 0x001C001D;         // Baudrate = 125 kbps @ 60 MHz
	C2MOD = 0x00;               // Normal mode
}

// CAN2 receive function
void can2_rx(CAN2_MSG *m1) {
	while ((C2GSR & 0x01) == 0);  // Wait for message

	m1->id = C2RID;
	m1->dlc = (C2RFS >> 16) & 0xF;
	m1->rtr = (C2RFS >> 30) & 0x1;

	if (m1->rtr == 0) {
		m1->byteA = C2RDA;
	}

	C2CMR = (1 << 2);  // Release receive buffer
}

// CAN2 interrupt service routine
void int_isr(void) __irq {
	can2_rx(&m1);

	if (m1.id == 0x01) {  // Temperature sensor
		t = m1.byteA & 0xFFF;     // 12-bit ADC value
	}

	else if (m1.id == 0x02) {  // Moisture sensor
		adc_value = m1.byteA & 0x3FF; // 10-bit value
		voltage = (3.3 * adc_value) / 1023.0;
	
	}

	else if (m1.id == 0x03) {  // Ultrasonic sensor
		distance = m1.byteA;
	}

	VICVectAddr = 0x00;  // End of ISR
}

void timer_isr(void) __irq
{
    T1IR = 0x01; // Clear interrupt flag (MR0)
    scounter++;

    if (scounter >= 15)
    {
        send_flag = 1;
        scounter = 0;
    }

    VICVectAddr = 0x00; // Acknowledge interrupt
}

void display_lcd()
{
		lcd_com(0x01);
		delay_ms(2);
		lcd_com(0x80);
		lcd_string("T:");
		lcd_float(t);
		lcd_string("C");
		lcd_com(0x88);
		lcd_string("ADC:");
		lcd_integer(adc_value);
		lcd_com(0xc0);
		lcd_string("Dis:");
		lcd_integer(distance);
		lcd_string("cm");
		lcd_string(" V:");
		lcd_float(voltage);
}

void timer1_init(void)
{
    T1PR = 60000 - 1;   // Prescaler: 60MHz / 60000 = 1ms tick
    T1MR0 = 1000;       // 1000 x 1ms = 1s match
    T1MCR = 3;          // Interrupt & reset on MR0
    T1TCR = 1;          // Enable Timer1
}




// Main function
int main(void) {
	VPBDIV = 1;
	can2_init();         // Initialize CAN2
	lcd_config();        // Initialize LCD
	UART1_Init();
	ESP_Init();

	IODIR0 = 1 << 17;    // Set P0.17 as output
	IOSET0 = 1 << 17;    // Turn off LED

	lcd_string("SENSOR based CAN");
	delay_ms(1000);
	lcd_com(0x01);

	// Configure CAN2 interrupt
	VICIntSelect = 0x00000000;         // IRQ
	VICVectCntl0 = 0x20 | 27;          // Slot 0, CAN2 (IRQ 27)
	VICVectAddr0 = (u32)int_isr;
	VICIntEnable |= (1 << 27);          // Enable CAN2 interrupt
	C2IER = 0x01;
	
    VICIntSelect &= ~(1 << 5);        // Timer1 interrupt is IRQ
    VICVectCntl2 = (1 << 5) | 5;      // Enable slot and assign Timer1 (interrupt #5)
    VICVectAddr2 = (unsigned long)timer_isr; // ISR address
    VICIntEnable |= (1 << 5);         // Enable Timer1 interrupt

	timer1_init();
	while (1) {
		display_lcd();
		IOCLR0 = 1 << 17;              // LED ON
		if(send_flag == 1)
		{
			send_flag = 0;
			ThingSpeak_Update(t,adc_value,distance);         // Send to cloud
		}
		delay_ms(500);
	}

}
