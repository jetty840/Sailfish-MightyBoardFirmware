#include "Compat.hh"
#include "Configuration.hh"
#include "InterfaceBoard.hh"
#include "LiquidCrystalSerial.hh"
#include "Host.hh"
#include "Timeout.hh"
#include "Command.hh"
#include "Motherboard.hh"

#if defined HAS_INTERFACE_BOARD

Timeout button_timeout;

InterfaceBoard::InterfaceBoard(ButtonArray& buttons_in,
                               LiquidCrystalSerial& lcd_in,
                               Screen* mainScreen_in,
                               Screen* buildScreen_in,
                               MessageScreen* messageScreen_in,
	                       Screen* buildFinishedScreen_in) :
        lcd(lcd_in),
        buttons(buttons_in),
	waitingMask(0)
{
        buildScreen = buildScreen_in;
        mainScreen = mainScreen_in;
        messageScreen = messageScreen_in;
	buildFinishedScreen = buildFinishedScreen_in;
}

void InterfaceBoard::init() {
	buttons.init();

	lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);

	lcd.clear();
	lcd.home();

    building = false;

    screenIndex = -1;
	waitingMask = 0;
    pushScreen(mainScreen);
    screen_locked = false;
    buttonRepetitions = 0;
    lockoutButtonRepetitionsClear = false;

#if defined(INTERFACE_LED_PORT) && defined(INTERFACE_DDR) && defined(INTERFACE_LED)
    INTERFACE_DDR |= INTERFACE_LED;
    INTERFACE_LED_PORT |= INTERFACE_LED;
#endif
}

void InterfaceBoard::setLED(bool on) {
#if defined(INTERFACE_LED_PORT) && defined(INTERFACE_DDR) && defined(INTERFACE_LED)
     if ( on ) {
	  INTERFACE_LED_PORT |= INTERFACE_LED;
     }
     else {
	  INTERFACE_LED_PORT &= ~(INTERFACE_LED);
     }
#elif defined(HAS_VIKI_INTERFACE)
     ((VikiInterface &)Motherboard::getBoard().getInterfaceBoard().lcd).setLED(on);
#elif defined(HAS_VIKI2_INTERFACE)
     ((Viki2Interface &)Motherboard::getBoard().getInterfaceBoard().lcd).setLED(on);
#endif
}

#ifdef HAS_HBP_INDICATOR
void InterfaceBoard::setHBPIndicator(bool on) {
#if defined(HAS_VIKI_INTERFACE)
	((VikiInterface &)Motherboard::getBoard().getInterfaceBoard().lcd).setHBPIndicator(on);
#elif defined(HAS_VIKI2_INTERFACE)
	((Viki2Interface &)Motherboard::getBoard().getInterfaceBoard().lcd).setHBPIndicator(on);
#endif
}
#endif

#ifdef HAS_TOOL_INDICATOR
void InterfaceBoard::setToolIndicator(uint8_t toolID, bool on) {
#if defined(HAS_VIKI_INTERFACE)
	((VikiInterface &)Motherboard::getBoard().getInterfaceBoard().lcd).setToolIndicator(toolID, on);
#elif defined(HAS_VIKI2_INTERFACE)
	((Viki2Interface &)Motherboard::getBoard().getInterfaceBoard().lcd).setToolIndicator(toolID, on);	
#endif
}
#endif

#ifdef HAS_COOLING_FAN_INDICATOR
void InterfaceBoard::setCoolingFanIndicator(bool on) {
#if defined(HAS_VIKI2_INTERFACE)
	((Viki2Interface &)Motherboard::getBoard().getInterfaceBoard().lcd).setCoolingFanIndicator(on);	
#endif
}
#endif

void InterfaceBoard::doInterrupt() {
     buttons.scanButtons();
}

bool InterfaceBoard::isButtonPressed(ButtonArray::ButtonName button) {
        bool buttonPressed = buttons.isButtonPressed(button);

        return buttonPressed;
}

micros_t InterfaceBoard::getUpdateRate() {
	return screenStack[screenIndex]->getUpdateRate();
}

/// push Error Message Screen
void InterfaceBoard::errorMessage(const prog_uchar *buf, bool incomplete) {
	errorMessage(buf, 0, incomplete);
}

void InterfaceBoard::errorMessage(const prog_uchar *buf1, const prog_uchar *buf2, bool incomplete){
		messageScreen->clearMessage();
		messageScreen->setXY(0,0);
		messageScreen->addMessage(buf1);
		if ( buf2 ) messageScreen->addMessage(buf2);
		messageScreen->incomplete = incomplete;
		pushScreen(messageScreen);
}

bool onboard_build = false;

void InterfaceBoard::doUpdate() {

	// If we are building, make sure we show a build menu; otherwise,
	// turn it off.
	switch(host::getHostState()) {
    case host::HOST_STATE_BUILDING_ONBOARD:
            onboard_build = true;    // fallthrough
	case host::HOST_STATE_BUILDING:
	case host::HOST_STATE_BUILDING_FROM_SD:
		if (!building ){

			// if a message screen is still active, wait until it times out to push the monitor mode screen
			// move the current screen up an index so when it pops off, it will load buildScreen
			// as desired instead of popping to main menu first
			// ie this is a push behind, instead of push on top
			if ( command::isWaiting() ||
			    ((screenStack[screenIndex] == messageScreen) && messageScreen->screenWaiting()))
			{
					if (screenIndex < SCREEN_STACK_DEPTH - 1) {
						screenIndex++;
						screenStack[screenIndex] = screenStack[screenIndex-1];
					}
					if ( screenIndex )	screenStack[screenIndex -1] = buildScreen;
					buildScreen->reset();
			}
			else
                 pushScreen(buildScreen);
			building = true;
		}
		break;
	case host::HOST_STATE_HEAT_SHUTDOWN:
		break;
	default:
		if ( building ) {
			//If we're not waiting for the message screen timeout
			if ( ! ((screenStack[screenIndex] == messageScreen) && messageScreen->screenWaiting()) ) {
				// when using onboard scrips, we want to return to the Utilites menu
				// which is one screen deep in the stack
				if( onboard_build ) {
					while(screenIndex > 1 && (! (screenStack[screenIndex]->optionsMask & IS_STICKY_MASK)))
						popScreen();
					onboard_build = false;
				}
				// else, after a build, we'll want to go back to the main menu
				else {
					while(screenIndex > 0 && (! (screenStack[screenIndex]->optionsMask & IS_STICKY_MASK)))
						popScreen();
					if ( buildFinishedScreen && !host::buildWasCancelled )
						pushScreen(buildFinishedScreen);
				}
				building = false;
			}
		}

		break;
	}

    bool forceRedraw = false;
    static ButtonArray::ButtonName button;
    if(!screen_locked){
        if (buttons.getButton(button)) {
	    if((((1<<button) & waitingMask) != 0) &&
                      (!(screenStack[screenIndex]->optionsMask & IS_CANCEL_SCREEN_MASK))){
                 waitingMask = 0;
            } else {
	       if ((button != ButtonArray::RIGHT) ||
		   (screenStack[screenIndex]->optionsMask & _BV(ButtonArray::RIGHT))) {
		     screenStack[screenIndex]->notifyButtonPressed(button);
		     if((screenStack[screenIndex]->optionsMask & CONTINUOUS_BUTTONS_MASK) & _BV((uint8_t)button)) {
			  buttonRepetitions ++;
			  lockoutButtonRepetitionsClear = true;
			  button_timeout.start(ButtonArray::ContinuousButtonRepeatDelay);
		     }
		     else buttonRepetitions = 0;
		}
		else {
		     // RIGHT was pressed and this screen does not consume
		     // the RIGHT button....
		     // Reset the LCD
		     lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
		     forceRedraw = true;
		}
            }
            // reset user input timeout when buttons are pressed
            Motherboard::getBoard().resetUserInputTimeout();
        }
	else if ( ! lockoutButtonRepetitionsClear ) {
		buttonRepetitions = 0;
	}

        // clear button press if button timeout occurs in continuous press mode
        if(button_timeout.hasElapsed())
        {
            buttons.clearButtonPress();
            button_timeout.clear();
	    lockoutButtonRepetitionsClear = false;
        }

        // update build data
        screenStack[screenIndex]->update(lcd, forceRedraw);
    }
}


// add a screen to the stack but don't refresh the screen
void InterfaceBoard::pushNoUpdate(Screen *newScreen){
	if (screenIndex < SCREEN_STACK_DEPTH - 1) {
		screenIndex++;
		screenStack[screenIndex] = newScreen;
	}
	screenStack[screenIndex]->reset();
}

// push screen to stack and call update
void InterfaceBoard::pushScreen(Screen* newScreen) {
	if (screenIndex < SCREEN_STACK_DEPTH - 1) {
		screenIndex++;
		screenStack[screenIndex] = newScreen;
	}
	buttons.setButtonDelay(ButtonArray::SlowDelay);
	screenStack[screenIndex]->reset();
	screenStack[screenIndex]->update(lcd, true);
}

void InterfaceBoard::popScreen() {

	// Don't allow the root menu to be removed.
	if (screenIndex > 0) {
		screenIndex--;
	}
	buttons.setButtonDelay(ButtonArray::SlowDelay);
	screenStack[screenIndex]->update(lcd, true);
}


/// Tell the interface board that the system is waiting for a button push
/// corresponding to one of the bits in the button mask. The interface board
/// will not process button pushes directly until one of the buttons in the
/// mask is pushed.
void InterfaceBoard::waitForButton(uint8_t button_mask) {
  waitingMask = button_mask;
}

/// Check if the expected button push has been made. If waitForButton was
/// never called, always return true.
bool InterfaceBoard::buttonPushed() {
  return waitingMask == 0;
}

/// Returns the number of times a button has been held down
/// Only applicable to continuous buttons
uint16_t InterfaceBoard::getButtonRepetitions(void) {
	return buttonRepetitions;
}

#endif
