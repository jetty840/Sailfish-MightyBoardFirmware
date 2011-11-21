
#include "Interface.hh"
#include "InterfaceBoard.hh"


// TODO: Make this a proper module.
#if defined HAS_INTERFACE_BOARD

namespace interface {


LiquidCrystalSerial* lcd;
InterfaceBoard* board;

bool isConnected() {

	// Strategy: Set up the foo pin as an input, turn on pull up resistor,
	// then measure it. If low, then we probably have an interface board.
	// If high, we probably don't.

	INTERFACE_DETECT.setValue(true);
	INTERFACE_DETECT.setDirection(false);

	// if we are pulled up, then we have an led attached
	if (INTERFACE_DETECT.getValue()) {
		INTERFACE_GLED.setDirection(true);
		INTERFACE_GLED.setValue(true);
		
		INTERFACE_RLED.setDirection(true);
		INTERFACE_RLED.setValue(true);

		return true;
	}
	else {
		INTERFACE_GLED.setDirection(true);
		INTERFACE_GLED.setValue(false);

		return false;
	}

	return (INTERFACE_DETECT.getValue());

}

void init(InterfaceBoard* board_in, LiquidCrystalSerial* lcd_in) {
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
