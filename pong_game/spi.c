/*
 * spi.c
 *
 * Author: Peter Sutton
 */ 

#include "spi.h"
#include <avr/io.h>

void spi_setup_master(uint8_t clockdivider) {
	// Set up SPI communication as a master
	// Make the SS, MOSI and SCK pins outputs. These are pins
	// 4, 5 and 7 of port B on the ATmega324A
	DDRB |= (1 << DDB7) | (1 << DDB5) | (1 << DDB4);
	
	// Set the slave select (SS) line high
	PORTB |= (1 << PORTB4);
	
	// Set up the SPI control registers SPCR and SPSR:
	// - SPE bit = 1 (SPI is enabled)
	// - MSTR bit = 1 (Master Mode)
	SPCR0 = (1 << SPE0) | (1 << MSTR0);
	
	// Set SPR0 and SPR1 bits in SPCR and SPI2X bit in SPSR
	// based on the given clock divider
	// Invalid values default to the slowest speed
	// We consider each bit in turn
	switch (clockdivider) {
		case 2: /* FALLTHROUGH */
		case 8: /* FALLTHROUGH */
		case 32:
			SPSR0 = (1 << SPI2X0);
			break;
		default:
			SPSR0 = 0;
			break;
	}
	switch (clockdivider) {
		case 128:
			SPCR0 |= (1 << SPR00);
			/* FALLTHROUGH */
		case 32: /* FALLTHROUGH */
		case 64:
			SPCR0 |= (1 << SPR10);
			break;
		case 8: /* FALLTHROUGH */
		case 16:
			SPCR0 |= (1 << SPR00);
			break;
	}
	
	// Take SS (slave select) line low
	PORTB &= ~(1 << PORTB4);
}

uint8_t spi_send_byte(uint8_t byte) {
	// Write out the byte to the SPDR0 register. This will initiate
	// the transfer. We then wait until the most significant byte of
	// SPSR0 (SPIF0 bit) is set - this indicates that the transfer is
	// complete. (The final read of SPSR0 followed by a read of SPDR0
	// will cause the SPIF bit to be reset to 0. See page 173 of the 
	// ATmega324A datasheet.)
	SPDR0 = byte;
	while ((SPSR0 & (1 << SPIF0)) == 0) {
		; // wait
	}
	return SPDR0;
}
