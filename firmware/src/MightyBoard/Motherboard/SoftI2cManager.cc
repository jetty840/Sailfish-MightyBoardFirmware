/* Copyright 2011 by Alison Leonard alison@makerbot.com
 * adapted for avr and MCP4018 digital i2c pot from:
 * Arduino SoftI2cManager Library
 * Copyright (C) 2009 by William Greiman
 *
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

#include "SoftI2cManager.hh"
#include <util/delay.h>
#include <util/atomic.h>

// initiate static i2cManager instance
SoftI2cManager SoftI2cManager::i2cManager;

// constructor
SoftI2cManager::SoftI2cManager():
    numPins(STEPPER_COUNT),
    sclPin(POTS_SCL)
{
    sdaPins[0] = X_POT_PIN;
    sdaPins[1] = Y_POT_PIN;
    sdaPins[2] = Z_POT_PIN;
    sdaPins[3] = A_POT_PIN;
    sdaPins[4] = B_POT_PIN;
    
}


// init pins and set bus high
void SoftI2cManager::init()
{
    for (uint8_t i = 0; i < numPins; i++)
    {
        sdaPins[i].setDirection(true);
        sdaPins[i].setValue(true);
    }
    sclPin.setDirection(true);
    sclPin.setValue(true);
    
}

//------------------------------------------------------------------------------
// read a byte and send Ack if last is false else Nak to terminate read
uint8_t SoftI2cManager::read(bool last, const Pin &sdaPin)
{
  uint8_t b = 0;
  // make sure pullup enabled
  sdaPin.setValue(true);
  sdaPin.setDirection(false);
  // read byte
  for (uint8_t i = 0; i < 8; i++) {
    // don't change this loop unless you verify the change with a scope
    b <<= 1;
    _delay_us(I2C_DELAY_USEC);
    sclPin.setValue(true);
    if (sdaPin.getValue()) b |= 1;
    sclPin.setValue(false);
  }
  // send Ack or Nak
  sdaPin.setDirection(true);
  sdaPin.setValue(last);
  sclPin.setValue(true);
  _delay_us(I2C_DELAY_USEC);
  sclPin.setValue(false);
  sdaPin.setValue(true);
    
  return b;
}
//------------------------------------------------------------------------------
// send new address and read/write without stop
uint8_t SoftI2cManager::restart(uint8_t addressRW, const Pin &sdaPin)
{
  sclPin.setValue(true);
  return start(addressRW, sdaPin);
}
//------------------------------------------------------------------------------
// issue a start condition for i2c address with read/write bit
uint8_t SoftI2cManager::start(uint8_t addressRW, const Pin &sdaPin)
{
    for(uint8_t i = 0; i < numPins; i++)
        sdaPins[i].setValue(false);
  _delay_us(I2C_DELAY_USEC);
  sclPin.setValue(false);
  return write(addressRW, sdaPin);
}
//------------------------------------------------------------------------------
// issue a stop condition
void SoftI2cManager::stop()
{
  _delay_us(I2C_DELAY_USEC);
   sclPin.setValue(true);  
  _delay_us(I2C_DELAY_USEC);
    for(uint8_t i = 0; i < numPins; i++)
        sdaPins[i].setValue(true);  
  _delay_us(I2C_DELAY_USEC);
}
//------------------------------------------------------------------------------
// write byte and return true for Ack or false for Nak
bool SoftI2cManager::write(uint8_t b, const Pin &sdaPin)
{
  // write byte
  for (uint8_t m = 0X80; m != 0; m >>= 1) {
    // don't change this loop unless you verivy the change with a scope
     sdaPin.setValue((m & b) != 0);
     sclPin.setValue(true);  
    _delay_us(I2C_DELAY_USEC);
     sclPin.setValue(false);  
  }
  // get Ack or Nak
   sdaPin.setValue(true);  
   sdaPin.setDirection(false); 
   sclPin.setValue(true);  
   b = sdaPin.getValue();
   sclPin.setValue(false); 
   sdaPin.setDirection(true);
   return b == 0;
}
