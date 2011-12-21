#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"

static uint8_t previousJ;

void ButtonArray::init() {
        previousJ = 0;

	INTERFACE_RLED.setDirection(true);
	INTERFACE_GLED.setDirection(true);

	INTERFACE_RLED.setValue(false);
	INTERFACE_GLED.setValue(false);

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
