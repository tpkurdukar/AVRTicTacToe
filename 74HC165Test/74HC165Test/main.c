/*
 * 74HC165Test.c
 *
 * Created: 12/26/2020 9:19:10 PM
 * Author : prair
 */ 

#include <avr/io.h>
#include <stdio.h>


#define F_CPU 16000000UL
#include <util/delay.h>
#define BAUD 9600
#define BRC ( (F_CPU / 16 / BAUD) - 1)

#define PISO_DDR DDRC
#define PISO_PORT PORTC
#define DATA_IN PINC5
#define DATA_PORT PINC
#define LD PORTC3 // Active low parallel LD
#define PISO_CLK PORTC2
#define CLK_INH PORTC4

uint16_t shift_74HC165();
void USART_init();
void USART_putchar(unsigned char c);

static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
int main(void)
{
	PISO_DDR = (1 << LD) | (1 << PISO_CLK) | (1 << CLK_INH);
	USART_init();
	stdout = &mystdout;
	uint16_t old = 0xFFFF;
	
    /* Replace with your application code */
    while (1) 
    {
		uint16_t data = shift_74HC165();
		printf("%d\r\n", data);
		_delay_ms(100);
    }
}

void USART_putchar(unsigned char c) {
	// Note: code taken from datasheet of atmega328p
	while(!(UCSR0A & (1 << UDRE0))); // poll, wait for empty transmit buffer
	
	// once data is placed in UDR0, it transmits automatically
	UDR0 = c;
}

void USART_init() {
	UBRR0H = (BRC >> 8);
	UBRR0L = BRC; // load baud rate
	
	UCSR0B = (1 << TXEN0); // enable transmitter
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // set number of bits per frame to 8 (1 char)
	
}

uint16_t shift_74HC165() {
	// Snapshot of current state
	PISO_PORT &= ~(1 << LD); // LD active low
	_delay_us(50);
	PISO_PORT |= (1 << LD);
	_delay_us(50);
	
	PISO_PORT &= ~(1 << CLK_INH); // disable CLK inhibit
	uint16_t data = 0;
	for(uint16_t i = 0; i < 16; i++) {
		data <<= 1;
		
		// Read in value in Qh
		if(DATA_PORT & (1 << DATA_IN)) {
			data |= 1;
		}
		
		PISO_PORT |= (1 << PISO_CLK); // pulse CLK signal to shift
		_delay_us(50);
		PISO_PORT &= ~(1 << PISO_CLK);
		//printf("%d\r\n", data);
		
	}
	
	return data;
}

