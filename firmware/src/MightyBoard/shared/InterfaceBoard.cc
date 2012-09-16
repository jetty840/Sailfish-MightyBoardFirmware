#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "LiquidCrystalSerial.hh"
#include "Host.hh"
#include "Timeout.hh"
#include "Command.hh"
#include "Motherboard.hh"

#if defined HAS_INTERFACE_BOARD

bool onboard_build = false;

InterfaceBoard::InterfaceBoard(ButtonArray& buttons_in,
                               LiquidCrystalSerial& lcd_in,
                               const Pin& gled_in,
                               const Pin& rled_in):
        lcd(lcd_in),
        buttons(buttons_in),
		waitingMask(0)
{
  LEDs[0] = gled_in;
  LEDs[1] = rled_in;
  buildPercentage = 101;
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
  waitingMask = 0;
  pushScreen(&mainScreen);
  screen_locked = false;
  onboard_build = false;
  onboard_start_idx = 1;
}

void InterfaceBoard::resetLCD() {

	lcd.begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
}

void InterfaceBoard::doInterrupt() {
	buttons.scanButtons();
}

micros_t InterfaceBoard::getUpdateRate() {
	return screenStack[screenIndex]->getUpdateRate();
}

/// push Error Message Screen
void InterfaceBoard::errorMessage(char buf[]){

		messageScreen.clearMessage();
		messageScreen.setXY(0,0);
		messageScreen.addMessage(buf);
		messageScreen.WaitForUser(true);
		if(screenStack[screenIndex] != &messageScreen){
			pushScreen(&messageScreen);
		}else{
			screenStack[screenIndex]->update(lcd, true);
		}
}

MessageScreen * InterfaceBoard::GetMessageScreen(){
  return &messageScreen;
}

/// pop Error Message Screen
void InterfaceBoard::DoneWithMessage(){

		messageScreen.WaitForUser(false);
		if(screenStack[screenIndex] == &messageScreen){
			popScreen();
		}
}

/// push a local screen
void InterfaceBoard::queueScreen(ScreenType screen){

	
	switch (screen){
		case BUILD_FINISHED:
			pushScreen(&buildFinished);
			break;
		case MESSAGE_SCREEN:
			pushScreen(&messageScreen);
			break;
		default:
			break;
		}
	
}

/// record screen stack index when onboard script is started so we can return there on finish
void InterfaceBoard::RecordOnboardStartIdx(){
	onboard_start_idx = screenIndex;
}

void InterfaceBoard::doUpdate() {

		// If we are building, make sure we show a build menu; otherwise,
		// turn it off.
		switch(host::getHostState()) {
		case host::HOST_STATE_BUILDING_ONBOARD:
				onboard_build = true;
		case host::HOST_STATE_BUILDING:
		case host::HOST_STATE_BUILDING_FROM_SD:
			if (!building ){
				/// remove the build finished screen if the user has not done so
				if(screenStack[screenIndex] == &buildFinished){
					popScreen();
				}
				
				// if a screen is waiting for user input, don't push the build screen on top
				// wait until the screen is finished.
        // we do not push the build screen at all for utility scripts that don't require heating.
				if (utility::showMonitor()){
					if(!(screenStack[screenIndex]->screenWaiting() || command::isWaiting()))
					{
						pushScreen(&buildScreen);
						building = true;
					}
				}else{
					building = true;
				}
				
			}
			break;
		case host::HOST_STATE_HEAT_SHUTDOWN:
			break;
		default:
			if (building) {
        
				if(!(screenStack[screenIndex]->screenWaiting())){	
					
					// when using onboard scrips, we want to return to whichever screen we launched the script from
					if(onboard_build){	
						while(screenIndex > onboard_start_idx){
							popScreenQuick();
						}
						screenStack[screenIndex]->update(lcd, true);
						onboard_build = false;
					}
					// else, after a build, we'll want to go back to the main menu
					else{
						while(screenIndex > 0){
							popScreenQuick();
						}
						screenStack[screenIndex]->update(lcd, true);
					}
					building = false;
				}
			}	
			break;
		}
	
	/// check for button pushes and send these to the active screen
	
    static ButtonArray::ButtonName button;

    if(!screen_locked){
        if (buttons.getButton(button)) {
            if (button == ButtonArray::RESET){
                host::stopBuild();
                return;
            // respond to button press if waiting
            // pass on to screen if a cancel screen is active
            } else if((((1<<button) & waitingMask) != 0) && 
                      (!screenStack[screenIndex]->isCancelScreen())){
                 waitingMask = 0;
            } else if (button == ButtonArray::EGG){
                pushScreen(&snake);
            } else {
                screenStack[screenIndex]->notifyButtonPressed(button);
            }
            // reset user input timeout when buttons are pressed
            Motherboard::getBoard().resetUserInputTimeout();

        }
       
        // update build data
        screenStack[screenIndex]->setBuildPercentage(buildPercentage);	
        screenStack[screenIndex]->update(lcd, false);
    }
}

//void InterfaceBoard::update(){
//	screenStack[screenIndex]->update(lcd, true);
//}

// push screen to stack and call update
void InterfaceBoard::pushScreen(Screen* newScreen) {
  screen_locked = true;
	if (screenIndex < SCREEN_STACK_DEPTH - 1) {
		screenIndex++;
		screenStack[screenIndex] = newScreen;
	}
  buttons.setButtonDelay(ButtonArray::SlowDelay);
	Motherboard::getBoard().StopProgressBar();
	screenStack[screenIndex]->reset();
  screen_locked = false;
	screenStack[screenIndex]->update(lcd, true);
}

void InterfaceBoard::setBuildPercentage(uint8_t percent){
	
	if(percent < 100){
		buildPercentage = percent;
	}
}

void InterfaceBoard::popScreen() {
	
	screenStack[screenIndex]->pop();
	// Don't allow the root menu to be removed.
	if (screenIndex > 0) {
		screenIndex--;
	}
 
  buttons.setButtonDelay(ButtonArray::SlowDelay);
	screenStack[screenIndex]->update(lcd, true);
}

/// pop screen without refreshing the new head screen
void InterfaceBoard::popScreenQuick() {
	
	screenStack[screenIndex]->pop();
	// Don't allow the root menu to be removed.
	if (screenIndex > 0) {
		screenIndex--;
	}
  buttons.setButtonDelay(ButtonArray::SlowDelay);

}

// turn interface LEDs on
void InterfaceBoard::setLED(uint8_t id, bool on){
	LEDs[id].setValue(on);
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

#endif
