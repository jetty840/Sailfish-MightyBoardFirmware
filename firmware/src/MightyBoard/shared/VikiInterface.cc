/* VikiInterface
 *
 * This is an implementation of the communciation routines for the
 * "Visual Kinetic control Interface" or Viki.
 *
 * http://www.panucatt.com/product_p/vikilcd.htm
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

#include "Compat.hh"
#include "Configuration.hh"
#include "VikiInterface.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "TWI.hh"

static bool initializationComplete = false;
static micros_t ButtonDelay;
static uint8_t previousButtons; ///< state of the button pins from the previous

#define A_BUTTONS_MASK 0x1F

// Encoder support
#if defined(VIKI_ENC_PIN_A) && defined(VIKI_ENC_PIN_B)
static int8_t encClicks, encDir, encTurning;
#endif

VikiInterface::VikiInterface() {
  has_i2c_lcd = false;

  // Three status LEDs are controlled via a MCP23017 port expander
  //   This uses three bits split across the two ports, A and B, on
  //   the expander.  To change the state of one of the LEDs, we
  //   need to re-assert the state for all the LEDs on a port.
  //   Consequently, we need to save three bits of state.  We go
  //   ahead and layout these three bits the same way they are laid
  //   out on the two ports.  Thus, we use two bytes of storage to
  //   save these three bits.

  expander_bits[0] = (1 << (A_TOOL0_LED_PIN)) | // Tool 0 LED off
                     (1 << (A_TOOL1_LED_PIN));  // Tool 1 LED off
  expander_bits[1] = (1 << (B_HBP_LED_PIN));    // HBP LED off

  // Button state
  previousButtons = A_BUTTONS_MASK;

#if defined(VIKI_ENC_PIN_A) && defined(VIKI_ENC_PIN_B)
  // Encoder state
  encDir = 0;
  encTurning = 0;
  encClicks = 0;
#endif

  // Initialize the I2C bus
  TWI_init();

  // Further initializations of the VikiInterface
  init();
}

void VikiInterface::init() {

  // Only initialize once.  Due to multiple inheritance, its possible
  // that multiple people will request an initialization.
  if (initializationComplete)
    return;

#if defined(VIKI_ENC_PIN_A) && defined(VIKI_ENC_PIN_B)
  // Configure encoder GPIO pins
  // VIKI_ENC_PIN_A.setDirection(false);
  // VIKI_ENC_PIN_A.setValue(true);
  // VIKI_ENC_PIN_B.setDirection(false);
  // VIKI_ENC_PIN_B.setValue(true);
  VIKI_ENC_DDR_A  &= ~(VIKI_ENC_MASK_A); // Set pin as input
  VIKI_ENC_PORT_A |=   VIKI_ENC_MASK_A;  // Enable pullup
  VIKI_ENC_DDR_B  &= ~(VIKI_ENC_MASK_B); // Set pin as input
  VIKI_ENC_PORT_B |=   VIKI_ENC_MASK_B;  // Enable pullup
#endif

  // We only support 4-bit mode
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

  // Configure the port extender inputs and outputs
  uint8_t packet[3];

  // I/O direction for the extender port A
  packet[0] = MCP23017_IODIRA;
  packet[1] = A_BUTTONS_MASK;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  // Set pullups for the buttons
  packet[0] = MCP23017_GPPUA;
  packet[1] = A_BUTTONS_MASK;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  // I/O direction for the extender port B
  packet[0] = MCP23017_IODIRB;
  packet[1] = 0x00;
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2))
    return;

  // Set the LED states (default states are all OFF)
  //   expander_bits[] set in our object initializer
  packet[0] = MCP23017_GPIOA;
  packet[1] = expander_bits[0];
  packet[2] = expander_bits[1];
  if (TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 3))
    return;

  has_i2c_lcd = true;
  initializationComplete = true;
}

/*........... Viki Specific Stuff */
bool VikiInterface::writePortAB() {
  uint8_t packet[3];
  packet[0] = MCP23017_GPIOA;
  packet[1] = expander_bits[0];
  packet[2] = expander_bits[1];
  return TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 3);
}

void VikiInterface::setToolIndicator(uint8_t toolID, bool state) {
  uint8_t pin = toolID ? (A_TOOL1_LED_PIN) : (A_TOOL0_LED_PIN);
  if (!state) {
    expander_bits[0] |= (1 << pin);
  } else {
    expander_bits[0] &= ~(1 << pin);
  }
  // This can be done just writing port A
  writePortAB();
}

void VikiInterface::setHBPIndicator(bool state) {
  if (!state) {
    expander_bits[1] |= (1 << (B_HBP_LED_PIN));
  } else {
    expander_bits[1] &= ~(1 << (B_HBP_LED_PIN));
  }
  // This can be done just writing port B
  writePortAB();
}

void VikiInterface::setBuzzer(bool state) {
  if (state) {
    expander_bits[0] |= (1 << (A_BUZZER_PIN));
  } else {
    expander_bits[0] &= ~(1 << (A_BUZZER_PIN));
  }
  // This can be done just writing port A
  writePortAB();
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void VikiInterface::send(uint8_t value, bool dataMode) {
  write4bits((value >> 4), dataMode);
  write4bits((value & 0x0F), dataMode);
}

// write4bits
void VikiInterface::write4bits(uint8_t value, bool dataMode) {

  // Send 4-bits to the B-register, since all of the LCD pins exist on
  // the expander's PORTB.  Get BIT0 from our expander_bits[] state because
  // it is not involved with LCD control and should not be changed during
  // this sequence.  (We cheat and just initialize with all 8 bits from
  // expander_bits[1] because we know that only the lowest bit in it is
  // is ever non-zero.

  uint8_t bits = expander_bits[1];

  // Map in the data bits
  if (value & 0b00000001) bits |= (1 << B_LCD_D4_PIN);
  if (value & 0b00000010) bits |= (1 << B_LCD_D5_PIN);
  if (value & 0b00000100) bits |= (1 << B_LCD_D6_PIN);
  if (value & 0b00001000) bits |= (1 << B_LCD_D7_PIN);

  // Is it a command or data (register select)
  if (dataMode) bits |= (1 << B_LCD_RS_PIN);

  pulseEnable(bits);
}

void VikiInterface::pulseEnable(uint8_t data) {
  uint8_t packet[2];

  // Write once with the LCD's enable pin held HIGH
  packet[0] = MCP23017_GPIOB;
  packet[1] = data | (1 << (B_LCD_EN_PIN));
  TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2);

  // Write again with the LCD's enable pin pulled LOW
  packet[1] &= ~(1 << (B_LCD_EN_PIN));
  TWI_write_data(VIKI_I2C_DEVICE_ADDRESS << 1, packet, 2);
}

void VikiInterface::writeSerial(uint8_t value) {
  // Note that our super will call writeSerial(0x00) on us...
  send(value, false);
}

/*********** ButtonArray implementation *******/

bool VikiInterface::getButtonRegister(uint8_t* buttons) {
  uint8_t packet[1];

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

#if defined(VIKI_ENC_PIN_A) && defined(VIKI_ENC_PIN_B)
  // Encoder
  bool encA = (VIKI_ENC_PIN_A & VIKI_ENC_MASK_A) == 0;
  bool encB = (VIKI_ENC_PIN_B & VIKI_ENC_MASK_B) == 0;
  if ( encA && encB ) {
       // We are at a detent
       if ( encTurning ) {
	    // We were turning AND this wasn't a bounce: accumulate it
	    encClicks += encDir;
	    encTurning = 0;
       }
       // Wipe this info out.  If we merely registered a bounce,
       // then encTurning was not true and so we didn't accumulate a turn.
       encDir = 0;
  }
  else if ( !encA && !encB ) {
       // We're at the half way point
       // This means that we're turning and isn't just bounce
       encTurning = 1;
  }
  else if ( !encTurning ) {
       // This is a bounce OR we've started to turn
       // Note direction
       encDir = encA ? 1 : -1;
  }
#endif

  // Get the buttons, return if there is an error.
  if (getButtonRegister(&newButtons)) return;

  buttonTimeout.clear();

  // Process actual button presses before encoder info
  // That is, we give more weight to a button press, even
  // if it happens whilst turning the encoder wheel.

#if defined(VIKI_ENC_PIN_A) && defined(VIKI_ENC_PIN_B)
  if ( encClicks != 0 ) {
       newButtons &= ( encClicks > 0 ) ?
	    ~(DOWN_BUTTON_MASK) : ~(UP_BUTTON_MASK);
       encClicks = 0;
  }
#endif

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

void VikiInterface::setLED(bool on) { return; }
void VikiInterface::setCoolingFanIndicator(bool state) { return; }
