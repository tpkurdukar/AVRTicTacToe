

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#define F_CPU 16000000ul
#include <util/delay.h>

#define BAUD 9600
#define BRC ( (F_CPU / 16 / BAUD) - 1)
void USART_init();
void USART_putchar(unsigned char c);

/* SPI pins */
#define CS PORTC1 /* This is connected to the latch on the 74hc595 */
#define MOSI PORTB3 /* Connected to data pin on 74hc595 */
#define SCK PORTB5 /* Connected to clock on 74hc595 */
/* No definition for MISO because it is not used */

uint8_t volatile i = 0;
uint16_t volatile data = 0x0;
uint8_t volatile flag = 0;

void master_transmit(uint16_t d);

static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
int main(void)
{
	/* This sets the SS' pin on the atmega328p as output
	 * Absolutely necessary step, despite SS' not being used here. If it is configured as input and picks up some noise from the programmer,
	 * master mode will turn off and the system thinks it is trying to receive something.
	 */
	DDRB = (1 << PORTB2);
	
	USART_init();
	stdout = &mystdout;
	
    /* Setup for SPI */
	SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR); /* Enable SPI interrupts, enable SPI, set as master */
	/* Note: CPOL and CPHA set so that the leading edge of SCK is the rising edge, and sampling occurs on rising edge */
	/* Using default SCK frequency of F_CPU / 4 */
	
	DDRC = (1 << CS); /* CS for 74hc595 is on DDRC. Just using a GPIO pin here */
	DDRB |= (1 << MOSI) | (1 << SCK); /* SPI is on DDRB */
	//sei(); /* Enable interrupts */
	
    while (1) 
    {
		if(i > 8) i = 0;
		data = 1 << i;
		++i;
		_delay_ms(200);
		PORTC &= ~(1 << CS); /* Chip select set to low for 74hc595 */
		master_transmit(data);
		PORTC |= (1 << CS);
		
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

void master_transmit(uint16_t d) {
	uint8_t upper = 0x0, lower = 0x0;
	uint8_t k;
	for(k = 0; k < 8; ++k) {
		upper <<= 1;
		if(d & 0x8000) {
			upper |= 1;
		}
		d <<= 1;
	}
	for(k = 0; k < 8; ++k) {
		lower <<= 1;
		if(d & 0x8000) {
			lower |= 1;
		}
		d <<= 1;
	}
	
	SPDR = upper; /* Writing to SPDR begins transmission */
	
	/* Polling loop, hence this is blocking/sequential code */
	// Issue with this: Interrupt occurs before while statement is read. It immediately detects the change in SPIF,
	// then changes it back to 0. So this polling loop is bad
	/*
	while(!(SPSR & (1 << SPIF)));
	*/
	
	/*
	while(!flag);
	flag = 0;
	*/
	
	while(!(SPSR & (1 << SPIF)));
	SPDR = lower;
	
	while(!(SPSR & (1 << SPIF)));
	
	/*
	while(!flag);
	flag = 0;
	*/
}

/*
ISR(SPI_STC_vect) {
	cli(); // mutual exclusion not really necessary since no other ISRs are used, but good practice
	flag = 1;
	if(i > 7) i = 0;
	data = 1 << i;
	++i;
	sei();
}
*/


