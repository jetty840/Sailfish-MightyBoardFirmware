/* VikiInterface
 *
 * This is an implementation of the communciation routines for the
 * "Visual Kinetic control Interface" or Viki.
 *
 * http://www.panucatt.com/product_p/vikilcd.htm
 * *
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
#include "ButtonArray.hh"

// Flags for Backlight Control.
#define LCD_BACKLIGHT_ACTIVE_HIGH
//#define LCD_BACKLIGHT_ACTIVE_LOW

// I2C Adress 0x27
#define VIKI_I2C_DEVICE_ADDRESS 0x20

// Pin mapings for the I2C Bus Extender
//(Mapping pins on the LCD to the pins on the bus extender)
#define B_LCD_EN_PIN 5
#define B_LCD_RW_PIN 6
#define B_LCD_RS_PIN 7
#define B_LCD_D4_PIN 4
#define B_LCD_D5_PIN 3
#define B_LCD_D6_PIN 2
#define B_LCD_D7_PIN 1

#define A_TOOL0_LED_PIN 7
#define A_TOOL1_LED_PIN 6
#define A_BUZZER_PIN 5
#define B_HBP_LED_PIN 0

// MCP23017 registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14

#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

// Button Mask
#define LEFT_BUTTON_MASK (1<<4)
#define UP_BUTTON_MASK (1<<3)
#define DOWN_BUTTON_MASK (1<<2)
#define RIGHT_BUTTON_MASK (1<<1)
#define CENTER_BUTTON_MASK (1<<0)

class VikiInterface : public LiquidCrystalSerial, public ButtonArray {

public:
  VikiInterface();

  // Both LCD and ButtonArray will call init()
  void init();

  // LCD Public routines
  bool hasI2CDisplay();
  void setToolLED(uint8_t toolID, bool state);
  void setHBPLED(bool state);
  void setBuzzer(bool state);
  
  // ButtonArray public routines
  void scanButtons();
  bool getButton(ButtonName &button);
  void clearButtonPress();
  bool isButtonPressed(ButtonArray::ButtonName button);
  void setButtonDelay(micros_t delay);
  
private:
  // LCD low-level private 
  void send(uint8_t, bool);
  void writeSerial(uint8_t);
  void write4bits(uint8_t value, bool dataMode);
  void pulseEnable(uint8_t value);

  bool send16Bits();
  bool getButtonRegister(uint8_t* buttons);
  bool has_i2c_lcd;
  uint16_t expander_bits;

};

#endif // STANDARD_LIQUID_CRYSTAL_HH
