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
 
 #include <util/twi.h>
 
 //TODO write proper error codes
uint8_t TWI_write_data(uint8_t address, uint8_t * data, uint8_t length){
 			
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if((TWSR & 0xF8) != TW_START)
	return 1;


  /* send address */
  TWDR = address | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
		return 2;


  for (int i = 0; i < length; i++)
    {
      TWDR = data[i];
      TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
			return 3;
    }

  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */
  
  return 0;
  
 }
  //TODO write proper error codes
uint8_t TWI_write_byte(uint8_t address, uint8_t data){
 
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if((TWSR & 0xF8) != TW_START)
	return 1;

  /* send address */
  TWDR = address | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
		return 2;

  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
		return 3;

  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */
  
  return 0;
  
 }
 
 void TWI_init(){

  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  TWSR = 0;

  uint32_t fCPU = 8000000;
  TWBR = (fCPU / 100000UL - 16) / 2;
 }
 
 //TODO write proper error codes
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
