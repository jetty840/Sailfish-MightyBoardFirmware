/*
 * Copyright 2011 by Alison Leonard	 <alison@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
 
// Based on I2C master code by Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
#include <util/twi.h>
#include "TWI.hh"

static bool twi_init_complete = false;

void TWI_init() {
	// If we've already done this, return.
	
	if (twi_init_complete)
		return;

	// If running on a board without pullups for debugging purposes
	//PORTD|=0b11;

	/* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
	
	TWSR = 0;                         /* no prescaler */
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

	// Set the flag
	twi_init_complete = true;
}

//TODO write proper error codes
uint8_t TWI_write_data(uint8_t address, uint8_t * data, uint8_t length) {
	uint8_t   twst;
	uint8_t   err = 0;
	
	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address | TW_WRITE;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst == TW_MT_SLA_ACK) || (twst == TW_MR_SLA_ACK) ) {
	
		// SEND DATA
		for (int i = 0; i < length; i++) {
			TWDR = data[i];
			TWCR = (1<<TWINT) | (1<<TWEN);

			// wait until transmission completed
			while(!(TWCR & (1<<TWINT)));

			// check value of TWI Status Register. Mask prescaler bits
			twst = TW_STATUS & 0xF8;
			if( twst != TW_MT_DATA_ACK ) { err = 3; }
	    }
		
	} else { err = 2; }
	
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));
  
	return err;
  
}
  //TODO write proper error codes
uint8_t TWI_write_byte(uint8_t address, uint8_t data){
	uint8_t   twst;
	uint8_t   err = 0;
		
	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address | TW_WRITE;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst == TW_MT_SLA_ACK) || (twst == TW_MR_SLA_ACK) ) {
		
		// Send the data
		TWDR = data;
		TWCR = (1<<TWINT) | (1<<TWEN);

		// wait until transmission completed
		while(!(TWCR & (1<<TWINT)));

		// check value of TWI Status Register. Mask prescaler bits
		twst = TW_STATUS & 0xF8;
		if( twst != TW_MT_DATA_ACK ) { err = 3; }

	} else { err = 2; }
	
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

	return err;
  
}
 
 
//TODO write proper error codes
// This is the original Makerbot read function, not modified
// read function is Totally untested
uint8_t TWI_read_byte(uint8_t address, uint8_t * data, uint8_t length){
 
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if((TWSR & 0xF8) != TW_START)
	return 1;
	
  /* send address */
   TWDR = address | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
   if((TWSR & 0xF8) != TW_MR_SLA_ACK)
	return 2;
    
    uint8_t twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
    /* send data bytes */
    for (int i=0; i < length; i++){
	 // if last byte send NAK
      if (i == length - 1)
			twcr = _BV(TWINT) | _BV(TWEN); /* send NAK this time */
      
      TWCR = twcr;		/* clear int to start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      if((TWSR & 0xF8) != TW_MR_DATA_ACK)
		return 3;
	  else
		  data[i] = TWDR;
    }
 
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return 0;

 }
