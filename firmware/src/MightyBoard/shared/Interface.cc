
#include "Interface.hh"
#include "InterfaceBoard.hh"
#include "Configuration.hh"


// TODO: Make this a proper module.
#if defined HAS_INTERFACE_BOARD

namespace interface {


LiquidCrystalSerial* lcd;
InterfaceBoard* board;

bool isConnected() {
	// Avoid repeatedly creating temp objects
	const Pin InterfaceDetect = INTERFACE_DETECT;

	// Strategy: Set up the foo pin as an input, turn on pull up resistor,
	// then measure it. If low, then we probably have an interface board.
	// If high, we probably don't.

	InterfaceDetect.setValue(false);
	InterfaceDetect.setDirection(false);

	// if we are pulled down, then we have an led attached??
	if (InterfaceDetect.getValue()) {

		board->setLED(1,false);
		board->setLED(0,false);
		return true;
	}
	else {

		return false;
	}

	return (InterfaceDetect.getValue());

}
void setBuildPercentage(uint8_t percent){
	board->setBuildPercentage(percent);
}

void init(InterfaceBoard* board_in, LiquidCrystalSerial* lcd_in) {
    board = board_in;
    lcd = lcd_in;
}

void pushScreen(Screen* newScreen) {
        board->pushScreen(newScreen);
}

void pushNoUpdate(Screen *newScreen){
	board->pushNoUpdate(newScreen);
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

void setLEDs(bool on){
	board->setLED(0,on);
	board->setLED(1,on);
}

}

#endif
