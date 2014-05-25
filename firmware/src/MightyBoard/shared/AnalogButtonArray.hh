/* AnalogButtonArray
 *
 * This class is a driver for a keypad that uses a single analog input.
 *
 * Specifically this driver was built for and tested with the following module:
 *
 * http://www.dx.com/p/ad-keyboard-simulate-five-key-module-256781
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

#ifndef ANALOGBUTTONARRAY_HH
#define ANALOGBUTTONARRAY_HH

#include "ButtonArray.hh"
#include <util/atomic.h>
#include "Types.hh"
#include "Timeout.hh"

#define DEBOUNCE_COUNT 2
#define NO_BUTTON 255

// See shared/ButtonArray.h for descriptions of the functions of this class.
class AnalogButtonArray : public ButtonArray {
private:
  bool adcValid;
  int16_t adcValue;
  uint8_t previousButton;
  uint8_t buttonCount;

  uint8_t buttonFromADC(int16_t adc_value);

public:
  void init();

  // Returns true if any of the button states have changed.
  void scanButtons();

  bool getButton(ButtonArray::ButtonName &button);

  void clearButtonPress();

  bool isButtonPressed(ButtonArray::ButtonName button);

  void setButtonDelay(micros_t delay);
};

#endif // ANALOGBUTTONARRAY_HH