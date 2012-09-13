#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
bool  center_holding = false;
bool right_holding = false;

void ButtonArray::init() {
    previousJ = 0;

    ButtonDelay = SlowDelay;

    // Set all of the known buttons to inputs (see above note)
    DDRJ = DDRJ & 0xE0;
    PORTJ = PORTJ & 0xE0;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        if (buttonPressWaiting ){
			return;
		}	
		
		uint8_t newJ = PINJ;// & 0xFE;		
			
		uint8_t diff = newJ ^ previousJ;
		// if the buttons have changed at all, set the button timeout to slow speed
		if(diff){
			ButtonDelay = SlowDelay;
		// if buttons are the same and our timeout has not expired, come back later
		} else if ((buttonTimeout.isActive() && !buttonTimeout.hasElapsed())){
			return;
		// if buttons are the same and our timeout has expired, set timeout to fast speed
		}else{
			ButtonDelay = FastDelay;
		}
			
        buttonTimeout.clear();

        
        /// test for special holds
        /// center hold
        if(!(newJ & (1 << CENTER))){
			if(!center_holding){
				centerHold.start(ResetDelay);
				center_holding = true;
			}
		}
		else{
			centerHold = Timeout();
			center_holding = false;
		}
			
		if(centerHold.hasElapsed()){
			buttonPress = RESET;
			buttonPressWaiting = true;
			centerHold = Timeout();
			return;
		}
		/// right hold
		if(!(newJ & (1 << RIGHT))){
			if(!right_holding){
				rightHold.start(ResetDelay);
				right_holding = true;
			}
		}
		else{
			rightHold = Timeout();
			right_holding = false;
		}
			
		if(rightHold.hasElapsed()){
			buttonPress = EGG;
			buttonPressWaiting = true;
			rightHold = Timeout();
			return;
		}
		
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

void ButtonArray::setButtonDelay(uint32_t delay){
  ButtonDelay = delay;

}
