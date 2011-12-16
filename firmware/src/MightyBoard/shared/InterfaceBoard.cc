#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "LiquidCrystalSerial.hh"
#include "Host.hh"
#include "Timeout.hh"

#if defined HAS_INTERFACE_BOARD

Timeout button_timeout;

InterfaceBoard::InterfaceBoard(ButtonArray& buttons_in,
                               LiquidCrystalSerial& lcd_in,
                               const Pin& gled_in,
                               const Pin& rled_in,
                               Screen* mainScreen_in,
                               Screen* buildScreen_in) :
        lcd(lcd_in),
        buttons(buttons_in)
{
        buildScreen = buildScreen_in;
        mainScreen = mainScreen_in;
        LEDs[0] = gled_in;
        LEDs[1] = rled_in;
}

void InterfaceBoard::init() {
        buttons.init();

        lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
        lcd.clear();
        lcd.home();

	LEDs[0].setDirection(true);
	LEDs[1].setDirection(true);

        building = false;

        screenIndex = -1;

        pushScreen(mainScreen);
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
                        pushScreen(buildScreen);
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


        static ButtonArray::ButtonName button;


	if (buttons.getButton(button)) {
		screenStack[screenIndex]->notifyButtonPressed(button);
		if(screenStack[screenIndex]->continuousButtons())
		{
			button_timeout.start(300000);// 0.3s timeout 
			//gled.setValue(false);
		}
	}
	// clear button press if button timeout occurs in continuous press mode
	if(button_timeout.hasElapsed())
	{
		buttons.clearButtonPress();
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

void InterfaceBoard::setLED(uint8_t id, bool on){
	LEDs[id].setValue(on);
}

#endif
