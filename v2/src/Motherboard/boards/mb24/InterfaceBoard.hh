#ifndef INTERFACE_BOARD_HH_
#define INTERFACE_BOARD_HH_

#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"
#include "Menu.hh"
#include "ButtonArray.hh"

class InterfaceBoard;

/**
 * Monitors the input state of a bunch of buttons, in polling mode.
 **/

#ifndef BUTTON_ARRAY_HH_
#define BUTTON_ARRAY_HH_

class ButtonArray {
private:
	uint8_t previousL;
	uint8_t previousC;

public:
	ButtonArray();

	// Returns true if any of the button states have changed.
	bool scanButtons(InterfaceBoard& board);


};

#endif


class InterfaceBoard {
public:
	LiquidCrystal lcd;

private:
	ButtonArray buttons;

	// For the file list menu
	int fileIndex;
	MainMenu mainMenu;
	SDMenu sdMenu;

	Menu* currentMenu;

public:
	InterfaceBoard();

	// This should be run periodically to check the buttons, update screen, etc
	void doInterrupt();

	// This gets called whenever a button is pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};

#endif
