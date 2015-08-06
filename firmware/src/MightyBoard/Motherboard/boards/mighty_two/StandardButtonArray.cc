/* StandardButtonArray - mighty_two version
 *
 * This class handles the stock OEM hardware button array on a Makerbot
 * Replicator 2/2X.  It differs from the Replicator 1 based hardware
 * which uses different pins for connecting up the buttons.
 *
 * This is a subclass of the "ButtonArray" class, which defines the protocol
 * for a button array.
 *
 * Note: previously, Makerbot used a different set of enum values for
 * the ButtonName enumeration between mighty_one and mighty_two based systems.
 * This introdced bugs elsewhere.  As part of refactoring the code into
 * the ButtonArray base class, the mighty_one values of the enum were used
 * for all button arrays.
 *
 * mighty_two based boards have the buttons connected across two PORTs.
 *   - The arrow buttons are connected to pins on PORTJ
 *   - The center button is connected to a pin on PORTG
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
#include "StandardButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ; ///< state of the PORTJ pins from the previous scan
static uint8_t previousG; ///< state of the PORTG pins from the previous scan
static micros_t ButtonDelay; ///< button delay for debouncing and key repeat

#define ARROW_BUTTON_MAP 0x78 /**< 0b01111000 - mask of the pins on PORTJ for
                               *   the Up/Dn/L/R buttons inputs */
#define CENTER_BUTTON_MAP 0x04 ///< mask of pin on PORTG for the CENTER button
void StandardButtonArray::init() {
     previousJ = 0;
     previousG = 0;

     ButtonDelay = SlowDelay;

     // Set all of the known buttons to inputs (see above note)
     // Set all of the known buttons to inputs
     DDRJ &= ~(ARROW_BUTTON_MAP);
     PORTJ &= ~(ARROW_BUTTON_MAP);

     DDRG &= ~(CENTER_BUTTON_MAP);
     PORTG &= ~(CENTER_BUTTON_MAP);
}

void StandardButtonArray::scanButtons() {

  // Don't bother scanning if we already have a button
  if (buttonPressWaiting ||
      (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
    return;

  uint8_t newJ = PINJ & ARROW_BUTTON_MAP;
  uint8_t newG = PING & CENTER_BUTTON_MAP;

  buttonTimeout.clear();

  // center button
  if (newG != previousG) {
    if (!(newG & (PORTG_CENTER_BUTTON_MASK))) {
      buttonPress = CENTER;
      buttonPressWaiting = true;
      buttonTimeout.start(ButtonDelay);
    }
  }

  if (newJ != previousJ) {
    uint8_t diff = (newJ ^ previousJ);
    if (!buttonPressWaiting && diff) {
      if ((diff & PORTJ_UP_BUTTON_MASK) && !(newJ & PORTJ_UP_BUTTON_MASK)) {
        buttonPress = UP;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);
      } else if ((diff & PORTJ_DOWN_BUTTON_MASK) && !(newJ & PORTJ_DOWN_BUTTON_MASK)) {
        buttonPress = DOWN;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);
      } else if ((diff & PORTJ_RIGHT_BUTTON_MASK) && !(newJ & PORTJ_RIGHT_BUTTON_MASK)) {
        buttonPress = RIGHT;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);
      } else if ((diff & PORTJ_LEFT_BUTTON_MASK) && !(newJ & PORTJ_LEFT_BUTTON_MASK)) {
        buttonPress = LEFT;
      } else {
        // we didn't find a new button press, so exit without setting buttonPressWaiting
        // and starting the buttonTimeout.
        goto exitScanButtons;
      }
      buttonPressWaiting = true;
      buttonTimeout.start(ButtonDelay);
    }
  }

exitScanButtons:
  previousG = newG;
  previousJ = newJ;
}

bool StandardButtonArray::getButton(ButtonName &button) {
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

void StandardButtonArray::clearButtonPress() {

  previousJ = ARROW_BUTTON_MAP;
  previousG = CENTER_BUTTON_MAP;
}

// Returns true is button is depressed
bool
StandardButtonArray::isButtonPressed(StandardButtonArray::ButtonName button) {

  // Buttons are active low
  switch (button) {
  case CENTER:
    if (PING & PORTG_CENTER_BUTTON_MASK)
      return false;
    break;
  case RIGHT:
    if (PINJ & PORTJ_RIGHT_BUTTON_MASK)
      return false;
    break;
  case LEFT:
    if (PINJ & PORTJ_LEFT_BUTTON_MASK)
      return false;
    break;
  case DOWN:
    if (PINJ & PORTJ_DOWN_BUTTON_MASK)
      return false;
    break;
  case UP:
    if (PINJ & PORTJ_UP_BUTTON_MASK)
      return false;
    break;
  }

  return true;
}

void StandardButtonArray::setButtonDelay(uint32_t delay) {
  ButtonDelay = delay;
}
