#include "txSerial.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define BRC ( (F_CPU / 16 / BAUD) - 1)

void USART_init() {
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC; // load baud rate
	
	UCSR0B = (1 << TXEN0); // enable transmitter
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // set number of bits per frame to 8 (1 char)
	
}

void USART_putchar(unsigned char c) {
	// Note: code taken from datasheet of atmega328p
	while(!(UCSR0A & (1 << UDRE0))); // poll, wait for empty transmit buffer
	
	// once data is placed in UDR0, it transmits automatically
	UDR0 = c;
}

void serial_write(char* string) {
	while(*string) {
		// write char to terminal
		USART_putchar(*string);
		// move to next char
		string++;
	}
}