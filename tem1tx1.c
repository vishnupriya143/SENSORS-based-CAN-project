#include <LPC21xx.H>
#include"4bitcode.h"
//#include "delay.h"
#include "spi.h"  // MCP3204 SPI ADC

typedef unsigned int u32;

// CAN2 Message structure
typedef struct {
	u32 id;       // CAN identifier
	u32 rtr;      // Remote Transmission Request flag (0 = data frame, 1 = remote frame)
	u32 dlc;      // Data Length Code (number of bytes)
	u32 byteA;    // Data bytes 1-4 (packed into 32 bits)
	u32 byteB;    // Data bytes 5-8 (packed into 32 bits)
} CAN2_MSG;

void can2_init(void) {
	PINSEL1 |= 0x00014000;    // Select CAN2 pins: P0.23 = RD2, P0.24 = TD2
	VPBDIV = 1;            // Set Peripheral Clock (PCLK) to 60 MHz (same as CPU)
	C2MOD = 0x01;             // Enter reset mode to configure CAN2
	AFMR = 0x02;              // Acceptance filter off (accept all messages)
	C2BTR = 0x001C001D;       // Set baud rate timing: 125 kbps @ 60 MHz PCLK
	C2MOD = 0x00;             // Exit reset mode, enter normal operation
}

void can2_tx(CAN2_MSG m1) {
	C2TID1 = m1.id;                 // Set message ID
	C2TFI1 = (m1.dlc << 16);        // Set DLC in the transmit frame info

	if (m1.rtr == 0) {              // If data frame
		C2TFI1 &= ~(1 << 30);       // Clear RTR bit
		C2TDA1 = m1.byteA;          // Load first 4 data bytes
		C2TDB1 = m1.byteB;          // Load next 4 data bytes (unused here)
	} else {                       // If remote frame
		C2TFI1 |= (1 << 30);        // Set RTR bit
	}

	C2CMR = (1 << 0) | (1 << 5);   // Request transmission using buffer 1 and abort any previous transmission
	while ((C2GSR & (1 << 3)) == 0);  // Wait until transmission is complete
}

int main(void) {
	CAN2_MSG m1;
	lcd_config();
	lcd_string("temperature sensor");
	delay_ms(500);
	lcd_com(0x01);
	can2_init();       // Initialize CAN2 module
	Init_SPI0();       // Initialize SPI0 for MCP3204 ADC

	m1.id = 0x01;      // Set CAN message ID for this data (e.g., temperature sensor)
	m1.rtr = 0;        // Data frame (not remote)
	m1.dlc = 2;        // Two bytes data length (although you send 4 bytes here)

	m1.byteB = 0;      // Clear unused bytes

	while (1) {
		float f1 = Read_ADC_MCP3204(0);
		int temp = f1 * 100;
		m1.byteA = temp;  // Read 12-bit ADC value from MCP3204 channel 0
		lcd_com(0x80);        // Line 1
        lcd_string("temp:");
        lcd_com(0x80 + 9);    // Line 1, col 10
        lcd_float(temp);
        lcd_string(" c");
		can2_tx(m1);                              // Transmit message on CAN bus
		delay_s(1);                             // Wait 1 second before next reading
	}
}

