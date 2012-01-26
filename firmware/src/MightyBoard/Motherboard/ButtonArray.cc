#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
bool  holding = false;

void ButtonArray::init() {
        previousJ = 0;

        // Set all of the known buttons to inputs (see above note)
        DDRJ = DDRJ & 0x1F;
        PORTJ = PORTJ & 0x1F;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        // or if sufficient time has not elapsed between the last button push
        if (buttonPressWaiting && buttonTimeout.hasElapsed()) {
                return;
        }
        
        buttonTimeout.clear();

        uint8_t newJ = PINJ;// & 0xFE;
        
        if(!(newJ & (1 << CENTER))){
			if(!holding){
				centerHold.start(10000000);
				holding = true;
			}
		}
		else{
			centerHold = Timeout();
			holding = false;
		}
			
		if(centerHold.hasElapsed()){
			buttonPress = RESET;
			buttonPressWaiting = true;
			return;
		}
        
        if (newJ != previousJ) {
                uint8_t diff = newJ ^ previousJ;
                for(uint8_t i = 0; i < 5; i++) {
                        if (diff&(1<<i)) {
                                if (!(newJ&(1<<i))) {
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
