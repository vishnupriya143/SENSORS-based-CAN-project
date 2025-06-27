#include <LPC21xx.H>
#include "delay.h"
#include "4bitcode.h"
//ltrasonic Module HC-SR04 works on the principle of SONAR and RADAR system.
// --- ULTRASONIC PINS ---
#define TRIG (1 << 1) // P0.1 output
#define ECHO (1 << 2) // P0.2 input

typedef unsigned int u32;
unsigned int distance;

typedef struct {
	u32 id;
	u32 rtr;
	u32 dlc;
	u32 byteA;
	u32 byteB;
} CAN2_MSG;

// Initialize CAN2 controller
void can2_init(void) {
	PINSEL1 = 0x00014000; // P0.23=RD2, P0.24=TD2 for CAN2
	VPBDIV = 1;            // PCLK = 60 MHz
	C2MOD = 1;             // Enter reset mode to configure CAN2
	AFMR = 2;              // Accept all messages
	C2BTR = 0x001C001D;    // Set baud rate to 125 kbps at 60 MHz PCLK
	C2MOD = 0;             // Normal mode (start CAN2)
}

// Transmit a CAN message using buffer 1
void can2_tx(CAN2_MSG m1) {
	C2TID1 = m1.id;
	C2TFI1 = (m1.dlc << 16);
	if (m1.rtr == 0) {
		C2TFI1 &= ~(1 << 30);  // Data frame
		C2TDA1 = m1.byteA;
		C2TDB1 = m1.byteB;
	} else {
		C2TFI1 |= (1 << 30);   // Remote frame (not used here)
	}
	C2CMR = (1 << 0) | (1 << 5); // Request to send buffer 1, abort any ongoing transmission
	while ((C2GSR & (1 << 3)) == 0); // Wait for transmit complete
}

// Initialize ultrasonic sensor pins
void ultrasonic_init(void) {
	IODIR0 |= TRIG;   // TRIG pin output
	IODIR0 &= ~ECHO;  // ECHO pin input
}

// Measure distance in cm using ultrasonic sensor
float get_distance_cm(void) {
	// Generate a 10us pulse on TRIG
	float time = 0, distance = 0;
	T0TC = T0PC = 0;
	IOSET0 = TRIG;
	delay_us(10);
	IOCLR0 = TRIG;
	// Wait for ECHO to go HIGH (start timing)
	while (!(IOPIN0 & ECHO));
	// Start timer (Timer0)
	T0TCR = 0x01;
	// Wait for ECHO to go LOW or timeout after ~60ms
	while (IOPIN0 & ECHO);
	// Stop timer
	T0TCR = 0;
    time = T0TC;
	if(time < 60000)
	{
		distance = time / 58.0;
	}
	else
	{
		distance = 0;
	}
  // Convert timer counts (us) to distance in cm
	return distance;
}


int main(void) {
	CAN2_MSG msg;
	can2_init();
    lcd_config();       // Initialize LCD
    ultrasonic_init();  // Setup ultrasonic
	msg.id = 0x03;    // CAN ID for ultrasonic data
	msg.rtr = 0;      // Data frame
	msg.dlc = 2;      // Data length: 2 bytes (distance)
	msg.byteB = 0;    // Clear unused data bytes
    lcd_string("Ultrasonic Sensor");
    delay_ms(1000);
    lcd_com(0x01);  // Clear LCD

    while (1) {
        int distance = (int)get_distance_cm();
		msg.byteA = distance;
        lcd_com(0x80);        // Line 1
        lcd_string("Distance:");
        lcd_com(0x80 + 9);    // Line 1, col 10
        lcd_integer(distance);
        lcd_string(" cm");
		can2_tx(msg);
        delay_ms(500);  // Wait before next reading
	}
}
/*What is an Ultrasonic Sensor?
An ultrasonic sensor measures distance by using sound waves at frequencies higher than humans can hear (typically above 20 kHz). 
It sends out a high-frequency sound pulse and listens for the echo reflected back from an object.

How Does It Work?
    1. Triggering the Sensor:
The sensor has a trigger pin where you send a short pulse (usually 10 microseconds). 
This pulse tells the sensor to emit an ultrasonic sound wave — a burst of high-frequency sound.
    2. Sound Wave Emission:
The sensor’s transmitter sends the ultrasonic wave traveling through the air.
    3. Echo Reception:
If the wave hits an object, it bounces back as an echo. The sensor’s receiver detects this returning wave.
    4. Measuring Time:
The sensor measures the time elapsed between sending the pulse and receiving the echo.
    5. Calculating Distance:
Using the formula:
       Distance=(Speed of sound×Time elapsed?)/2
    6. The division by 2 is because the time is for the round trip (go + return).
        ? The speed of sound in air is roughly 343 meters per second (or 0.0343 cm/µs).
        ? In microseconds and centimeters, a common formula is:
        ? Distance (cm)=Time (µs)/58?

Why Divide by 58?
    • Time measured is in microseconds (µs).
    • Speed of sound ˜ 343 m/s = 0.0343 cm/µs.
    • Round trip: sound travels to the object and back, so divide by 2.
    • So,
Distance=(Time×0.0343)/2==Time/58 

Summary of Pins:
Pin                       Function
Trigger (TRIG)   Input to sensor; pulse triggers ultrasonic burst
Echo (ECHO)      Output from sensor; pulse length represents echo time
       
*/

