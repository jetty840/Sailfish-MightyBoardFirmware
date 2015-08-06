/* StandardButtonArray - mighty_one version
 *
 * This class handles the stock OEM hardware button array on a Makerbot
 * Replicator 1 and other RevE clones. It differs from the Replicator 2/2X
 * based hardware which uses different pins for connecting up the buttons.
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
 * mighty_one based motherboards use pins on PORTJ for all buttons.
 *
 * This code is largely unchanged from the original MBI code, it is simply
 * refactored into a class that inherits from a new class heirarchy.
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
static micros_t ButtonDelay; ///< button delay for debouncing and key repeat

#define FULL_BUTTON_MAP 0x1F

void StandardButtonArray::init() {
     previousJ = 0;
     ButtonDelay = SlowDelay;

     // Set all of the known buttons to inputs (see above note)
     DDRJ = DDRJ & 0xE0;
     PORTJ = PORTJ & 0xE0;
}

void StandardButtonArray::scanButtons() {
  // Don't bother scanning if we already have a button
  // or if sufficient time has not elapsed between the last button push
  if (buttonPressWaiting ||
      (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
    return;

  uint8_t newJ = PINJ; // & 0xFE;

  buttonTimeout.clear();

  if (newJ != previousJ) {
    uint8_t diff = newJ ^ previousJ;
    for (uint8_t i = 0; i < 5; i++) {
      if (diff & (1 << i)) {
        if (!(newJ & (1 << i))) {
          if (!buttonPressWaiting) {
            buttonPress = i;
            buttonPressWaiting = true;
            buttonTimeout.start(ButtonDelay);
          }
        }
      }
    }
  }

  previousJ = newJ;
}

bool StandardButtonArray::getButton(ButtonArray::ButtonName &button) {
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

void StandardButtonArray::clearButtonPress() { previousJ = FULL_BUTTON_MAP; }

// Returns true is button is depressed
bool StandardButtonArray::isButtonPressed(ButtonArray::ButtonName button) {
  uint8_t newJ = PINJ; // & 0xFE;

  // Buttons are active low
  if (newJ & (1 << button))
    return false;

  return true;
}

void StandardButtonArray::setButtonDelay(micros_t delay) { ButtonDelay = delay; }
