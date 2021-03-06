#include <stdlib.h>
#include <stdio.h>
#define F_CPU 4912000UL
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "../drivers/EXTMEM_driver.h"

const unsigned char sram_test[] PROGMEM = "SRAM test completed with\n %d errors in write phase (%d upper, %d lower) and\n %d errors in retrieval phase (%d upper, %d lower)\n";

uint16_t SRAM_test(void) {
	volatile char* ext_ram = (char*) 0x1800; // Start address for the SRAM
	uint16_t ext_ram_size = 0x800;
	uint16_t write_errors = 0;
	uint16_t retrieval_errors = 0;
	uint16_t upper_retrieval_errors	= 0;
	uint16_t lower_retrieval_errors	= 0;
	uint16_t upper_write_errors	= 0;
	uint16_t lower_write_errors	= 0;
	printf("Starting SRAM test... %d bytes \n", ext_ram_size);
	// rand() stores some internal state, so calling this function in a loop will
	// yield different seeds each time (unless srand() is called before this function)
	uint16_t seed = rand();
	// Write phase: Immediately check that the correct value was stored
	srand(seed);
	for (uint16_t i = 0; i < ext_ram_size; i++) {
		uint8_t some_value = rand();
		ext_ram[i] = some_value;
		uint8_t retreived_value = ext_ram[i];
		if (retreived_value != some_value) {
			write_errors++;
			if (i < ext_ram_size / 2) {
				lower_write_errors++;
			} else {
				upper_write_errors++;
			}
		}
	}
	// Retrieval phase: Check that no values were changed during or after the write	phase
	srand(seed);
	// reset the PRNG to the state it had before the write phase
	for (uint16_t i = 0; i < ext_ram_size; i++) {
		uint8_t some_value = rand();
		uint8_t retreived_value = ext_ram[i];
		if (retreived_value != some_value) {
			retrieval_errors++;
			if (i < ext_ram_size / 2) {
				lower_retrieval_errors++;
			} else {
				upper_retrieval_errors++;
			}
		}
	}
	printf_P(sram_test, write_errors, upper_write_errors, lower_write_errors, retrieval_errors, upper_retrieval_errors, lower_retrieval_errors);
	return retrieval_errors + write_errors;
}

const void* EXTMEM_start = (const void*) 0x1800;
size_t EXTMEM_size = 0x0400;
static void* EXTMEM_next = (void*) 0x1800;

void* SRAM_allocate(size_t size) {
	if (EXTMEM_next + size > EXTMEM_start + EXTMEM_size) {
		printf("Out of memory!\r\n");
		return NULL;
	}else{
		void* mem = EXTMEM_next;
		EXTMEM_next += size;
		return mem;
	}
}