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

#include "VikiInterface.hh"
#include "Configuration.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "TWI.hh"

static bool initializationComplete = false;
static micros_t ButtonDelay;
static uint8_t previousButtons; ///< state of the button pins from the previous
/// scan
static uint8_t packet[4];

#define A_BUTTONS_MASK 0x1F

VikiInterface::VikiInterface() {
  has_i2c_lcd = false;
  expander_bits = 0;
  previousButtons = A_BUTTONS_MASK;
  TWI_init();
  init();
}

void VikiInterface::init() {
  // Only initialize once.  Due to multiple inheritance, its possible
  // that multiple people will request an initialization.
  if (initializationComplete)
    return;

  // We only support 4-bit mode
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  // Zero out all of the pins on the bus extender

  // Configure the extender inputs and outputs per the extender
  packet[0] = MCP23017_IODIRA;
  packet[1] = A_BUTTONS_MASK;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  // Set pullups for the butons
  packet[0] = MCP23017_GPPUA;
  packet[1] = A_BUTTONS_MASK;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  // Set direction for the extender port B
  packet[0] = MCP23017_IODIRB;
  packet[1] = 0x00;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  packet[0] = MCP23017_GPIOA;
  packet[1] = 0; // A
  packet[2] = 0; // B
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 3))
    return;

  // Turn off the LEDs.  They default on, so they will briefly blink
  // during the initialization process.
  setToolLED(0,false);
  setToolLED(1,false);
  setHBPLED(false);
    
  has_i2c_lcd = true;
  initializationComplete = true;
}

/*........... Viki Specific Stuff */
bool VikiInterface::send16Bits() {
  packet[0] = MCP23017_GPIOA;
  packet[1] = (expander_bits & 0xFF); // A
  packet[2] = (expander_bits >> 8);   // B
  return TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 3);
}

void VikiInterface::setToolLED(uint8_t toolID, bool state) {
  uint8_t pin = toolID ? (A_TOOL1_LED_PIN) : (A_TOOL0_LED_PIN);
  if (!state) {
    expander_bits |= (1 << pin);
  } else {
    expander_bits &= ~(1 << pin);
  }
  send16Bits();
}

void VikiInterface::setHBPLED(bool state) {
  if (!state) {
    expander_bits |= (1 << (B_HBP_LED_PIN + 8));
  } else {
    expander_bits &= ~(1 << (B_HBP_LED_PIN + 8));
  }
  send16Bits();
}

void VikiInterface::setBuzzer(bool state) {
  if (state) {
    expander_bits |= (1 << (A_BUZZER_PIN));
  } else {
    expander_bits &= ~(1 << (A_BUZZER_PIN));
  }
  send16Bits();
}

// Return true if we have an LCD connected
bool VikiInterface::hasI2CDisplay() { return has_i2c_lcd; }

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void VikiInterface::send(uint8_t value, bool dataMode) {
  write4bits((value >> 4), dataMode);
  write4bits((value & 0x0F), dataMode);
}

// write4bits
void VikiInterface::write4bits(uint8_t value, bool dataMode) {
  uint8_t bits = 0;

  // Map in the data bits
  if (value & 0b00000001)
    bits |= (1 << B_LCD_D4_PIN);
  if (value & 0b00000010)
    bits |= (1 << B_LCD_D5_PIN);
  if (value & 0b00000100)
    bits |= (1 << B_LCD_D6_PIN);
  if (value & 0b00001000)
    bits |= (1 << B_LCD_D7_PIN);

  // Is it a command or data (register select)
  if (dataMode)
    bits |= (1 << B_LCD_RS_PIN);

  pulseEnable(bits);
}

void VikiInterface::pulseEnable(uint8_t data) {
  // Send only 8-bits, the B-register, since all of the LCD pins exist on
  // the expander's PORTB.  Get BIT0 from our expander_bits state because
  // it is not involved with LCD control and should not be changed during
  // this sequence.
  uint8_t byte = data | (1 << B_LCD_EN_PIN);
  writeSerial(byte);
  byte &= ~(1 << B_LCD_EN_PIN);
  writeSerial(byte);
}

void VikiInterface::writeSerial(uint8_t value) {
  // Get BIT0 from our expander_bits state because it is not involved with
  // LCD control and should not be changed during this sequence.
  packet[0] = (MCP23017_GPIOB);
  packet[1] = value | ((expander_bits >> 8) & 0x01);
  TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2);
}

/*********** ButtonArray implementation *******/

bool VikiInterface::getButtonRegister(uint8_t* buttons) {
  packet[0] = (MCP23017_GPIOA);
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 1)) {
    return true;
  }

  if (TWI_read_byte(VIKI_I2C_DEVICE_ADDRESS << 1, buttons, 1)) {
    return true;
  }

  // Mask out the relevant bits
  *buttons &= A_BUTTONS_MASK;
  return false;
}

void VikiInterface::scanButtons() {
  if (buttonPressWaiting ||
      (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
    return;

  uint8_t newButtons = 0;
  
  // Get the buttons, return if there is an error.
  if (getButtonRegister(&newButtons)) return;
  
  buttonTimeout.clear();

  if (newButtons != previousButtons) {
    uint8_t diff = newButtons ^ previousButtons;
    if (!buttonPressWaiting && diff) {
      if ((diff & UP_BUTTON_MASK) && !(newButtons & UP_BUTTON_MASK)) {
        buttonPress = UP;
      } else if ((diff & DOWN_BUTTON_MASK) && !(newButtons & DOWN_BUTTON_MASK)) {
        buttonPress = DOWN;
      } else if ((diff & RIGHT_BUTTON_MASK) && !(newButtons & RIGHT_BUTTON_MASK)) {
        buttonPress = RIGHT;
      } else if ((diff & LEFT_BUTTON_MASK) && !(newButtons & LEFT_BUTTON_MASK)) {
        buttonPress = LEFT;
      } else if ((diff & CENTER_BUTTON_MASK) && !(newButtons & CENTER_BUTTON_MASK)) {
        buttonPress = CENTER;
      } else {
        // we didn't find a new button press, so exit without setting
        // buttonPressWaiting and starting the buttonTimeout.
        goto exitScanButtons;
      }
      buttonPressWaiting = true;
      buttonTimeout.start(ButtonDelay);
    }
  }

exitScanButtons:
  previousButtons = newButtons;
}

bool VikiInterface::getButton(ButtonName &button) {
  bool buttonValid;
  uint8_t buttonNumber;

  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    buttonValid = buttonPressWaiting;
    buttonNumber = buttonPress;
    buttonPressWaiting = false;
  }

  if (buttonValid) {
    button = (ButtonName)(buttonNumber);
  }

  return buttonValid;
}

void VikiInterface::clearButtonPress() { previousButtons = A_BUTTONS_MASK; }

bool VikiInterface::isButtonPressed(ButtonArray::ButtonName button) {
  uint8_t buttons = 0;
  
  // Get the buttons, return if there is an error
  if (getButtonRegister(&buttons)) return false;
    
  // Buttons are active low
  switch (button) {
  case CENTER:
    if (buttons & CENTER_BUTTON_MASK)
      return false;
    break;
  case RIGHT:
    if (buttons & RIGHT_BUTTON_MASK)
      return false;
    break;
  case LEFT:
    if (buttons & LEFT_BUTTON_MASK)
      return false;
    break;
  case DOWN:
    if (buttons & DOWN_BUTTON_MASK)
      return false;
    break;
  case UP:
    if (buttons & UP_BUTTON_MASK)
      return false;
    break;
  }
  return true;
}

void VikiInterface::setButtonDelay(micros_t delay) { ButtonDelay = delay; }
