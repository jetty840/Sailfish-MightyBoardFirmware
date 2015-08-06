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

#include "Compat.hh"
#include "AnalogButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>
#include "AnalogPin.hh"

static micros_t ButtonDelay;

#if defined(HAS_ANALOG_BUTTONS)
static uint8_t previousButton; ///< state of the button pins from the previous scan
#endif

void AnalogButtonArray::init() {
	adcValid = false;
	initAnalogPin(ANALOG_BUTTONS_PIN);
}

// This function takes an ADC value and returns which button was pressed.
// It is private, and really used only once, so inlined.
inline uint8_t AnalogButtonArray::buttonFromADC(int16_t adc_value) {
	if (adc_value < 50) {
		return LEFT;
	} else if (adc_value < 200) {
		return UP;
	} else if (adc_value < 400) {
		return DOWN;
	} else if (adc_value < 600) {
		return RIGHT;
	} else if (adc_value < 800) {
		return CENTER;
	}
	return 255;
}

void AnalogButtonArray::scanButtons() {
	// This is similar to the Thermistor.cc implementation
	// Save the current value and valid flag first.
	int16_t buttonValue;
	bool valid;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		valid = adcValid;
		buttonValue = adcValue;

		// Invalidate the result now that we have read it
		if (adcValid)
			adcValid = false;
	}
	
	// initiate next read, if the ADC is busy, return and wait for next time.
	if (!startAnalogRead(ANALOG_BUTTONS_PIN, &adcValue, &adcValid)) return;
	
	// We we don't have a valid reading return and wait for next time.
	if (!valid) return;

    // Don't bother scanning if we already have a button
    if (buttonPressWaiting || (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
    return;
	
	// Take the ADC value and determine if we have a button press
	uint8_t currentButton = buttonFromADC(buttonValue);
	
	// See if the button we have now is different from the last button
	if (currentButton == previousButton) {
		buttonCount++;
		
		// If we have mutliple values of the same button, then act on it.
		if (currentButton != NO_BUTTON && buttonCount > DEBOUNCE_COUNT) {
			buttonTimeout.clear();
			buttonPress = currentButton;
			buttonPressWaiting = true;
			buttonTimeout.start(ButtonDelay);			
		}
	} else {
		buttonCount = 0;
	}
	
	// Save the current value for next time.
	previousButton = currentButton;
}

bool AnalogButtonArray::getButton(ButtonArray::ButtonName& button) {
        bool buttonValid;
        uint8_t buttonNumber;

        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
                buttonValid  = buttonPressWaiting;
                buttonNumber = buttonPress;        
                buttonPressWaiting = false;             
        }

        if (buttonValid) {
                button = (ButtonName)(buttonNumber);
        }

        return buttonValid;
}

void AnalogButtonArray::clearButtonPress(){
	//Reset the previous state to "no button pressed"
	previousButton=NO_BUTTON;
}

//Returns true is button is depressed
bool AnalogButtonArray::isButtonPressed(ButtonArray::ButtonName button) {
	return (buttonPress == button);
}

void AnalogButtonArray::setButtonDelay(uint32_t delay) {
        ButtonDelay = delay;
}
