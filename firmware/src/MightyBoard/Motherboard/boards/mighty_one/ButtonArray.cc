#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;

void ButtonArray::init() {
        previousJ = 0;

        // Set all of the known buttons to inputs (see above note)
        DDRJ = DDRJ & 0xE0;
        PORTJ = PORTJ & 0xE0;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        // or if sufficient time has not elapsed between the last button push
        if (buttonPressWaiting || (buttonTimeout.isActive() && !buttonTimeout.hasElapsed())) {
                return;
        }
        
        uint8_t newJ = PINJ;// & 0xFE;
        
        buttonTimeout.clear();

	for(uint8_t i = 0; i < 5; i++) {
		if (!(newJ&(1<<i))) {
			if (!buttonPressWaiting) {
				buttonPress = i;
				buttonPressWaiting = true;
				buttonTimeout.start(ButtonDelay);
			}
		}
        }

        previousJ = newJ;
}

bool ButtonArray::getButton(ButtonName& button) {
        bool buttonValid;
        uint8_t buttonNumber;

        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
                buttonValid =  buttonPressWaiting;
                buttonNumber = buttonPress;        
                buttonPressWaiting = false;             
        }

        if (buttonValid) {
                button = (ButtonName)(buttonNumber);
        }

        return buttonValid;
}

void ButtonArray::clearButtonPress(){

		previousJ = 0x1F;
}

//Returns true is button is depressed
bool ButtonArray::isButtonPressed(ButtonArray::ButtonName button) {
        uint8_t newJ = PINJ;// & 0xFE;

	//Buttons are active low
	if ( newJ & (1<<button) ) return false;

	return true;
}

