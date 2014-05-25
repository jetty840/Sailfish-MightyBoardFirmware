/* ButtonArray
 *
 * This is an abstract base class that should be subclassed as a driver for
 * ButtonArray hardware.  1 and 2/2X series replicators have different hardware
 * and hence need different software implementations.
 *
 * Note that previously, Makerbot used a different set of enum values for
 * the ButtonName enumeration between mighty_one and mighty_two based systems.
 * This introdced bugs elsewhere.  As part of refactoring the code into
 * the ButtonArray base class, the mighty_one values of the enum were used
 * for all button arrays.
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

#ifndef BUTTONARRAY_HH
#define BUTTONARRAY_HH

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

/* Note Originally, MBI defined different CONTINUOUS_BUTTON_MASKS for
   mighty_one and mighty_two implementations.  In refactoring this interface,
   the mighty_one bitmask and ButtonName enum values were selected as standard.

   For a mighty_one, this is easy as the pins, masks, and ButtonName values all
   correspond.  (eg bit 0 on PORTJ corresponds to CENTER, which is 0 in the
   enum) Any implementor of a ButtonArray subclass should make sure that
   the class uses this mask and the associated ButtonNames, even if the
   hardware does not correspond.
*/

/// The following masks are used as optionsMasks for screens and menus.
#define CONTINUOUS_BUTTONS_MASK 0b00011111
#define IS_STICKY_MASK _BV(6)
#define IS_CANCEL_SCREEN_MASK _BV(7)

// These bitmasks must not be within the CONTINUOUS_BUTTONS_MASK
#if (CONTINUOUS_BUTTONS_MASK & IS_STICKY_MASK) ||                              \
    (CONTINUOUS_BUTTONS_MASK &IS_CANCEL_SCREEN_MASK)
#error CONTINUOUS_BUTTONS_MASK must be distinct from IS_STICKY_MASK and IS_CANCEL_SCREEN_MASK
#endif

// RESET_MASK appears to not be used, so it is commented out.
//#define RESET_MASK 0x06

class ButtonArray {
protected:
  uint8_t buttonPress;
  bool buttonPressWaiting;
  Timeout buttonTimeout;

public:
  enum ButtonName {
    CENTER = 0,
    RIGHT = 1,
    LEFT = 2,
    DOWN = 3,
    UP = 4,
  };

  const static micros_t ContinuousButtonRepeatDelay =
      320000; // Must be larger than ButtonDelay
  const static uint32_t FastDelay = 100000;
  const static uint32_t SlowDelay = 300000;

  /* initialization function

     Used to intitialize the ButtonArray. Should be used to initialize any
     pins as inputs, and set the initial state of the ButtonArray.
  **/
  virtual void init() = 0;

  /** a fast button scanning routine that should be called from an interrupt.

    Note: I do not beleive that this is acutally called from an interrupt,
    and is instead called from a slice.
  **/
  virtual void scanButtons() = 0;

  /*  Check if a button is currently depressed.

      This should be called by a slow loop that has time to respond to the
     button.

      button - returns the ButtonName of the currently depressed button.
      return - returns true if a button is pressed and a value is placed in
     button,
               false if no button is pressed and the button parameter is
     unchanged.
  */
  virtual bool getButton(ButtonName &button) = 0;

  /* Clears the previous state of the ButtonArray from memory.

    The ButtonArray stores the previous state of the buttons, and this
    function clears that previous state.  This is used by the continuous
    button press to trigger the next repetition.
  */
  virtual void clearButtonPress() = 0;

  /* Check to see if a button is currently pressed.

    button - the button to test.
    return - true if pressed, false if not.
  */
  virtual bool isButtonPressed(ButtonArray::ButtonName button) = 0;

  /* Set the button delay - additional button presses will be ignored during
     this time interval.

     delay - the new button delay in microseconds.
  */
  virtual void setButtonDelay(micros_t delay) = 0;
};

#endif // BUTTONARRAY_HH
