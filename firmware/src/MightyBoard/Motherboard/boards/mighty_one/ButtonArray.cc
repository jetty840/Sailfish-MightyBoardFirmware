#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
static micros_t ButtonDelay;

void ButtonArray::init() {
        previousJ = 0;

	ButtonDelay = SlowDelay;

        // Set all of the known buttons to inputs (see above note)
        DDRJ = DDRJ & 0xE0;
        PORTJ = PORTJ & 0xE0;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        // or if sufficient time has not elapsed between the last button push
        if (buttonPressWaiting || (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
	    return;
#if 0
        if (buttonPressWaiting)
	    return;
#endif  
        
        uint8_t newJ = PINJ;// & 0xFE;

#if 0
	uint8_t diff = newJ ^ previousJ;

	// if the buttons have changed at all, set the button timeout to slow speed
	if ( diff )
	    ButtonDelay = SlowDelay;
	// if buttons are the same and our timeout has not expired, come back later
        else if ( (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()) )
	    return;
        // if buttons are the same and our timeout has expired, set timeout to fast speed
	else
	    ButtonDelay = FastDelay;
#endif

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

void ButtonArray::setButtonDelay(micros_t delay) {
        ButtonDelay = delay;
}
