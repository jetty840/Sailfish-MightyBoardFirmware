#include "ButtonArray.hh"
#include "Motherboard.hh"

static uint8_t previousJ;

void ButtonArray::init() {
        previousJ = 0;

        // Set all of the known buttons to inputs (see above note)
        DDRJ = DDRJ & 0x1F;
        PORTJ = PORTJ & 0x1F;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button.
        if (buttonPressWaiting) {
                return;
        }

        uint8_t newJ = PINJ;// & 0xFE;

        if (newJ != previousJ) {
                uint8_t diff = newJ ^ previousJ;

                for(uint8_t i = 0; i < 5; i++) {
                        if (diff&(1<<i)) {
                                if (!(newJ&(1<<i))) {
                                        if (!buttonPressWaiting) {
                                                buttonPress = i;
                                                buttonPressWaiting = true;
                                                if(i > 2)
                                                {
													INTERFACE_GLED.setValue(true);
													INTERFACE_RLED.setValue(false);
												}
												else if (i == 0){
													INTERFACE_RLED.setValue(true);
													INTERFACE_GLED.setValue(false);
												}
												else if (i == 1){
													INTERFACE_RLED.setValue(true);
													INTERFACE_GLED.setValue(true);
												}
												else if (i==2) {
													INTERFACE_RLED.setValue(false);
													INTERFACE_GLED.setValue(false);
												}
													
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
             //   DEBUG_PIN3.setValue(false);
        }

        if (buttonValid) {
                button = (ButtonName)(buttonNumber);
        }

        return buttonValid;
}
