/* LiquidCrystalSerial_I2C
 *
 * This is an implementation of communciation routines for a
 * HD44780-based LCD display connected to an I2C bus expander/extender
 *
 * Bus extenders/expanders of this type may have their I/O pins connected
 * to the HD44780 in different ways depending on the implementation of
 * the hardare.  This mapping is done in below  in the preprocessor
 * defintions, and should be modified as necessary for the specific
 * hardware in use.
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

#ifndef STANDARD_LIQUID_CRYSTAL_HH
#define STANDARD_LIQUID_CRYSTAL_HH

// TODO: Proper attribution

#include <stdint.h>
#include <avr/pgmspace.h>
#include "Pin.hh"
#include "LiquidCrystalSerial.hh"

// Flags for Backlight Control.
#define LCD_BACKLIGHT_ACTIVE_HIGH
//#define LCD_BACKLIGHT_ACTIVE_LOW

// I2C Adress 0x27
#define LCD_I2C_DEVICE_ADDRESS 0x27

// Pin mapings for the I2C Bus Extender
//(Mapping pins on the LCD to the pins on the bus extender)
#define LCD_BACKLIGHT_PIN 3
#define LCD_EN_PIN 2
#define LCD_RW_PIN 1
#define LCD_RS_PIN 0
#define LCD_D4_PIN 4
#define LCD_D5_PIN 5
#define LCD_D6_PIN 6
#define LCD_D7_PIN 7

class LiquidCrystalSerial_I2C : public LiquidCrystalSerial {

public:
  LiquidCrystalSerial_I2C();

  void init();

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  bool setBacklight(bool value);
  bool hasI2CDisplay();

private:
  void send(uint8_t, bool);
  void writeSerial(uint8_t);
  void write4bits(uint8_t value, bool dataMode);
  void pulseEnable(uint8_t value);

  bool has_i2c_lcd;
  bool backlight_state;
};

#endif // STANDARD_LIQUID_CRYSTAL_HH
