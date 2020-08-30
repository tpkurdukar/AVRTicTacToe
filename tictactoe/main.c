/*
 * tictactoe.c
 *
 * Created: 8/9/2020 8:56:22 PM
 * Author : prair
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "txSerial.h"
#include "tictactoefunctions.h"

#define PISO_DDR DDRC
#define PISO_PORT PORTC
#define DATA_IN PINC5
#define DATA_PORT PINC
#define LD PORTC3
#define PISO_CLK PORTC2
#define CLK_INH PORTC4

#define DS_PIN PORTD7
#define LATCH_PIN PORTD5
#define SIPO_CLK PORTD6
#define SIPO_PORT PORTD
#define SIPO_DDR DDRD

void shift_74HC595(uint16_t data);
uint16_t shift_74HC165(); 

/* line pulled from: https://www.sparkfun.com/tutorials/105 */
static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
int main(void)
{
	USART_init();
	stdout = &mystdout; //necessary to use printf
	
	char** game = init_board();
	unsigned char player = 'x', row, col;
	uint16_t new_bits = 0, old_bits = 0;
	uint8_t changed_pos, flag = 0; // flag is active when it is necessary to restore the board to a previous state
	uint16_t end_game_pattern = 0;
	
	/* initialize shift registers */
	PISO_DDR = (1 << LD) | (1 << PISO_CLK) | (1 << CLK_INH); // note DATA_IN is set as input
	SIPO_DDR = (1 << LATCH_PIN) | (1 << SIPO_CLK) | (1 << DS_PIN);
	shift_74HC595(0);
	while(shift_74HC165()) {
		printf("Clear the board.\r\n");
	}
	
    while (1) {
		if(flag) {
			// player disrupted the game board, does not proceed until they return to the previous state of the board
			if(new_bits == old_bits) {
				flag = 0;
			}
			new_bits = shift_74HC165();
			_delay_ms(200);
			continue;
		} else {
			// display information on terminal if game running normally
			display_board(game);
			printf("\r\nCurrent player: %c\r\n", player);
			printf("Waiting for input...\r\n");
		}
		
		while(new_bits == old_bits) {
			new_bits = shift_74HC165(); //load new bits
			_delay_ms(200);
		}
		
		changed_pos = find_changed_bit(old_bits, new_bits); // locate the changed bit
		
		flag = 1;
		switch(changed_pos) {
			case 0x0A:
				printf("Space already occupied.\r\n");
				continue;
			case 0x0B:
				printf("Player put two or more pieces on at once. Remove those pieces and try again\r\n");
				continue;
			case 0x0C:
				printf("Player removed a piece. Put it back bruh\r\n");
				continue;
			default:
				flag = 0;
		}
		
		_delay_ms(200);
		shift_74HC595((1 << changed_pos));
		_delay_ms(500);
		shift_74HC595(0);
		
		row = changed_pos / 3;
		col = changed_pos % 3;
		// update board, previously stored bits
		update_board(game, player, row, col);
		old_bits = new_bits;
		
		uint16_t victory = check_victory(game, player);
		
		if(victory) {
			printf("%c wins. Press clear the board\r\n", player);	
			end_game_pattern = victory;
		} else if(!(0x01FF - (new_bits & 0x01FF))) {
			printf("Draw. Please clear the board\r\n.");
			end_game_pattern = 0x01FF;
		}
		
		// wait for all inputs to zero
		if(end_game_pattern) {
			display_board(game);
			while(shift_74HC165()) {
				shift_74HC595(end_game_pattern);
				_delay_ms(300);
				shift_74HC595(0);
				_delay_ms(300);
			}
					
			player = 'x';
			remake_board(game);
			old_bits = 0, new_bits = 0;
			continue;
		}

		(player == 'x') ? (player = 'o') : (player = 'x');
		
    }
}



/* Shifts in data LSB first */
uint16_t shift_74HC165() {
	
	// use LD pin to take snapshot of current state
	PISO_PORT &= ~(1 << LD); // parallel ld pin is active low
	_delay_us(50);
	PISO_PORT |= (1 << LD);
	_delay_us(50);
	
	PISO_PORT &= ~(1 << CLK_INH); // clear CLK and set inhibit to low
	uint16_t data = 0;
	for(uint16_t i = 0; i < 16; i++) {
		
		data <<= 1;	
		// if 1 in QH
		if(DATA_PORT & (1 << DATA_IN)) {
			data |= 1;
		}
		
		PISO_PORT |= (1 << PISO_CLK); // pulse clk signal
		_delay_us(50);
		PISO_PORT &= ~(1 << PISO_CLK);
		_delay_us(50);
	} 
	
	PISO_PORT |= (1 << CLK_INH); // set clk inhibit high
	return data;
}

void shift_74HC595(uint16_t data) {
	SIPO_PORT &= ~(1 << LATCH_PIN); // set latch low
	
	for(uint16_t i = 0; i < 16; i++) {
		
		/* Consider data byte as QhQgQfQeQdQcQbQa
		 Take the MSB of the data byte and send it to Qa in the shift reg.
		 Then, pulse the shift reg's clock to cascade the data through.
		 Finally, shift data byte left to check the next bit 
		 Repeat until data byte is exhausted */
		
		if(data & 0b1000000000000000) {
			SIPO_PORT |= (1 << DS_PIN); // send 1 to Qa
		} else {
			SIPO_PORT &= ~(1 << DS_PIN); // send 0 to Qa
		}
		
		data <<= 1;
		
		SIPO_PORT |= (1 << SIPO_CLK); // rise clk
		_delay_us(50);
		SIPO_PORT &= ~(1 << SIPO_CLK); // drop clk signal
		_delay_us(50);
	}
	
	SIPO_PORT |= (1 << LATCH_PIN); // display output
}


