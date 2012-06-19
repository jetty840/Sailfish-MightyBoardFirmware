#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
static uint8_t previousG;
bool  center_holding = false;
bool right_holding = false;

static const uint8_t ARROW_BUTTON_MAP = 0x78;
static const uint8_t CENTER_BUTTON_MAP = 0x04;

void ButtonArray::init() {
        previousJ = 0;
        previousG = 0;

        // Set all of the known buttons to inputs 
        DDRJ = DDRJ & ARROW_BUTTON_MAP; 
        PORTJ = PORTJ & ARROW_BUTTON_MAP; 
        
      //  DDRG = DDRG & CENTER_BUTTON_MAP; 
     //   PORTG = PORTG & CENTER_BUTTON_MAP; 
        
        INTERFACE_CENTER.setDirection(false);
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button 
        // or if sufficient time has not elapsed between the last button push
        if (buttonPressWaiting || (buttonTimeout.isActive() && !buttonTimeout.hasElapsed())) {
                return;
        }
        
        buttonTimeout.clear();

        uint8_t newJ = PINJ;// & 0xFE;
        uint8_t newG = PING & CENTER_BUTTON_MAP;
        
        /// test for special holds
        /// center hold
        if(!(newG & (1 << CENTER))){
			if(!center_holding){
				centerHold.start(10000000);
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
				rightHold.start(10000000);
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
		
		if(newG != previousG){
			uint8_t diff = newG ^ previousG;
			if (diff&(1<<CENTER)){
				if(!(newG&(1<<CENTER))){
					buttonPress = CENTER;
					buttonPressWaiting = true;
					buttonTimeout.start(ButtonDelay);
				}
			}
		}
		      
        if (newJ != previousJ) {
                uint8_t diff = newJ ^ previousJ;
                for(uint8_t i = 3; i < 7; i++) {
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
