#ifndef TXSERIAL_H
#define TXSERIAL_H

#include <stdio.h>
#include <avr/io.h>

void USART_init();
void USART_putchar(unsigned char c);
void serial_write(char* string);

#endif
