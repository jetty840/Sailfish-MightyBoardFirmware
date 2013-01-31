#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
static uint8_t previousG;

// Use #define for these

//static const uint8_t ARROW_BUTTON_MAP = 0x78;
//static const uint8_t CENTER_BUTTON_MAP = 0x04;

#define ARROW_BUTTON_MAP 0x78
#define CENTER_BUTTON_MAP 0x04

void ButtonArray::init() {
        previousJ = 0;
	previousG = 0;

        // Set all of the known buttons to inputs (see above note)
        // Set all of the known buttons to inputs 
        DDRJ = DDRJ | (0xFF -  ARROW_BUTTON_MAP); 
        PORTJ = PORTJ | (0xFF -  ARROW_BUTTON_MAP); 

        INTERFACE_CENTER.setDirection(false);
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        // or if sufficient time has not elapsed between the last button push
        if (buttonPressWaiting || (buttonTimeout.isActive() && !buttonTimeout.hasElapsed())) {
                return;
        }
        
        uint8_t newJ = PINJ & ARROW_BUTTON_MAP;// & 0xFE;
	uint8_t newG = PING & CENTER_BUTTON_MAP;

        buttonTimeout.clear();

        /// center hold
	if(!(newG&(1<<CENTER))){
		buttonPress = CENTER;
		buttonPressWaiting = true;
		buttonTimeout.start(ButtonDelay);
	}

	for(uint8_t i = 3; i < 7; i++) {
		if (!(newJ&(1<<i))) {
			if (!buttonPressWaiting) {
				buttonPress = i;
				buttonPressWaiting = true;
				buttonTimeout.start(ButtonDelay);
			}
		}
	}

        previousG = newG;
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

	previousJ = ARROW_BUTTON_MAP;
	previousG = CENTER_BUTTON_MAP;
}

//Returns true is button is depressed
bool ButtonArray::isButtonPressed(ButtonArray::ButtonName button) {

	//Buttons are active low
	if ( button == CENTER ) {
		uint8_t newG = PING & CENTER_BUTTON_MAP;
	        if ( newG & (1 << CENTER) ) return false;
	}
	else {
		uint8_t newJ = PINJ & ARROW_BUTTON_MAP;
		if ( newJ & ( 1 << button ) ) return false;
	}

	return true;
}

