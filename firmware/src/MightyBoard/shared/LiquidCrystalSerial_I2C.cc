#include "LiquidCrystalSerial_I2C.hh"
#include "Configuration.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "TWI.hh"
// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystalSerial_I2C::LiquidCrystalSerial_I2C() {
  DEBUG_PIN.setValue(true);  
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