/* StnadardButtonArray - mighty_two version
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

#ifndef STANDARDBUTTONARRAY_HH
#define STANDARDBUTTONARRAY_HH

#include "Compat.hh"
#include "ButtonArray.hh"
#include <util/atomic.h>
#include "Types.hh"
#include "Timeout.hh"

/* **The following comments are from the original MBI source code **

// TODO: Make this an interface?

/// The button array modules manages an array of buttons, and maintains
/// a buffer of the last pressed button. It has two entry points: a fast
/// #scanButtons, which is a fast button scanning routine that should be
/// called from an interrupt, and #getButton, which should be called by a
/// slow loop that has time to respond to the button.
///
/// Porting Notes:
/// This modules uses low-level port registers, and must be re-written for
/// each board archetecture. This should be done by adding a ButtonArray.cc
/// definition in the board directory.
/// \ingroup HardwareLibraries
  ** END original MBI source code comment. */

// The following bitmaks define the locations of the button bits on their
// various ports.
#define PORTG_CENTER_BUTTON_MASK (1 << 2)
#define PORTJ_RIGHT_BUTTON_MASK (1 << 3)
#define PORTJ_DOWN_BUTTON_MASK (1 << 4)
#define PORTJ_UP_BUTTON_MASK (1 << 5)
#define PORTJ_LEFT_BUTTON_MASK (1 << 6)

// See shared/ButtonArray.h for descriptions of the functions of this class.
class StandardButtonArray : public ButtonArray {

public:
  void init();

  // Returns true if any of the button states have changed.
  void scanButtons();

  bool getButton(ButtonName &button);

  void clearButtonPress();

  bool isButtonPressed(ButtonArray::ButtonName button);

  void setButtonDelay(micros_t delay);
};

#endif // STANDARDBUTTONARRAY_HH
