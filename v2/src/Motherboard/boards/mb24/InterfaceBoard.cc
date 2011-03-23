#include <AvrPort.hh>
#include <util/atomic.h>
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

#define INTERFACE_OK_PIN		Pin(PortC,2)
#define INTERFACE_CANCEL_PIN	Pin(PortC,1)

#define INTERFACE_FOO_PIN		Pin(PortC,0)
#define INTERFACE_BAR_PIN		Pin(PortL,0)
#define INTERFACE_DEBUG_PIN		Pin(PortB,7)

class ButtonArray {
private:
	uint8_t previousL;
	uint8_t previousC;

	uint8_t buttonPress;
	bool buttonPressWaiting;
public:
	ButtonArray();

	// Returns true if any of the button states have changed.
	void scanButtons();

	bool getButton(InterfaceBoardDefinitions::ButtonName& button);
};

class InterfaceBoard {
public:
	LiquidCrystal lcd;

private:
	ButtonArray buttons;

	MainMenu mainMenu;

	Screen* screenStack[MENU_DEPTH];
	uint8_t screenIndex;

public:
	InterfaceBoard();

	// This should be run periodically to check the buttons
	void doInterrupt();

	void pushScreen(Screen* newScreen);

	void popScreen();

	void doUpdate();
};


ButtonArray::ButtonArray() :
	previousL(0),
	previousC(0)
{
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

bool ButtonArray::getButton(InterfaceBoardDefinitions::ButtonName& button) {
	bool buttonValid;
	uint8_t buttonNumber;

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		buttonValid =  buttonPressWaiting;
		buttonNumber = buttonPress;
		buttonPressWaiting = false;
	}

	if (buttonValid) {
		button = (InterfaceBoardDefinitions::ButtonName)(buttonNumber);
	}

	return buttonValid;
}


InterfaceBoard::InterfaceBoard() :
	lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN),
	buttons()
{
	// TODO: do we need to set pin directions here?

	lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
	lcd.clear();
	lcd.home();

	screenIndex = 0;
	screenStack[0] = &mainMenu;

	screenStack[screenIndex]->reset();
	screenStack[screenIndex]->update(lcd, true);
}


void InterfaceBoard::doInterrupt() {
	buttons.scanButtons();
}

void InterfaceBoard::doUpdate() {
	InterfaceBoardDefinitions::ButtonName button;

	if (buttons.getButton(button)) {
		screenStack[screenIndex]->notifyButtonPressed(button);
	}

	screenStack[screenIndex]->update(lcd, false);
}

void InterfaceBoard::pushScreen(Screen* newScreen) {
	if (screenIndex < MENU_DEPTH - 1) {
		screenIndex++;
		screenStack[screenIndex] = newScreen;
	}
	screenStack[screenIndex]->reset();
	screenStack[screenIndex]->update(lcd, true);
}

void InterfaceBoard::popScreen() {
	if (screenIndex > 0) {
		screenIndex--;
	}

	screenStack[screenIndex]->update(lcd, true);
}

InterfaceBoard board;

void init() {

}

void pushScreen(Screen* newScreen) {
	board.pushScreen(newScreen);
}

void popScreen() {
	board.popScreen();
}

void doInterrupt() {
	board.doInterrupt();
}

void doUpdate() {
	board.doUpdate();
}

}

