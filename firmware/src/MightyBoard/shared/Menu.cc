#include "Menu.hh"
#include "Menu.hh"
#include "Configuration.hh"

// TODO: Kill this, should be hanlded by build system.
#ifdef HAS_INTERFACE_BOARD

#include "Steppers.hh"
#include "Commands.hh"
#include "Errors.hh"
//#include "Tool.hh"
#include "Host.hh"
#include "Timeout.hh"
#include "InterfaceBoard.hh"
#include "Interface.hh"
#include <util/delay.h>
#include <stdlib.h>
#include "SDCard.hh"
#include <string.h>
#include "Version.hh"


#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

void SplashScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar splash3[] = "                    ";
	static PROGMEM prog_uchar splash1[] = "  The Replicator    ";
	static PROGMEM prog_uchar splash2[] = "    ----------      ";
	static PROGMEM prog_uchar splash4[] = "Firmware Version 4. ";


	if (forceRedraw) {
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(splash1);

		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(splash2);

		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(splash3);

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(splash4);
		lcd.setCursor(19,3);
		lcd.writeInt((uint16_t)firmware_version,1);
	}
	else {
	//	 The machine has started, so we're done!
                interface::popScreen();
        }
}

void SplashScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	// We can't really do anything, since the machine is still loading, so ignore.
	switch (button) {
		case ButtonArray::CENTER:
           interface::popScreen();
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void SplashScreen::reset() {
	
}

void HeaterTestScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar splash1[] = "   Heater Test On   ";
	static PROGMEM prog_uchar splash2[] = "Press Center to Quit";
	static PROGMEM prog_uchar splash3[] = "This test takes ten ";
	static PROGMEM prog_uchar splash4[] = "    seconds         ";
	
	static PROGMEM prog_uchar splash1a[] = "    FAIL!           ";
	static PROGMEM prog_uchar splash2a[] = "    SUCCESS!        ";
	static PROGMEM prog_uchar splash3a[] = "connected correctly ";
	static PROGMEM prog_uchar splash4a[] = "Heaters are not     ";
	static PROGMEM prog_uchar splash5a[] = "                    ";	

	if(heater_timeout.hasElapsed())
	{
		if(Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().get_current_temperature() > 45)
					heater_failed = true;
				
		Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);

		if(heater_failed)
		{
			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(splash1a);
			
			lcd.setCursor(0,1);
			lcd.writeFromPgmspace(splash4a);

			lcd.setCursor(0,2);
			lcd.writeFromPgmspace(splash3a);
		}
		else
		{
			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(splash2a);
			
			lcd.setCursor(0,1);
			lcd.writeFromPgmspace(splash5a);

			lcd.setCursor(0,2);
			lcd.writeFromPgmspace(splash5a);
		}

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(splash2);
	}

	if (forceRedraw) {
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(splash1);

		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(splash2);

		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(splash3);

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(splash4);
	}
}

void HeaterTestScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	switch (button) {
		case ButtonArray::CENTER:
			// set heater back to zero
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
           interface::popScreen();
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void HeaterTestScreen::reset() {
	Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(60);
	heater_failed = false;
	heater_timeout.start(12000000); /// ten second timeout
}

void WelcomeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar splash1[] = "   The Replicator   ";
	static PROGMEM prog_uchar splash2[] = "      Welcome!      ";
	static PROGMEM prog_uchar splash3[] = "  Press the Center  ";
	static PROGMEM prog_uchar splash4[] = " Button to Continue ";
	
	static PROGMEM prog_uchar splash1a[] = " The next menu items";
	static PROGMEM prog_uchar splash2a[] = "  will set up your  ";
	static PROGMEM prog_uchar splash3a[] = "  bot and get your  ";
	static PROGMEM prog_uchar splash4a[] = " first print going! ";	


	if (forceRedraw) {
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(splash1);

		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(splash2);

		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(splash3);

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(splash4);
	}
}

void WelcomeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	switch (button) {
		case ButtonArray::CENTER:
           interface::popScreen();
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void WelcomeScreen::reset() {
	
}

void MessageScreen::addMessage(CircularBuffer& buf) {
	char c = buf.pop();
	while (c != '\0' && cursor < BUF_SIZE && buf.getLength() > 0) {
		message[cursor++] = c;
		c = buf.pop();
	}
	// ensure that message is always null-terminated
	if (cursor == BUF_SIZE) {
		message[BUF_SIZE-1] = '\0';
	} else {
		message[cursor] = '\0';
		// decrement cursor to prepare for subsequent
		// extensions to the message
		cursor--;
	}
	needsRedraw = true;
}

void MessageScreen::clearMessage() {
	x = y = 0;
	message[0] = '\0';
	cursor = 0;
	needsRedraw = true;
	timeout = Timeout();
}

void MessageScreen::setTimeout(uint8_t seconds) {
	timeout.start((micros_t)seconds * 1000 * 1000);
}

void MessageScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	char* b = message;
	int ycursor = y;
	if (timeout.hasElapsed()) {
		interface::popScreen();
		return;
	}
	if (forceRedraw || needsRedraw) {
		needsRedraw = false;
		lcd.clear();
		while (*b != '\0') {
			lcd.setCursor(x,ycursor);
			b = lcd.writeLine(b);
			if (*b == '\n') {
				b++;
				ycursor++;
			}
		}
	}
}

void MessageScreen::reset() {
}

void MessageScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	// Integrate with button wait here
}

void JogMode::reset() {
	jogDistance = DISTANCE_LONG;
	distanceChanged = modeChanged = false;
	JogModeScreen = JOG_MODE_X;
}


void JogMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar jog1[]  = "     Jog mode       ";
	static PROGMEM prog_uchar jog2x[] = "        X+          ";
	static PROGMEM prog_uchar jog3x[] = "      (Back)   Y->  ";
	static PROGMEM prog_uchar jog4x[] = "        X-          ";
	
	static PROGMEM prog_uchar jog2y[] = "        Y+          ";
	static PROGMEM prog_uchar jog3y[] = "  <-X (Back)  Z->   ";
	static PROGMEM prog_uchar jog4y[] = "        Y-          ";
	
	static PROGMEM prog_uchar jog2z[] = "        Z+          ";
	static PROGMEM prog_uchar jog3z[] = "  <-Y (Back)        ";
	static PROGMEM prog_uchar jog4z[] = "        Z-          ";

	static PROGMEM prog_uchar distanceShort[] = "SHORT";
	static PROGMEM prog_uchar distanceLong[] = "LONG";

	if (forceRedraw || distanceChanged || modeChanged) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(jog1);

	/*	switch (jogDistance) {
		case DISTANCE_SHORT:
			lcd.writeFromPgmspace(distanceShort);
			break;
		case DISTANCE_LONG:
			lcd.writeFromPgmspace(distanceLong);
			break;
		}
   */
		switch (JogModeScreen){
			case JOG_MODE_X:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(jog2x);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(jog3x);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(jog4x);
				break;
			case JOG_MODE_Y:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(jog2y);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(jog3y);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(jog4y);
				break;
			case JOG_MODE_Z:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(jog2z);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(jog3z);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(jog4z);
				break;
		}

		distanceChanged = false;
		modeChanged = false;
	}
}

void JogMode::jog(ButtonArray::ButtonName direction) {
	Point position = steppers::getPosition();

	int32_t interval = 2000;
	uint8_t steps;

	switch(jogDistance) {
	case DISTANCE_SHORT:
		steps = 20;
		break;
	case DISTANCE_LONG:
		steps = 4000;
		break;
	}

	if(JogModeScreen == JOG_MODE_X)
	{
		switch(direction) {
			case ButtonArray::RIGHT:
			JogModeScreen = JOG_MODE_Y;
			modeChanged = true;
			break;
			case ButtonArray::DOWN:
			position[0] -= steps;
			break;
			case ButtonArray::UP:
			position[0] += steps;
			break;
		}
	}
	else if (JogModeScreen == JOG_MODE_Y)
	{
		switch(direction) {
			case ButtonArray::RIGHT:
			JogModeScreen = JOG_MODE_Z;
			modeChanged = true;
			break;
			case ButtonArray::LEFT:
			JogModeScreen = JOG_MODE_X;
			modeChanged = true;
			break;
			case ButtonArray::DOWN:
			position[1] -= steps;
			break;
			case ButtonArray::UP:
			position[1] += steps;
			break;
		}
			
	}
	else if (JogModeScreen == JOG_MODE_Z)
	{
		switch(direction) {
			case ButtonArray::LEFT:
			JogModeScreen = JOG_MODE_Y;
			modeChanged = true;
			break;
			case ButtonArray::DOWN:
			position[2] -= steps;
			break;
			case ButtonArray::UP:
			position[2] += steps;
			break;
		}
	}

	steppers::setTarget(position, interval);
}

void JogMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
		case ButtonArray::CENTER:
           interface::popScreen();
           for(int i = 0; i < STEPPER_COUNT; i++)
			steppers::enableAxis(i, false);
		break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
		jog(button);
		break;
	}
}


void SnakeMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar gameOver[] =  "GAME OVER!";

	// If we are dead, restart the game.
	if (!snakeAlive) {
		reset();
		forceRedraw = true;
	}

	if (forceRedraw) {
		lcd.clear();

		for (uint8_t i = 0; i < snakeLength; i++) {
			lcd.setCursor(snakeBody[i].x, snakeBody[i].y);
			lcd.write('O');
		}
	}

	// Always redraw the apple, just in case.
	lcd.setCursor(applePosition.x, applePosition.y);
	lcd.write('*');

	// First, undraw the snake's tail
	lcd.setCursor(snakeBody[snakeLength-1].x, snakeBody[snakeLength-1].y);
	lcd.write(' ');

	// Then, shift the snakes body parts back, deleting the tail
	for(int8_t i = snakeLength-1; i >= 0; i--) {
		snakeBody[i+1] = snakeBody[i];
	}

	// Create a new head for the snake (this causes it to move forward)
	switch(snakeDirection)
	{
	case DIR_EAST:
		snakeBody[0].x = (snakeBody[0].x + 1) % LCD_SCREEN_WIDTH;
		break;
	case DIR_WEST:
		snakeBody[0].x = (snakeBody[0].x +  LCD_SCREEN_WIDTH - 1) % LCD_SCREEN_WIDTH;
		break;
	case DIR_NORTH:
		snakeBody[0].y = (snakeBody[0].y + LCD_SCREEN_HEIGHT - 1) % LCD_SCREEN_HEIGHT;
		break;
	case DIR_SOUTH:
		snakeBody[0].y = (snakeBody[0].y + 1) % LCD_SCREEN_HEIGHT;
		break;
	}

	// Now, draw the snakes new head
	lcd.setCursor(snakeBody[0].x, snakeBody[0].y);
	lcd.write('O');

	// Check if the snake has run into itself
	for (uint8_t i = 1; i < snakeLength; i++) {
		if (snakeBody[i].x == snakeBody[0].x
			&& snakeBody[i].y == snakeBody[0].y) {
			snakeAlive = false;

			lcd.setCursor(1,1);
			lcd.writeFromPgmspace(gameOver);
			updateRate = 5000L * 1000L;
		}
	}

	// If the snake just ate an apple, increment count and make new apple
	if (snakeBody[0].x == applePosition.x
			&& snakeBody[0].y == applePosition.y) {
		applesEaten++;

		if(applesEaten % APPLES_BEFORE_GROW == 0) {
			snakeLength++;
			updateRate -= 5L * 1000L;
		}

		applePosition.x = rand()%LCD_SCREEN_WIDTH;
		applePosition.y = rand()%LCD_SCREEN_HEIGHT;

		lcd.setCursor(applePosition.x, applePosition.y);
		lcd.write('*');
	}
}

void SnakeMode::reset() {
	updateRate = 150L * 1000L;
	snakeDirection = DIR_EAST;
	snakeLength = 3;
	applesEaten = 0;
	snakeAlive = true;

	// Put the snake in an initial position
	snakeBody[0].x = 2; snakeBody[0].y = 1;
	snakeBody[1].x = 1; snakeBody[1].y = 1;
	snakeBody[2].x = 0; snakeBody[2].y = 1;

	// Put the apple in an initial position (this could collide with the snake!)
	applePosition.x = rand()%LCD_SCREEN_WIDTH;
	applePosition.y = rand()%LCD_SCREEN_HEIGHT;
	
	
}


void SnakeMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::DOWN:
		snakeDirection = DIR_SOUTH;
		break;
        case ButtonArray::UP:
		snakeDirection = DIR_NORTH;
		break;
        case ButtonArray::LEFT:
		snakeDirection = DIR_WEST;
		break;
        case ButtonArray::RIGHT:
		snakeDirection = DIR_EAST;
		break;
        case ButtonArray::CENTER:
                interface::popScreen();
		break;
	}
}


void MonitorMode::reset() {
	updatePhase = 0;
	
}
void MonitorMode::setBuildPercentage(uint8_t percent){

	buildPercentage = percent;
}

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar build_percent[] =    "                ---%";
	static PROGMEM prog_uchar extruder1_temp[] =   "Right Tool: ---/---C";
	static PROGMEM prog_uchar extruder2_temp[] =   "Left Tool:  ---/---C";
	static PROGMEM prog_uchar platform_temp[]  =   "Platform:   ---/---C";

	if (forceRedraw) {
		lcd.clear();
		lcd.setCursor(0,0);
		switch(host::getHostState()) {
		case host::HOST_STATE_READY:
			lcd.writeString(host::getMachineName());
			break;
		case host::HOST_STATE_BUILDING:
		case host::HOST_STATE_BUILDING_FROM_SD:
			lcd.writeFromPgmspace(build_percent);
			lcd.setCursor(0,0);
			lcd.writeString(host::getBuildName());
			break;
		case host::HOST_STATE_ERROR:
			lcd.writeString("error!");
			break;
		}
		

		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(extruder1_temp);
		
		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(extruder2_temp);

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(platform_temp);

	} else {
	}


	OutPacket responsePacket;
	Motherboard& board = Motherboard::getBoard();
	uint16_t data;

	// Redraw tool info
	switch (updatePhase) {
	case 0:
		lcd.setCursor(12,1);
			data = board.getExtruderBoard(0).getExtruderHeater().get_current_temperature();
			if(data == DEFAULT_THERMOCOUPLE_VAL)
				lcd.writeString(" NA");
			else
				lcd.writeInt(data,3);
		break;

	case 1:
		lcd.setCursor(16,1);
			data = board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
			lcd.writeInt(data,3);
		break;
	case 2:
		lcd.setCursor(12,2);
			data = board.getExtruderBoard(1).getExtruderHeater().get_current_temperature();
			if(data == DEFAULT_THERMOCOUPLE_VAL)
				lcd.writeString(" NA");
			else
				lcd.writeInt(data,3);
		break;
	case 3:
		lcd.setCursor(16,2);
			data = board.getExtruderBoard(1).getExtruderHeater().get_set_temperature();
			lcd.writeInt(data,3);

		break;

	case 4:
		lcd.setCursor(12,3);
			data = board.getPlatformHeater().get_current_temperature();
			if(data == DEFAULT_THERMOCOUPLE_VAL)
				lcd.writeString(" NA");
			else
				lcd.writeInt(data,3);
		break;

	case 5:
		lcd.setCursor(16,3);
			data = board.getPlatformHeater().get_set_temperature();
			lcd.writeInt(data,3);
		break;
	}

	updatePhase++;
	if (updatePhase > 5) {
		updatePhase = 0;
	}
}

void MonitorMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::CENTER:
		switch(host::getHostState()) {
		case host::HOST_STATE_BUILDING:
		case host::HOST_STATE_BUILDING_FROM_SD:
                        interface::pushScreen(&cancelBuildMenu);
			break;
		default:
                        interface::popScreen();
			break;
		}
	}
}


void Menu::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar blankLine[] =  "                ";

	// Do we need to redraw the whole menu?
	if ((itemIndex/LCD_SCREEN_HEIGHT) != (lastDrawIndex/LCD_SCREEN_HEIGHT)
			|| forceRedraw ) {
		// Redraw the whole menu
		lcd.clear();

		for (uint8_t i = 0; i < LCD_SCREEN_HEIGHT; i++) {
			// Instead of using lcd.clear(), clear one line at a time so there
			// is less screen flickr.

			if (i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT +1 > itemCount) {
				break;
			}

			lcd.setCursor(1,i);
			// Draw one page of items at a time
			drawItem(i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT, lcd);
		}
	}
	else {
		// Only need to clear the previous cursor
		lcd.setCursor(0,(lastDrawIndex%LCD_SCREEN_HEIGHT));
		lcd.write(' ');
	}

	lcd.setCursor(0,(itemIndex%LCD_SCREEN_HEIGHT));
	lcd.write('>');
	lastDrawIndex = itemIndex;
}

void Menu::reset() {
	firstItemIndex = 0;
	itemIndex = 0;
	lastDrawIndex = 255;
	
	resetState();
}

void Menu::resetState() {
}

void Menu::handleSelect(uint8_t index) {
}

void Menu::handleCancel() {
	// Remove ourselves from the menu list
        interface::popScreen();
}

void Menu::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::CENTER:
		handleSelect(itemIndex);
		break;
     //   case ButtonArray::CANCEL:
	//	handleCancel();
	//	break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
		// increment index
		break;
        case ButtonArray::UP:
        if (itemIndex > firstItemIndex) {
			itemIndex--;
		}
		break;
        case ButtonArray::DOWN:
		// decrement index
		if (itemIndex < itemCount - 1) {
			itemIndex++;
		}
		break;
	}
}

StartupMenu::StartupMenu(){
	itemCount = 3;
	reset();
}
void StartupMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar plastic[] = "Load Plastic";
	static PROGMEM prog_uchar axes[]   =   "Check Axes";
	static PROGMEM prog_uchar build[] =   "Build from SD";
	
	switch (index) {
	case 0:
		lcd.writeFromPgmspace(plastic);
		break;
	case 1:
		lcd.writeFromPgmspace(axes);
		break;
	case 2:
		lcd.writeFromPgmspace(build);
		break;
	}
}

void StartupMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// Show monitor build screen
                       // interface::pushScreen(&plasticMode);
                       interface::popScreen();
			break;
		case 1:
			// Show build from SD screen
                      //  interface::pushScreen(&axesMenu);
			break;
		case 2:
			// Show build from SD screen
                     //   interface::pushScreen(&sdMenu);
			break;
		}
}


CancelBuildMenu::CancelBuildMenu() {
	itemCount = 4;
	reset();
}

void CancelBuildMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void CancelBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar cancel[] = "Cancel ?";
	static PROGMEM prog_uchar yes[] =   "Yes";
	static PROGMEM prog_uchar no[]   =   "No";

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(cancel);
		break;
	case 1:
		break;
	case 2:
		lcd.writeFromPgmspace(yes);
		break;
	case 3:
		lcd.writeFromPgmspace(no);
		break;
	}
}

void CancelBuildMenu::handleSelect(uint8_t index) {
	switch (index) {
	case 2:
		// Cancel build, returning to whatever menu came before monitor mode.
		// TODO: Cancel build.
		interface::popScreen();
		host::stopBuild();
		break;
	case 3:
		// Don't cancel, just close dialog.
                interface::popScreen();
		break;
	}
}


MainMenu::MainMenu() {
	itemCount = 9;
	reset();
}

void MainMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar monitor[] = "Monitor Mode";
	static PROGMEM prog_uchar build[] =   "Build from SD";
	static PROGMEM prog_uchar jog[]   =   "Jog Mode";
	static PROGMEM prog_uchar calibration[] = "Calibrate Axes";
	static PROGMEM prog_uchar home_axes[] = "Home Axes";
	static PROGMEM prog_uchar load_filament[] = "Load Filament";
	static PROGMEM prog_uchar startup[] = "Run Startup Script";
	static PROGMEM prog_uchar heater_test[] = "Heater Test";
	static PROGMEM prog_uchar snake[] =   "Snake Game";

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(monitor);
		break;
	case 1:
		lcd.writeFromPgmspace(build);
		break;
	case 2:
		lcd.writeFromPgmspace(jog);
		break;
	case 3:
		lcd.writeFromPgmspace(home_axes);
		break;
	case 4:
		lcd.writeFromPgmspace(heater_test);
		break;
	case 5:
		lcd.writeFromPgmspace(load_filament);
		break;
	case 6:
		lcd.writeFromPgmspace(startup);
		break;
	case 7:
		//do nothing
		break;
	case 8:
		lcd.writeFromPgmspace(snake);
		break;
	}
}

void MainMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// Show monitor build screen
                        interface::pushScreen(&monitorMode);
			break;
		case 1:
			// Show build from SD screen
                        interface::pushScreen(&sdMenu);
			break;
		case 2:
			// Show jog mode
                        interface::pushScreen(&jogger);
			break;
		case 3:
			// home axes script
                        interface::pushScreen(&home);
			break;
		case 4:
			// calibration script
                        interface::pushScreen(&heater);
			break;
		case 5:
			// load filament script
                        interface::pushScreen(&filament);
			break;
		case 6:
			// run startup script
                        interface::pushScreen(&welcome);
			break;
		case 8:
			// Snake GAME!
                        interface::pushScreen(&snake);
			break;
		}
}

SDSpecialBuild::SDSpecialBuild(){
	buildFailed = false;
	reset();
}

void SDSpecialBuild::resetState() {
}

void SDSpecialBuild::reset(){
	resetState();
	if(!startBuild())
		buildFailed = true;
}

void SDSpecialBuild::update(LiquidCrystalSerial& lcd, bool forceRedraw){
	static PROGMEM prog_uchar SDone[] =    "An SD card with     ";
	static PROGMEM prog_uchar SDtwo[] =    "MakerBot scripts is ";
	static PROGMEM prog_uchar SDthree[] =   "required.  See      ";
	static PROGMEM prog_uchar SDfour[]  =   "makerbot.com/repSD  ";

	if(forceRedraw)
	{
		if (buildFailed) {
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(SDone);

			lcd.setCursor(0,1);
			lcd.writeFromPgmspace(SDtwo);
			
			lcd.setCursor(0,2);
			lcd.writeFromPgmspace(SDthree);

			lcd.setCursor(0,3);
			lcd.writeFromPgmspace(SDfour);

		}
	}
}

bool SDSpecialBuild::startBuild(){

	if (host::getHostState() != host::HOST_STATE_READY) {
		return false;
	}
		
	char* buildName = host::getBuildName();

    strcpy(buildName, buildType);

    sdcard::SdErrorCode e;
	e = host::startBuildFromSD();
	
	if (e != sdcard::SD_SUCCESS) {
		return false;
	}
	
	return true;
}

void SDSpecialBuild::notifyButtonPressed(ButtonArray::ButtonName button){
	switch (button) {
		case ButtonArray::CENTER:
           interface::popScreen();
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void HomeAxes::resetState()
{
	strcpy(buildType, "Home Axes.s3g");
}
void Calibration::resetState()
{
	strcpy(buildType, "Calibrate.s3g");
}
void LoadFilament::resetState()
{
	strcpy(buildType, "Load Filament.s3g");
}

SDMenu::SDMenu() {
	reset();
}

void SDMenu::resetState() {
	cardNotFound = false;
	itemCount = countFiles() + 1;
	
}

// Count the number of files on the SD card
uint8_t SDMenu::countFiles() {
	uint8_t count = 0;

	sdcard::SdErrorCode e;	

	// First, reset the directory index
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		cardNotFound = true;
		return 0;
	}

	const int MAX_FILE_LEN = 2;
	char fnbuf[MAX_FILE_LEN];

	// Count the files
	do {
		e = sdcard::directoryNextEntry(fnbuf,MAX_FILE_LEN);
		if (fnbuf[0] == '\0') {
			break;
		}

		// If it's a dot file, don't count it.
		if (fnbuf[0] == '.') {
		}
		else {
			count++;
		}
	} while (e == sdcard::SD_SUCCESS);

	// TODO: Check for error again?

	return count;
}

bool SDMenu::getFilename(uint8_t index, char buffer[], uint8_t buffer_size) {
	sdcard::SdErrorCode e;

	// First, reset the directory list
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
                return false;
	}


	for(uint8_t i = 0; i < index+1; i++) {
		// Ignore dot-files
		do {
			e = sdcard::directoryNextEntry(buffer,buffer_size);
			if (buffer[0] == '\0') {
                                return false;
			}
		} while (e == sdcard::SD_SUCCESS && buffer[0] == '.');

		if (e != sdcard::SD_SUCCESS) {
                        return false;
		}
	}

        return true;
}

void SDMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	 static PROGMEM prog_uchar exit[] =   "exit menu";
	 static PROGMEM prog_uchar noCard[] = "No SD card found";
       
       // print error message if no SD card found;
       if(cardNotFound == true) {
               lcd.writeFromPgmspace(noCard);
			return;
		}
		// print last line for SD card - an exit option
       if (index >= itemCount - 1) {
               lcd.writeFromPgmspace(exit);
			return;
		}

	const uint8_t MAX_FILE_LEN = LCD_SCREEN_WIDTH;
	char fnbuf[MAX_FILE_LEN];

        if ( !getFilename(index, fnbuf, MAX_FILE_LEN) ) {
                // TODO: report error
		return;
	}

	uint8_t idx;
	for (idx = 0; (idx < MAX_FILE_LEN) && (fnbuf[idx] != 0); idx++) {
		lcd.write(fnbuf[idx]);
	}
}

void SDMenu::handleSelect(uint8_t index) {
	
	if(index >= itemCount -1)
	{
		interface::popScreen();
		return;
	}
	if (host::getHostState() != host::HOST_STATE_READY) {
		// TODO: report error
		return;
	}
		
	char* buildName = host::getBuildName();

    if ( !getFilename(index, buildName, host::MAX_FILE_LEN) ) {
		// TODO: report error
		return;
	}

    sdcard::SdErrorCode e;
	e = host::startBuildFromSD();
	
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return;
	}
}

#endif
