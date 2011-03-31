/*
 * Copyright 2011 by Matt Mets <matt.mets@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#ifndef INTERFACE_BOARD_HH_
#define INTERFACE_BOARD_HH_

#include "Menu.hh"

// Maximum number of
#define SCREEN_STACK_DEPTH 5


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
	void init();

	// Returns true if any of the button states have changed.
	void scanButtons();

	bool getButton(InterfaceBoardDefinitions::ButtonName& button);
};


class InterfaceBoard {
public:
	LiquidCrystal lcd;
private:
	ButtonArray buttons;

	SplashScreen splashScreen;
	MainMenu mainMenu;
	MonitorMode monitorMode;

	Screen* screenStack[SCREEN_STACK_DEPTH];
	uint8_t screenIndex;

	bool building;

public:
	InterfaceBoard();

	void init();

	// This should be run periodically to check the buttons
	void doInterrupt();

	void pushScreen(Screen* newScreen);

	void popScreen();

	micros_t getUpdateRate();

	void doUpdate();

	void showMonitorMode();
};

#endif
