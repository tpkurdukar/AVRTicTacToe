
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
#define CS PORTC1 /* Connected to CLK INH on the 74hc165*/
#define LD PORTC2 /* No simple way to be able to control LD pin via SPI, so just using an extra GPIO pin*/
#define MOSI PORTB3 /* Not connected to 74hc165 */
#define MISO PORTB4 /* Connected to Qh on 74hc165 */
#define SCK PORTB5 /* Connected to clock on 74hc165 */
/* No definition for MISO because it is not used */


uint8_t data = 0x0;

void master_transmit(uint16_t d);
void master_receive(void);

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
	SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1); /* Enable SPI interrupts, enable SPI, set as master */
	/* Note: CPOL and CPHA set so that the leading edge of SCK is the rising edge, and sampling occurs on falling edge */
	/* Using SCK frequency of F_CPU / 128 */
	
	DDRC = (1 << CS); /* CS for 74hc165 is on DDRC. Just using a GPIO pin here */
	DDRB |= (1 << MOSI) | (1 << SCK); /* SPI is on DDRB. MISO is configured as input */
	
	PORTC &= ~(1 << CS);
	
    while (1) 
    {
		
		//PORTC &= ~(1 << CS); /* Chip select set to low for 74hc165 */
		_delay_ms(100);
		master_receive();
		//PORTC |= (1 << CS);
		//printf("%d\r\n", data);
		
		/* Main problem: The interface cannot, for whatever reason, detect button presses on 74hc165.
		 * However, it does function as expected if pins on 74hc165 are directly set high. 
		 * Possible issues: 
		 * - Something with the way data is being sampled, though I do not think this is the case because I tried all four CPHA/CPOL configurations
		 * - Clock speed with SPI is too fast for 74hc165. I also find this unlikely because I have added delays and set the speed as slow as possible.
		 * - The 74hc165 is not really SPI compliant... the data output that connects to MISO is not tristate and is always active - may be an issue with the programmer on the same line
		 * Overall, direct bit banging was far less of a hassle and was a simpler approach.
		 */
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

void master_receive(void) {
	// Remember that SPI is full duplex. Even though we don't need to transmit real information to 74hc165, 
	// we should send some dummy data so that we can read from MISO.
	
	PORTC &= ~(1 << LD);
	_delay_ms(10);
	PORTC |= (1 << LD);
	
	SPDR = 0x1; // Send some dummy data out
	while(!(SPSR & (1 << SPIF))) {
		//printf("test");
		_delay_ms(1);
	}
	printf("%d\r\nSPDR is ", SPDR);
	data = SPDR; // Send back 
	
}




