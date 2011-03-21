#include <AvrPort.hh>
#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "SDCard.hh"


/**
 * This is the pin mapping for the interface board. Because of the relatively
 * high cost of using the pins in a direct manner, we will instead read the
 * buttons directly by scanning their ports. If any of these definitions are
 * modified, the scanButtons() function
 */
#define INTERFACE_X+_PIN		Pin(PortL,7)
#define INTERFACE_X-_PIN		Pin(PortL,6)
#define INTERFACE_Y+_PIN		Pin(PortL,5)
#define INTERFACE_Y-_PIN		Pin(PortL,4)
#define INTERFACE_Z+_PIN		Pin(PortL,3)
#define INTERFACE_Z-_PIN		Pin(PortL,2)
#define INTERFACE_ZERO_PIN		Pin(PortL,1)

#define INTERFACE_CANCEL_PIN	Pin(PortC,1)
#define INTERFACE_OK_PIN		Pin(PortC,2)

#define INTERFACE_FOO_PIN		Pin(PortC,0)
#define INTERFACE_BAR_PIN		Pin(PortL,0)
#define INTERFACE_DEBUG_PIN		Pin(PortB,7)

ButtonArray::ButtonArray() :
	previousL(0),
	previousC(0)
{
	// Set all of the known buttons to inputs (see above note)
	DDRL = DDRL & 0x1;
	DDRC = DDRC & 0xF9;
}

bool ButtonArray::scanButtons(InterfaceBoard& board) {
	uint8_t newL = PINL & 0xFE;
	uint8_t newC = PINC & 0x06;

	if (newL != previousL) {
		uint8_t diff = newL ^ previousL;

		for(uint8_t i = 0; i < 8; i++) {
			if (diff&(1<<i)) {
				if (!(newL&(1<<i))) {
					// This button was pressed, notify someone.
					board.notifyButtonPressed((InterfaceBoardDefinitions::ButtonName)i);
				}
			}
		}
	}

	if (newC != previousC) {
		uint8_t diff = newC ^ previousC;

		for(uint8_t i = 0; i < 8; i++) {
			if (diff&(1<<i)) {
				if (!(newC&(1<<i))) {
					// This button was pressed, notify someone.
					board.notifyButtonPressed((InterfaceBoardDefinitions::ButtonName)(i+10));
				}
			}
		}
	}

	previousL = newL;
	previousC = newC;

	return false;
}


InterfaceBoard::InterfaceBoard() :
	lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN),
	buttons()
{
	// TODO: do we need to set pin directions here?

	lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
	lcd.clear();
	lcd.home();

	currentMenu = &sdMenu;

	currentMenu->reset();
	currentMenu->draw(lcd);
}


void InterfaceBoard::doInterrupt() {
	buttons.scanButtons(*this);
}

void InterfaceBoard::notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button) {
	currentMenu->notifyButtonPressed(button);
	currentMenu->draw(lcd);
}


