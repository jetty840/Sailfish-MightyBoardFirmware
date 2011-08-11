
#include "Interface.hh"
#include "InterfaceBoard.hh"


// TODO: Make this a proper module.
#if defined HAS_INTERFACE_BOARD

namespace interface {


LiquidCrystal* lcd;
InterfaceBoard* board;

bool isConnected() {

	// Strategy: Set up the foo pin as an input, turn on pull up resistor,
	// then measure it. If low, then we probably have an interface board.
	// If high, we probably don't.

	INTERFACE_FOO_PIN.setValue(true);
	INTERFACE_FOO_PIN.setDirection(false);

	// if we are pulled down, then we have an led attached??
	if (!INTERFACE_FOO_PIN.getValue()) {
		INTERFACE_FOO_PIN.setDirection(true);
		INTERFACE_FOO_PIN.setValue(true);

		return true;
	}
	else {
		INTERFACE_FOO_PIN.setDirection(true);
		INTERFACE_FOO_PIN.setValue(false);

		return false;
	}

	return (!INTERFACE_FOO_PIN.getValue());

}

void init(InterfaceBoard* board_in, LiquidCrystal* lcd_in) {
    board = board_in;
    lcd = lcd_in;
}

void pushScreen(Screen* newScreen) {
        board->pushScreen(newScreen);
}

void popScreen() {
        board->popScreen();
}

void doInterrupt() {
        board->doInterrupt();
}

micros_t getUpdateRate() {
        return board->getUpdateRate();
}

void doUpdate() {
        board->doUpdate();
}


}

#endif
