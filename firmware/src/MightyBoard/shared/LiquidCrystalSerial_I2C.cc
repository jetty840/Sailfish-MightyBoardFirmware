/* LiquidCrystalSerial_I2C
 *
 * This is an implementation of communciation routines for a
 * HD44780-based LCD display connected to an I2C bus expander/extender
 *
 * Bus extenders/expanders of this type may have their I/O pins connected
 * to the HD44780 in different ways depending on the implementation of
 * the hardare.  This mapping is done in LiquidCrystalSerial_I2C in the
 * preprocessor defintions, and should be modified as necessary for the
 * specific hardware in use.
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
#include "LiquidCrystalSerial_I2C.hh"
#include "Configuration.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "TWI.hh"

LiquidCrystalSerial_I2C::LiquidCrystalSerial_I2C() {
  has_i2c_lcd = false;
  TWI_init();
  init();
}

void LiquidCrystalSerial_I2C::init() {
  // We only support 4-bit mode
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  // Zero out all of the pins on the bus extender
  if (TWI_write_byte(LCD_I2C_DEVICE_ADDRESS << 1, 0) == 0) {
    // If we were successful in zeroing out the extender, then
    // odds are we have an I2C display connected.

    has_i2c_lcd = true;
    setBacklight(true);
  }
}

/*........... I2C Specific Stuff */

// Return true if we have an LCD connected
bool LiquidCrystalSerial_I2C::hasI2CDisplay() { return has_i2c_lcd; }

bool LiquidCrystalSerial_I2C::setBacklight(bool value) {
  // Store the backlight state for later
  backlight_state = value;

#ifdef LCD_BACKLIGHT_ACTIVE_HIGH
  uint8_t backlight_bits = (backlight_state ? 1 : 0) << LCD_BACKLIGHT_PIN;
#else
  uint8_t backlight_bits = (backlight_state ? 0 : 1) << LCD_BACKLIGHT_PIN;
#endif

  if (TWI_write_byte(LCD_I2C_DEVICE_ADDRESS << 1, backlight_bits))
    return true; // Error

  // Success
  return false;
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystalSerial_I2C::send(uint8_t value, bool dataMode) {
  write4bits((value >> 4), dataMode);
  write4bits((value & 0x0F), dataMode);
}

// write4bits
void LiquidCrystalSerial_I2C::write4bits(uint8_t value, bool dataMode) {
  uint8_t bits = 0;

  // Map in the data bits
  if (value & 0b00000001)
    bits |= (1 << LCD_D4_PIN);
  if (value & 0b00000010)
    bits |= (1 << LCD_D5_PIN);
  if (value & 0b00000100)
    bits |= (1 << LCD_D6_PIN);
  if (value & 0b00001000)
    bits |= (1 << LCD_D7_PIN);

  // Is it a command or data (register select)
  if (dataMode)
    bits |= (1 << LCD_RS_PIN);

#ifdef LCD_BACKLIGHT_ACTIVE_HIGH
  if (backlight_state)
    bits |= (1 << LCD_BACKLIGHT_PIN);
#else
  if (!backlight_state)
    bits |= (1 << LCD_BACKLIGHT_PIN);
#endif

  pulseEnable(bits);
}

void LiquidCrystalSerial_I2C::pulseEnable(uint8_t data) {
  TWI_write_byte(LCD_I2C_DEVICE_ADDRESS << 1, data | (1 << LCD_EN_PIN));
  TWI_write_byte(LCD_I2C_DEVICE_ADDRESS << 1, data & ~(1 << LCD_EN_PIN));
}

void LiquidCrystalSerial_I2C::writeSerial(uint8_t value) {
  TWI_write_byte(LCD_I2C_DEVICE_ADDRESS << 1, value);
}
