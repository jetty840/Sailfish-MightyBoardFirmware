/* Copyright 2011 by Alison Leonard alison@makerbot.com
 * adapted for avr and MCP4018 digital i2c pot from:
 * Arduino SoftI2cMaster Library
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

#ifndef SOFT_I2C_MANAGER
#define SOFT_I2C_MANAGER

#include "Pin.hh"
#include "Configuration.hh"

// delay used to tweek signals
#define I2C_DELAY_USEC 4

// R/W direction bit to OR with address for start or restart
#define I2C_READ 1
#define I2C_WRITE 0

class SoftI2cManager {
private:
    
    static SoftI2cManager i2cManager;
public:
    SoftI2cManager();
    static SoftI2cManager& getI2cManager() {return i2cManager; }
    

  
  /** init bus */
  void init();
  
  /** read a byte and send Ack if last is false else Nak to terminate read */
  uint8_t read(bool last, const Pin &sdaPin);
  
  /** send new address and read/write bit without stop */
  uint8_t restart(uint8_t addressRW, const Pin &sdaPin);
  
  /** issue a start condition for i2c address with read/write bit */
  uint8_t start(uint8_t addressRW, const Pin &sdaPin);
  
  /** issue a stop condition */
  void stop(void);
  
  /** write byte and return true for Ack or false for Nak */
  bool write(uint8_t b, const Pin &sdaPin);
    
private:
    Pin sdaPins[MAX_STEPPERS];
    Pin sclPin;
    uint8_t numPins;
};
#endif //SOFT_I2C_MANAGER