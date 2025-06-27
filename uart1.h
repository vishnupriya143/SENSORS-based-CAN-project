// UART1 Initialization for ESP-01 (TxD1 = P0.8, RxD1 = P0.9)
void UART1_Init() {
    PINSEL0 |= 0x00050000;  // Enable TxD1 and RxD1
    U1LCR = 0x83;           // Enable DLAB
    U1DLM = 1;
    U1DLL = 134;             // Baud rate = 9600 for 15MHz
    U1LCR = 0x03;           // 8-bit, no parity, 1 stop bit
}

void UART1_TxChar(char ch) {
    while (!(U1LSR & 0x20));
    U1THR = ch;
}

void UART1_SendString(const char *str) {
    while (*str) {
        UART1_TxChar(*str++);
    }
}

// Send AT commands to connect ESP-01 to WiFi
void ESP_Init() {
    UART1_SendString("AT\r\n");
    delay_ms(2000);

    UART1_SendString("AT+CWMODE=1\r\n");  // Station mode
    delay_ms(2000);

    UART1_SendString("AT+CWJAP=\"12345678\",\"12345678\"\r\n"); // Your WiFi
    delay_ms(6000);
}


void ThingSpeak_Update(unsigned int field1, unsigned int field2, unsigned int field3) {
    char buffer[300];
    char cmd[50];
    int len;

    // 1. Start TCP connection
    UART1_SendString("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
    delay_ms(4000);

    // 2. Prepare the HTTP GET request with multiple fields
    sprintf(buffer,
        "GET /update?api_key=E4XPRIF4D1P6JS5R"
        "&field1=%d"
        "&field2=%d"
        "&field3=%d"
        " HTTP/1.1\r\n"
        "Host: api.thingspeak.com\r\n"
        "Connection: close\r\n\r\n",
        field1, field2, field3);

    len = strlen(buffer);

    // 3. Send length using AT+CIPSEND
    sprintf(cmd, "AT+CIPSEND=%d\r\n", len);
    UART1_SendString(cmd);
    delay_ms(2000);

    // 4. Send the actual data
    UART1_SendString(buffer);
    delay_ms(4000);
}
                                                                                                                                                                                                                                                                                                                                                                                                                  