#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "CAN_driver.h"
#include "MCP2515_driver.h"
uint8_t test_data = 0b10100101;

const char canerror[] PROGMEM = "CAN interrupt due to error\n";

bool message_received = false;
uint8_t intr_recv_buf[8] = {0,0,0,0,0,0,0,0};
CAN_MESSAGE CAN_receive_buf={.data=intr_recv_buf};

#define RX0 0b110
#define RX1 0b111
#define ERROR 0b001

CAN_MESSAGE test_message = {
	.id = 0,
	.length = 1,
	.data = &test_data
};

void CAN_init(void) {
	MCP_init();
	
	//enable interrupts
	sei();
	DDRE &= ~(1 << PE0);
	GICR |=  (1 << INT2);
	EMCUCR &= ~(1 << ISC2);
	
	//disable receive masks and filters on both receive buffers
	MCP_bit_modify(RXBnCTRL + RXB0_OFFSET,5,1);
	MCP_bit_modify(RXBnCTRL + RXB0_OFFSET,6,1);
	MCP_bit_modify(RXBnCTRL + RXB1_OFFSET,5,1);
	MCP_bit_modify(RXBnCTRL + RXB1_OFFSET,6,1);
	
	//set identifier for receive buffers
	MCP_write(RXBnSIDH + RXB0_OFFSET,0);
	MCP_write(RXBnSIDL + RXB0_OFFSET,0);
	MCP_write(RXBnSIDH + RXB1_OFFSET,0);
	MCP_write(RXBnSIDL + RXB1_OFFSET,0);
}

void CAN_loopback_init(void)
{
	MCP_bit_modify(CANCTRL,5,0);
	MCP_bit_modify(CANCTRL,6,1);
	MCP_bit_modify(CANCTRL,7,0);
}

void CAN_send_message(uint8_t id, uint8_t buffer, CAN_MESSAGE * message) {
	uint8_t buf_offset = TXB0_OFFSET; // Buffer offset TODO: Make flexible buf_offset = buffer etc..
	
	// Check if TX buffer is ready 
	while(MCP_read(TXBnCTRL + buf_offset) & 0b1000)
	{
		// TXB currently pending transmission
	}
	
	// Set message ID to be transmitted 
	MCP_write(TXBnSIDH + buf_offset, (id >> 3));
	MCP_write(TXBnSIDL + buf_offset, (id << 5));
	
	MCP_write(TXBnDLC+buf_offset, message->length & 0b1111);
	
	// Write in data
	for (uint8_t i = 0; i < message->length; i++) {
		uint8_t data_reg = TXBnDm + buf_offset + i;
		//uint8_t data_reg = ((0b0011 + buffer) << 4) | (0b0110 + i);
		MCP_write(data_reg, message->data[i]);
	}
	
	MCP_request_to_send(buffer);
	
}


ISR(INT2_vect){
	//printf("ISR\n");	
	int lambda = (MCP_read(0xe) & 0b1110)>>1;
	//if first buffer
	switch(lambda){
		case RX0:
			CAN_receive_message(RXBnDLC+RXB0_OFFSET, &CAN_receive_buf);
			MCP_bit_modify(CANINTF, 0, 0);
			break;
		case RX1:
		CAN_receive_message(RXBnDLC+RXB1_OFFSET, &CAN_receive_buf);
			MCP_bit_modify(CANINTF, 1, 0);
			break;
		case ERROR:
			printf_P(canerror);
			MCP_bit_modify(CANINTF, 5, 0);
			break;
		default:
			printf_P(canerror);
			MCP_write(CANINTF, 0);
	}
	
}


void CAN_receive_message( uint8_t messageaddr, CAN_MESSAGE * message)
{
	message->length = MCP_read(messageaddr) & 0b1111;
	
	for (int i = 0 ; i < message->length; i++)
	{
		message->data[i]=MCP_read(messageaddr + 1 + i);
	}
	message_received=true;
}