#include <AvrPort.hh>
#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "SDCard.hh"


namespace interfaceboard {

class InterfaceBoard;
class ButtonArray;


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

class ButtonArray {
private:
	uint8_t previousL;
	uint8_t previousC;

public:
	ButtonArray();

	// Returns true if any of the button states have changed.
	bool scanButtons(InterfaceBoard& board);


};

class InterfaceBoard {
public:
	LiquidCrystal lcd;

private:
	ButtonArray buttons;

	MainMenu mainMenu;

	Menu* menuStack[MENU_DEPTH];
	uint8_t menuIndex;

public:
	InterfaceBoard();

	// This should be run periodically to check the buttons, update screen, etc
	void doInterrupt();

	// This gets called whenever a button is pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);

	void pushMenu(Menu* newMenu);

	void popMenu();
};


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

	menuIndex = 0;
	menuStack[0] = &mainMenu;

	menuStack[menuIndex]->reset();
	menuStack[menuIndex]->draw(lcd, true);
}


void InterfaceBoard::doInterrupt() {
	buttons.scanButtons(*this);
}

void InterfaceBoard::notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button) {
	menuStack[menuIndex]->notifyButtonPressed(button);
	menuStack[menuIndex]->draw(lcd, false);
}

void InterfaceBoard::pushMenu(Menu* newMenu) {
	if (menuIndex < MENU_DEPTH - 1) {
		menuIndex++;
		menuStack[menuIndex] = newMenu;
	}
	menuStack[menuIndex]->reset();
	menuStack[menuIndex]->draw(lcd, true);
}

void InterfaceBoard::popMenu() {
	if (menuIndex > 0) {
		menuIndex--;
	}

	menuStack[menuIndex]->draw(lcd, true);
}

InterfaceBoard board;

void init() {

}

void pushMenu(Menu* newMenu) {
	board.pushMenu(newMenu);
}

void popMenu() {
	board.popMenu();
}

void doInterrupt() {
	board.doInterrupt();
}

}

