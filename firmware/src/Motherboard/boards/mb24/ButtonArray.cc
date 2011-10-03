#include "ButtonArray.hh"

static uint8_t previousL;
static uint8_t previousC;

void ButtonArray::init() {
        previousL = 0;
        previousC = 0;

        // Set all of the known buttons to inputs (see above note)
        DDRL = DDRL & 0x1;
        DDRC = DDRC & 0xF9;
        PORTL = PORTL & 0x1;
        PORTC = PORTC & 0xF9;
}

void ButtonArray::scanButtons() {
        // Don't bother scanning if we already have a button.
        if (buttonPressWaiting) {
                return;
        }

        uint8_t newL = PINL;// & 0xFE;
        uint8_t newC = PINC;// & 0x06;

        if (newL != previousL) {
                uint8_t diff = newL ^ previousL;

                for(uint8_t i = 1; i < 8; i++) {
                        if (diff&(1<<i)) {
                                if (!(newL&(1<<i))) {
                                        if (!buttonPressWaiting) {
                                                buttonPress = i;
                                                buttonPressWaiting = true;
                                        }
                                }
                        }
                }
        }

        if (newC != previousC) {
                uint8_t diff = newC ^ previousC;

                for(uint8_t i = 1; i < 3; i++) {
                        if (diff&(1<<i)) {
                                if (!(newC&(1<<i))) {
                                        if (!buttonPressWaiting) {
                                                buttonPress = i+10;
                                                buttonPressWaiting = true;
                                        }
                                }
                        }
                }
        }

        previousL = newL;
        previousC = newC;
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
