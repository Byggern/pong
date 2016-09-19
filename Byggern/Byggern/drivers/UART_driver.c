#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

#include "UART_driver.h"

void UART0_init(uint32_t clock, uint32_t baud){
	uint32_t ubrr = (clock /(16 * baud)) - 1;
	UBRR0H = (unsigned char) (ubrr >> 8);
	UBRR0L = (unsigned char) (ubrr);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Set RX-enable and TX-enable bits
	UCSR0C = (1 << URSEL0) | (0b11 << UCSZ00) | (0b00 << UPM00) | (1 << USBS0); // ( UCSR0C register enable ) | ( 8-bit character size ) | ( disable parity ) | ( 2 stop bits )

	FILE * uart0_stream = fdevopen(UART0_put,NULL);
	stdout = uart0_stream;
}

void UART0_transmit(unsigned char c) {
	while (! (UCSR0A & (1 << UDRE0)) ) {
	}
	UDR0 = c;
}

int UART0_put( char c, FILE * out){
	UART0_transmit(c);
	return 0;
}