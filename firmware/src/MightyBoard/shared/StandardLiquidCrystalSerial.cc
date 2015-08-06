/* StandardLiquidCrystalSerial
 *
 * This is an implementation of communciation routines for the
 * Makerbot OEM display hardware.
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

#include "Compat.hh"
#include "StandardLiquidCrystalSerial.hh"
#include "Configuration.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "TWI.hh"

//*** These functions are for and LCD using a shift register, the stock makerbot
// hardware.
StandardLiquidCrystalSerial::StandardLiquidCrystalSerial(Pin strobe, Pin data,
                                                         Pin CLK) {
  init(strobe, data, CLK);
}

void StandardLiquidCrystalSerial::init(Pin strobe, Pin data, Pin clk) {
  _strobe_pin = strobe;
  _data_pin = data;
  _clk_pin = clk;

  _strobe_pin.setDirection(true);
  _data_pin.setDirection(true);
  _clk_pin.setDirection(true);

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

  // begin(16, 1);
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void StandardLiquidCrystalSerial::send(uint8_t value, bool mode) {

  // serial assumes 4 bit mode
  // load((value & 0xF0) + modeBits);
  // load(((value << 4) & 0xF0) + modeBits);
  write4bits((value >> 4), mode);
  write4bits((value & 0x0F), mode);
}

void StandardLiquidCrystalSerial::write4bits(uint8_t value, bool dataMode) {
  // On the standard hardware, the top 4 bits are the data lines
  uint8_t bits = value << 4;

  // Is it a command or data (register select)
  // On standard hardware, the register select is 0b0010
  if (dataMode)
    bits |= 0b0010;

  // Send the data
  pulseEnable(bits);
}

void StandardLiquidCrystalSerial::pulseEnable(uint8_t value) {

  _delay_us(1);
  // set enable to true, on standard hardware it is 0b1000
  value |= 0b01000;
  writeSerial(value);
  _delay_us(1); // enable pulse must be >450ns
  // set enable to false
  value &= 0b11110111;
  writeSerial(value);
  _delay_us(1); // commands need > 37us to settle [citation needed]
}

void StandardLiquidCrystalSerial::writeSerial(uint8_t value) {

  for (int8_t i = 7; i >= 0; i--) {
    _clk_pin.setValue(false);
    bool data = (value >> i) & 0x01 ? true : false;
    _data_pin.setValue(data);

    _clk_pin.setValue(true);
    _delay_us(1);
  }

  _strobe_pin.setValue(true);
  _delay_us(1);
  _strobe_pin.setValue(false);
}
