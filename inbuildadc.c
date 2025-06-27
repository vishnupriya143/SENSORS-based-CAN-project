#include <LPC21xx.h>     // Header file for LPC21xx microcontroller
#include "4bitcode.h"    // LCD 4-bit mode driver (assumed already working)
// #include "delay.h"    // Optional delay header (user commented it)
#include <stdio.h>       // For sprintf()

unsigned int adc_value = 0;       // Global ADC result variable
float voltage = 0.0;     // Global voltage value
int ab;
typedef unsigned int u32;
// CAN2 Message Structure
typedef struct {
	u32 id;
	u32 rtr;
	u32 dlc;
	u32 byteA;
} CAN2_MSG;
 CAN2_MSG msg;
// CAN2 Initialization
void can2_init(void) {

	VPBDIV = 1;               // PCLK = 60 MHz
	C2MOD = 1;                // Enter reset mode
	AFMR = 2;                 // Accept all messages
	C2BTR = 0x001C001D;       // 125 kbps @ 60 MHz
	C2MOD = 0;                // Enter normal mode
}

// CAN2 Transmit Function
void can2_tx(CAN2_MSG m1) {
	C2TID1 = m1.id;
	C2TFI1 = (m1.dlc << 16);

	if (m1.rtr == 0) {
		C2TFI1 &= ~(1 << 30);    // Data frame
		C2TDA1 = m1.byteA;
	} else {
		C2TFI1 |= (1 << 30);     // Remote frame
	}

	C2CMR = (1 << 0) | (1 << 5);     // Transmit request
	while ((C2GSR & (1 << 3)) == 0); // Wait for transmission complete
}

//  Function to read ADC and return 10-bit result
int ADC_Conversion() {
    
	 ADCR = (1 << 21) | (13 << 8) | (1 << 2) | (1 << 24);

    while ((ADDR & 0x80000000) == 0); // Wait until DONE bit (31) is set

    ab = (ADDR & 0x0000FFC0);  // Extract bits [15:6] (10-bit result)
    ab = ab >> 6;
     return ab;
}

// Function to display ADC and voltage on LCD
void ADC_Check() {
    char str[16];
	ADCR = (1 << 21) | (13 << 8) | (1 << 2);  // PDN=1, CLKDIV=13, SEL=AD0.2
  	adc_value = ADC_Conversion(); // Call ADC function
	msg.id = 0x02;  // Example CAN message ID
	msg.rtr = 0;    // Data frame
	msg.dlc = 4;    // Two bytes (high and low)
	msg.byteA = adc_value; 	// Low byte
	can2_tx(msg);  // Send ADC value over CAN2
    lcd_com(0x80);                // Move LCD cursor to 1st row
    lcd_string("ADC: ");
    lcd_integer(adc_value);      // Display integer ADC value
    voltage = (3.3 * adc_value) / 1023.0; // Convert ADC result to voltage
    lcd_com(0xC0);                // Move LCD cursor to 2nd row
    sprintf(str, "Volt: %.2fV", voltage);
    lcd_string(str);             // Display voltage (e.g., 2.54V)
    delay_ms(200);               // Small delay for readability
}

//  Main function
int main() {
	lcd_config();
	PINSEL1 = 0x04014000;    // P0.23 = RD2, P0.24 = TD2 (CAN2),p0.29->ad2
	 can2_init();  // Initialize CAN2
     while (1) 
	 {
        ADC_Check();       // Read and display repeatedly
      }
}
