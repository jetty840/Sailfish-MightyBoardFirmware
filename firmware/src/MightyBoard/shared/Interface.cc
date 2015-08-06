#include "Compat.hh"
#include "Interface.hh"
#include "InterfaceBoard.hh"
#include "Configuration.hh"
#if defined(HAS_I2C_LCD)
#include "LiquidCrystalSerial_I2C.hh"
#elif defined(HAS_VIKI_INTERFACE)
#include "VikiInterface.hh"
#endif

// TODO: Make this a proper module.
#if defined HAS_INTERFACE_BOARD

namespace interface {


LiquidCrystalSerial* lcd;
InterfaceBoard* board;

bool isConnected() {
	
#if defined(HAS_I2C_LCD)
     // the I2C display is detectable on the bus.  If we have
     // detected such a display, then return true
     if (((LiquidCrystalSerial_I2C*)lcd)->hasI2CDisplay()) 
	  return true;
     else
	  return false;
#elif defined(HAS_VIKI_INTERFACE)
     if (((VikiInterface*)lcd)->hasI2CDisplay()) 
	  return true;  
     else
	  return false;
#else
     // Avoid repeatedly creating temp objects
     const Pin InterfaceDetect = INTERFACE_DETECT;

     // Strategy: Set up the foo pin as an input, turn on pull up resistor,
     // then measure it. If low, then we probably have an interface board.
     // If high, we probably don't.

     InterfaceDetect.setValue(false);
     InterfaceDetect.setDirection(false);

     // if we are pulled down, then we have an led attached??
     return InterfaceDetect.getValue() ? true : false;
#endif
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

bool isButtonPressed(ButtonArray::ButtonName button) {
        return board->isButtonPressed(button);
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
	board->setLED(on);
}


}

#endif
