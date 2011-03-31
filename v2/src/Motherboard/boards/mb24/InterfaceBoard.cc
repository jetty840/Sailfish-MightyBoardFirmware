#include <AvrPort.hh>
#include <util/atomic.h>
#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "SDCard.hh"
#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"
#include "Host.hh"

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
	lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN)
{
	buttons.init();

	lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
	lcd.clear();
	lcd.home();

	INTERFACE_FOO_PIN.setValue(false);
	INTERFACE_FOO_PIN.setDirection(true);
	INTERFACE_BAR_PIN.setValue(false);
	INTERFACE_BAR_PIN.setDirection(true);

	building = false;

	screenIndex = 0;
	screenStack[screenIndex] = &mainMenu;
	screenStack[screenIndex]->reset();

	pushScreen(&splashScreen);
}

void InterfaceBoard::init() {
}

void InterfaceBoard::doInterrupt() {
	buttons.scanButtons();
}

micros_t InterfaceBoard::getUpdateRate() {
	return screenStack[screenIndex]->getUpdateRate();
}

void InterfaceBoard::doUpdate() {
	// If we are building, make sure we show a build menu; otherwise,
	// turn it off.
	switch(host::getHostState()) {
	case host::HOST_STATE_BUILDING:
	case host::HOST_STATE_BUILDING_FROM_SD:
		if (!building) {
			pushScreen(&monitorMode);
			building = true;
		}
		break;
	default:
		if (building) {
			popScreen();
			building = false;
		}
		break;
	}

	InterfaceBoardDefinitions::ButtonName button;

	if (buttons.getButton(button)) {
		screenStack[screenIndex]->notifyButtonPressed(button);
	}

	screenStack[screenIndex]->update(lcd, false);
}

void InterfaceBoard::pushScreen(Screen* newScreen) {
	if (screenIndex < SCREEN_STACK_DEPTH - 1) {
		screenIndex++;
		screenStack[screenIndex] = newScreen;
	}
	screenStack[screenIndex]->reset();
	screenStack[screenIndex]->update(lcd, true);
}

void InterfaceBoard::popScreen() {
	// Don't allow the root menu to be removed.
	if (screenIndex > 0) {
		screenIndex--;
	}

	screenStack[screenIndex]->update(lcd, true);
}

