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
#include "UtilityScripts.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>
#include "RGB_LED.hh"


#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

bool ready_fail = false;

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


HeaterPreheat::HeaterPreheat(){
	itemCount = 4;
	reset();
}

void HeaterPreheat::resetState(){
    uint8_t heatSet = eeprom::getEeprom8(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET, 0);
	_rightActive = (heatSet & (1 << HEAT_MASK_RIGHT)) != 0;
    _platformActive = (heatSet & (1 << HEAT_MASK_PLATFORM)) != 0;
	_leftActive = (heatSet & (1 << HEAT_MASK_LEFT)) != 0;
}

void HeaterPreheat::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar go[] = "Preheat!";
	static PROGMEM prog_uchar right[] = "Right Tool";
	static PROGMEM prog_uchar left[] = "Left Tool";
	static PROGMEM prog_uchar platform[] = "Platform";
    static PROGMEM prog_uchar tool[] = "Tool";
	
	switch (index) {
	case 0:
		lcd.writeFromPgmspace(go);
		break;
	case 1:
		lcd.writeFromPgmspace(right);
		lcd.setCursor(13,1);
		if(_rightActive)
			lcd.writeString("ON");
		else
			lcd.writeString("OFF");
		break;
	case 2:
		lcd.writeFromPgmspace(left);
		lcd.setCursor(13,2);
		if(_leftActive)
			lcd.writeString("ON");
		else
			lcd.writeString("OFF");
		break;
	case 3:
		lcd.writeFromPgmspace(platform);
		lcd.setCursor(13,3);
		if(_platformActive)
			lcd.writeString("ON");
		else
			lcd.writeString("OFF");
		break;
	}
}
                   
void HeaterPreheat::storeHeatByte(){
    uint8_t heatByte = (_rightActive*(1<<HEAT_MASK_RIGHT)) + (_leftActive*(1<<HEAT_MASK_LEFT)) + (_platformActive*(1<<HEAT_MASK_PLATFORM));
    eeprom_write_byte((uint8_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET), heatByte);
}

void HeaterPreheat::handleSelect(uint8_t index) {
	int temp;
	switch (index) {
		case 0:
			temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET,0) *_rightActive; 
            Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
            temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET,0) *_leftActive;
			Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
            temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET,0) *_platformActive;
			Motherboard::getBoard().getPlatformHeater().set_target_temperature(temp);
            interface::popScreen();
            interface::pushScreen(&monitorMode);
			break;
		case 1:
			_rightActive  = !_rightActive;
            storeHeatByte();
			lineUpdate = true;
			break;
		case 2:
			_leftActive  = !_leftActive;
            storeHeatByte();
			lineUpdate = true;
			break;
		case 3:
			_platformActive = !_platformActive;
            storeHeatByte();
			lineUpdate = true;
			break;
		}
}

void WelcomeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar start[] = "   The Replicator         Welcome!                          Press M to Continue ";
	
	static PROGMEM prog_uchar explain[] = " The next few steps   will set up your    bot and get your   first print going! ";	
    
    static PROGMEM prog_uchar level[] = "We'll calibrate the build platform      to ensure it is     level               ";

    static PROGMEM prog_uchar tool_one[] = "We'll load the      plastic filament    for printing                          ";
	static PROGMEM prog_uchar tool_dual[] = "First the right tool";
    
    static PROGMEM prog_uchar tool_two[] = "We'll load the      plastic filament    for the left tool                     ";
    
    static PROGMEM prog_uchar sdmenu[] = "Awesome!            We'll go to the SD  card Menu and you   can select a print  ";
    static PROGMEM prog_uchar welcome_fail[] = "We'll go to the mainmenu. You can rerun  startup using the   Utilites menu";
    
    
	if (forceRedraw || needsRedraw) {
		lcd.setCursor(0,0);
        switch (welcomeState){
            case WELCOME_START:
                lcd.writeFromPgmspace(start);
                Motherboard::getBoard().interfaceBlink(25,15);
                 break;
            case WELCOME_EXPLAIN:
                lcd.writeFromPgmspace(explain);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(500000);
                break;
            case WELCOME_LOAD_LEVEL:
                lcd.writeFromPgmspace(level);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(500000);
                break;
            case WELCOME_LOAD_ONE:
                lcd.writeFromPgmspace(tool_one);
                if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 2){
                    lcd.setCursor(0,3);
                    lcd.writeFromPgmspace(tool_dual);
                }
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(500000);
                break;
            case WELCOME_LOAD_TWO:
                lcd.writeFromPgmspace(tool_two);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(500000);
                break;
            case WELCOME_READY:
                interface::pushScreen(&ready);
                welcomeState++;
                break;
            case WELCOME_LOAD_SD:
                if(ready_fail){
                    lcd.writeFromPgmspace(welcome_fail);
                    welcomeState++;
                }
                else
                 lcd.writeFromPgmspace(sdmenu);
                 eeprom_write_byte((uint8_t*)eeprom_offsets::FIRST_BOOT_FLAG, 1);
                 Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(500000);
                break;
            case WELCOME_DONE:
                interface::popScreen();
                break;
            
        }
        needsRedraw = false;
	}
}

void WelcomeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	Point position;
	
	switch (button) {
		case ButtonArray::CENTER:
           welcomeState++;
            switch (welcomeState){
                case WELCOME_TOOL_SELECT:
                    Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    interface::pushScreen(&tool_select);
                    break;
                case WELCOME_LEVEL:
                    welcomeState++;
                     Motherboard::getBoard().interfaceBlink(0,0);
                    host::startOnboardBuild(utility::LEVEL_PLATE);
                    break;
                case WELCOME_RIGHT_TOOL:
					position = steppers::getPosition();
					// move the z stage down
					position[2] += 4000;
					steppers::setTarget(position, 4000);
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
                        welcomeState += 2;
                    host::startOnboardBuild(utility::FILAMENT_RIGHT);
                    break;
                case WELCOME_LEFT_TOOL:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    host::startOnboardBuild(utility::FILAMENT_LEFT);
                    break;
                case WELCOME_PRINT_FROM_SD:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    interface::pushScreen(&sdmenu);
                    break;
                default:
                    needsRedraw = true;
                    break;
            }
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void WelcomeScreen::reset() {
    needsRedraw = false;
    Motherboard::getBoard().interfaceBlink(25,15);
    welcomeState=WELCOME_START;
    ready_fail = false;
}

ReadyMenu::ReadyMenu() {
	itemCount = 4;
	reset();
}

void ReadyMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ReadyMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar ready1[] = "Are you Ready to";
    static PROGMEM prog_uchar ready2[] = "Print?";
    static PROGMEM prog_uchar no[]   =   "No";
    static PROGMEM prog_uchar yes[]  =   "Yes!";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(ready1);
            break;
        case 1:
            lcd.writeFromPgmspace(ready2);
            break;
        case 2:
            lcd.writeFromPgmspace(yes);
            break;
        case 3:
            lcd.writeFromPgmspace(no);
            break;
	}
}

void ReadyMenu::handleSelect(uint8_t index) {

	switch (index) {
        case 2:
            interface::popScreen();
            break;
        case 3:
            // set this as a flag to the welcome menu that the bot is not ready to print
            ready_fail = true;
            interface::popScreen();
            break;
	}
}

ToolSelectMenu::ToolSelectMenu() {
	itemCount = 4;
	reset();
}

void ToolSelectMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ToolSelectMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar qone[] = "Am I a dual or      ";
    static PROGMEM prog_uchar qtwo[] = "single tool machine?";
    static PROGMEM prog_uchar dual[]   =   "DUAL extruder";
    static PROGMEM prog_uchar single[]  =   "SINGLE extruder";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(qone);
            break;
        case 1:
            lcd.writeFromPgmspace(qtwo);
            break;
        case 2:
            lcd.writeFromPgmspace(single);
            break;
        case 3:
            lcd.writeFromPgmspace(dual);
            break;
	}
}

void ToolSelectMenu::handleSelect(uint8_t index) {
	switch (index) {
        case 2:
            // single extruder
            eeprom::setToolHeadCount(1);
            interface::popScreen();
            break;
        case 3:
            // dual extruder
            eeprom::setToolHeadCount(2);
            interface::popScreen();
            break;
	}
}


bool MessageScreen::screenWaiting(void){
	return timeout.isActive();
}

void MessageScreen::addMessage(CircularBuffer& buf, bool msgComplete) {
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
		//endCursor--;
	}
	if(msgComplete)
		needsRedraw = true;
}


void MessageScreen::addMessage(char msg[],  bool msgComplete) {

	char* letter = msg;
	while (*letter != 0) {
		message[cursor++] = *letter;
		letter++;
	}
//	for(int i = 0; i < length; i++)
//		message[cursor++] = msg[i];
		
	// ensure that message is always null-terminated
	if (cursor == BUF_SIZE) {
		message[BUF_SIZE-1] = '\0';
	} else {
		message[cursor] = '\0';
		// decrement cursor to prepare for subsequent
		// extensions to the message
		//endCursor--;
	}
	if(msgComplete)
		needsRedraw = true;
}

void MessageScreen::clearMessage() {
	x = y = 0;
	message[0] = '\0';
	cursor = 0;
	needsRedraw = true;
	lcdClear = true;
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
		if(lcdClear)
		{
			lcd.clear();
			lcdClear = false;
		}
		while (*b != '\0') {
			lcd.setCursor(x, ycursor);
			b = lcd.writeLine(b);
			if (*b == '\n') {
				b++;
				ycursor++;
			}
		}
	}
}

void MessageScreen::reset() {
	//clearMessage();
}

void MessageScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	// TODO: Integrate with button wait here
	switch (button) {
		case ButtonArray::CENTER:
			timeout= Timeout();
            interface::popScreen();
			break;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
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
	
	static PROGMEM prog_uchar jog2z[] = "        Z-          ";
	static PROGMEM prog_uchar jog3z[] = "  <-Y (Back)        ";
	static PROGMEM prog_uchar jog4z[] = "        Z+          ";

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
	steppers::abort();

	int32_t interval = 1000;
	int32_t steps;

	switch(jogDistance) {
	case DISTANCE_SHORT:
		steps = 20;
		break;
	case DISTANCE_LONG:
		steps = 1000;
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
			position[2] += steps;
			break;
			case ButtonArray::UP:
			position[2] -= steps;
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
	buildPercentage = 101;
	
}
void MonitorMode::setBuildPercentage(uint8_t percent){

	buildPercentage = percent;
}

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar build_percent[] =    " --%";
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
		case host::HOST_STATE_BUILDING_ONBOARD:
			lcd.writeString(host::getBuildName());
			
			lcd.setCursor(16,0);
			lcd.writeFromPgmspace(build_percent);
			
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
	case 6:
		if(host::getHostState() == host::HOST_STATE_BUILDING || host::HOST_STATE_BUILDING_FROM_SD 
				|| host::HOST_STATE_BUILDING_ONBOARD )
		{
			if(buildPercentage < 100)
			{
				lcd.setCursor(17,0);
				lcd.writeInt(buildPercentage,2);
			}
			else if(buildPercentage == 100)
			{
				lcd.setCursor(16,0);
				lcd.writeString("Done");
			}
		}
		else{
			lcd.setCursor(17,0);
			lcd.writeString("   ");
		}
		break;
	}

	updatePhase++;
	if (updatePhase > 6) {
		updatePhase = 0;
	}
}

void MonitorMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::CENTER:
        case ButtonArray::LEFT:
			switch(host::getHostState()) {
			case host::HOST_STATE_BUILDING:
			case host::HOST_STATE_BUILDING_FROM_SD:
			case host::HOST_STATE_BUILDING_ONBOARD:
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
			|| forceRedraw  || lineUpdate) {
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
	lineUpdate = false;
}

void Menu::reset() {
	firstItemIndex = 0;
	itemIndex = 0;
	lastDrawIndex = 255;
	lineUpdate = false;
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
        case ButtonArray::LEFT:
			interface::popScreen();
			break;
        case ButtonArray::RIGHT:
		break;
        case ButtonArray::UP:
        // increment index
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

void CounterMenu::reset(){
    selectMode = false;
    selectIndex = -1;
    Menu::reset();
}
void CounterMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
    switch (button) {
        case ButtonArray::CENTER:
			if(!selectMode){
				selectMode = true;
				selectIndex = itemIndex;
				lineUpdate = true;
			}
            break;
        case ButtonArray::LEFT:
			if(selectMode){
				selectMode = false;
				selectIndex = -1;
                handleSelect(itemIndex);
                lineUpdate = true;
			}
			else
				interface::popScreen();
			break;
        case ButtonArray::RIGHT:
            break;
        case ButtonArray::UP:
            if(selectMode){
                handleCounterUpdate(itemIndex, true);
                lineUpdate = true;
            }
            // increment index
            else{
                if (itemIndex > firstItemIndex) {
                    itemIndex--;
                }}
            break;
        case ButtonArray::DOWN:
            if(selectMode){
                handleCounterUpdate(itemIndex, false);
                lineUpdate = true;
            }
            // decrement index
            else{    
                if (itemIndex < itemCount - 1) {
                    itemIndex++;
                }}
            break;
	}
}

PreheatSettingsMenu::PreheatSettingsMenu() {
	itemCount = 4;
	reset();
}   
void PreheatSettingsMenu::resetState(){
    itemIndex = 1;
	firstItemIndex = 1;
    
    counterRight = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET, 225);
    counterLeft = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET, 225);
    counterPlatform = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET, 110);
}

void PreheatSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar set1[] = "Preheat Settings    ";
    static PROGMEM prog_uchar right[] = "Right Tool          ";
    static PROGMEM prog_uchar left[]   = "Left Tool           ";
    static PROGMEM prog_uchar platform[]  = "Platform        ";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(set1);
            break;
        case 1:
            lcd.writeFromPgmspace(right);
            if(selectIndex == 1){
                lcd.setCursor(14,1);
                lcd.writeString("-->");
            }
            lcd.setCursor(17,1);
            lcd.writeInt(counterRight,3);
            break;
        case 2:
            lcd.writeFromPgmspace(left);
            if(selectIndex == 2){
                lcd.setCursor(14,2);
                lcd.writeString("-->");
            }
            lcd.setCursor(17,2);
            lcd.writeInt(counterLeft,3);
            break;
         case 3:
            lcd.writeFromPgmspace(platform);
            if(selectIndex == 3){
                lcd.setCursor(14,3);
                lcd.writeString("-->");
            }
            lcd.setCursor(17,3);
            lcd.writeInt(counterPlatform,3);
            break;
            
	}
}
void PreheatSettingsMenu::handleCounterUpdate(uint8_t index, bool up){
    switch (index) {
        case 1:
            // update right counter
            if(up)
                counterRight++;
            else
                counterRight--;
            if(counterRight > 260)
                counterRight = 260;
            break;
        case 2:
            // update left counter
            if(up)
                counterLeft++;
            else
                counterLeft--;
            if(counterLeft > 260)
                counterLeft = 260;
            break;
        case 3:
            // update platform counter
            if(up)
                counterPlatform++;
            else
                counterPlatform--;
            if(counterPlatform > 120)
                counterPlatform = 120;
            break;
	}
    
}

void PreheatSettingsMenu::handleSelect(uint8_t index) {
	switch (index) {
        case 1:
            // store right tool setting
            eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), counterRight);
            break;
        case 2:
            // store left tool setting
            eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET), counterLeft);
            break;
        case 3:
            // store platform setting
            eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET), counterPlatform);
            break;
	}
}


ResetSettingsMenu::ResetSettingsMenu() {
	itemCount = 4;
	reset();
}

void ResetSettingsMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ResetSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar set1[] = "Reset Settings to ";
    static PROGMEM prog_uchar set2[] = "Default values?";
    static PROGMEM prog_uchar no[]   =   "No";
    static PROGMEM prog_uchar yes[]  =   "Yes";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(set1);
            break;
        case 1:
            lcd.writeFromPgmspace(set2);
            break;
        case 2:
            lcd.writeFromPgmspace(no);
            break;
        case 3:
            lcd.writeFromPgmspace(yes);
            break;
	}
}

void ResetSettingsMenu::handleSelect(uint8_t index) {
	switch (index) {
        case 2:
            // Don't reset settings, just close dialog.
            interface::popScreen();
            break;
        case 3:
            // Reset setings to defaults
            eeprom::setDefaultSettings();
            interface::popScreen();
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
        static PROGMEM prog_uchar no[]   =   "No";
        static PROGMEM prog_uchar yes[]  =   "Yes";

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(cancel);
		break;
	case 1:
		break;
	case 2:
                lcd.writeFromPgmspace(no);
		break;
	case 3:
                lcd.writeFromPgmspace(yes);
		break;
	}
}

void CancelBuildMenu::handleSelect(uint8_t index) {
	switch (index) {
        case 2:
                // Don't cancel, just close dialog.
                interface::popScreen();
                break;
        case 3:
		// Cancel build, returning to whatever menu came before monitor mode.
		// TODO: Cancel build.
		interface::popScreen();
		host::stopBuild();
		break;
	}
}


MainMenu::MainMenu() {
	itemCount = 9;
	reset();
}
void MainMenu::resetState() {
	itemIndex = 1;
	firstItemIndex = 1;
}

void MainMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar build[] =   "Print from SD";
	static PROGMEM prog_uchar preheat[] = "Preheat";
	static PROGMEM prog_uchar utilities[] = "Utilities";
	static PROGMEM prog_uchar snake[] =   "Snake Game";

	char * name;
	
	switch (index) {
	case 0:
		name = host::getMachineName();
		lcd.setCursor((20 - strlen(name))/2,0);
		lcd.writeString(host::getMachineName());
		break;
	case 1:
		lcd.writeFromPgmspace(build);
		break;
	case 2:
		lcd.writeFromPgmspace(preheat);
		break;
	case 3:
		lcd.writeFromPgmspace(utilities);
		break;
	case 4:
	case 5:
	case 6:
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
		case 1:
			// Show build from SD screen
                        interface::pushScreen(&sdMenu);
			break;
		case 2:
			// Show preheat screen
                        interface::pushScreen(&preheat);
			break;
		case 3:
			// home axes script
                        interface::pushScreen(&utils);
			break;
		case 8:
			// Snake GAME!
                        interface::pushScreen(&snake);
			break;
		}
}

uint8_t LEDrates[] = {0, 255, 200, 100, 50}; 

UtilitiesMenu::UtilitiesMenu() {
	itemCount = 10;
	stepperEnable = false;
	LEDrate = LED_BLINK_OFF;
	reset();
}

void UtilitiesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar monitor[] = "Monitor Mode";
	static PROGMEM prog_uchar jog[]   =   "Jog Mode";
	static PROGMEM prog_uchar calibration[] = "Calibrate Axes";
	static PROGMEM prog_uchar home_axes[] = "Home Axes";
	static PROGMEM prog_uchar load_filamentR[] = "Load Filament Right";
	static PROGMEM prog_uchar load_filamentL[] = "Load Filament Left";
	static PROGMEM prog_uchar startup[] = "Run Startup Script";
	static PROGMEM prog_uchar heater_test[] = "Heater Test";
	static PROGMEM prog_uchar Dsteps[] = "Disable Steppers";
	static PROGMEM prog_uchar Esteps[] = "Enable Steppers";
	static PROGMEM prog_uchar plate_level[] = "Level Build Plate";
	static PROGMEM prog_uchar led[] = "Blink LEDs";
    static PROGMEM prog_uchar settings[] = "Settings";
	

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(monitor);
		break;
	case 1:
		lcd.writeFromPgmspace(jog);
		break;
	case 3:
		lcd.writeFromPgmspace(load_filamentR);
		break;
	case 4:
		lcd.writeFromPgmspace(load_filamentL);
		break;
	case 2:
		if(stepperEnable)
			lcd.writeFromPgmspace(Esteps);
		else
			lcd.writeFromPgmspace(Dsteps);
		break;
	case 5:
		lcd.writeFromPgmspace(home_axes);
		break;
	case 6:
		lcd.writeFromPgmspace(plate_level);
		break;
	case 7:
		lcd.writeFromPgmspace(led);
		lcd.setCursor(14,3);
            switch(LEDrate){
                case LED_BLINK_OFF:
                    lcd.writeString("OFF");
                    break;
                case LED_BLINK_SLOW:
                    lcd.writeString("SLOW");
                    break;
                case LED_BLINK_MID:
                    lcd.writeString("MID");
                    break;
                case LED_BLINK_FAST:
                    lcd.writeString("FAST");
                    break;
                case LED_BLINK_FASTER:
                    lcd.writeString("FASTER");
                    break;
            }
		break;
	case 8:
		lcd.writeFromPgmspace(startup);
		break;
	case 9:
		lcd.writeFromPgmspace(settings);
		break;
	}
}

void UtilitiesMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// Show monitor build screen
                        interface::pushScreen(&monitorMode);
			break;
		case 1:
			// Show build from SD screen
                       interface::pushScreen(&jogger);
			break;
		case 3:
			// load filament script
                        host::startOnboardBuild(utility::FILAMENT_RIGHT);
			break;
		case 4:
			// load filament script
                      host::startOnboardBuild(utility::FILAMENT_LEFT);
			break;
		case 2:
			for (int i = 0; i < STEPPER_COUNT; i++) 
					steppers::enableAxis(i, stepperEnable);
			lineUpdate = true;
			stepperEnable = !stepperEnable;
			break;
		case 5:
			// home axes script
                    host::startOnboardBuild(utility::HOME_AXES);
			break;
		case 6:
			// level_plate script
                    host::startOnboardBuild(utility::LEVEL_PLATE);
			break;
		case 7:
			LEDrate++;
			if(LEDrate > 4)
				LEDrate = 0;
			RGB_LED::setLEDBlink(LEDrates[LEDrate]);
			lineUpdate = true;		 
			 break;
		case 8:
			// startup wizard script
                        interface::pushScreen(&welcome);
			break;
		case 9:
			// run heater test
                        interface::pushScreen(&set);
			break;

		}
}

SettingsMenu::SettingsMenu() {
	itemCount = 5;
    reset();
}

void SettingsMenu::resetState(){
	singleExtruder = (eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1);
    soundOn = (eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS, 1) != 0);
    LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar preheat_settings[] = "Preheat Settings";
	static PROGMEM prog_uchar tool_single[]   =   "Tool Count  SINGLE ";
    static PROGMEM prog_uchar tool_dual[]   =     "Tool Count  DUAL ";
	static PROGMEM prog_uchar sound_on[] =        "Sound Effects ON";
	static PROGMEM prog_uchar sound_off[] =       "Sound Effects OFF";
	static PROGMEM prog_uchar LED[] =             "LED Color     ";
	static PROGMEM prog_uchar reset[] = "Restore Defaults";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(preheat_settings);
            break;
        case 1:
            if(singleExtruder)
                lcd.writeFromPgmspace(tool_single);
            else
                lcd.writeFromPgmspace(tool_dual);
            break;
        case 2:
            if(soundOn)
                lcd.writeFromPgmspace(sound_on);
            else
                lcd.writeFromPgmspace(sound_off);
            break;
        case 3:
            lcd.writeFromPgmspace(LED);
            lcd.setCursor(12,3);
            switch(LEDColor){
                case LED_DEFAULT_RED:
                    lcd.writeString("RED");
                    break;
                case LED_DEFAULT_ORANGE:
                    lcd.writeString("ORANGE");
                    break;
                case LED_DEFAULT_PINK:
                    lcd.writeString("PINK");
                    break;
                case LED_DEFAULT_GREEN:
                    lcd.writeString("GREEN");
                    break;
                case LED_DEFAULT_BLUE:
                    lcd.writeString("BLUE");
                    break;
                case LED_DEFAULT_PURPLE:
                    lcd.writeString("PURPLE");
                    break;
                case LED_DEFAULT_WHITE:
                    lcd.writeString("WHITE");
                    break;
                case LED_DEFAULT_CUSTOM:
					lcd.writeString("CUSTOM");
					break;
            }
            break;
        case 4:
            lcd.writeFromPgmspace(reset);
            break;
 	}
}

void SettingsMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// preheat settings
            interface::pushScreen(&preheat);
			break;
		case 1:
			// update tool count
            singleExtruder = !singleExtruder;
            eeprom::setToolHeadCount(singleExtruder*1);
            lineUpdate = 1;
			break;
		case 2:
			// update sound preferences
            soundOn = !soundOn;
            eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS, soundOn * 1);
            lineUpdate = 1;
			break;
		case 3:
			// update LED preferences
            LEDColor++;
            if(LEDColor > 6)
                LEDColor = 0;
            eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
            RGB_LED::setDefaultColor();
            lineUpdate = 1;
			break;
		case 4:
			// restore defaults
            interface::pushScreen(&reset_settings);
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
	buildFailed = startBuild();
}

void SDSpecialBuild::update(LiquidCrystalSerial& lcd, bool forceRedraw){
	static PROGMEM prog_uchar SDone[] =    "An SD card with     ";
	static PROGMEM prog_uchar SDtwo[] =    "MakerBot scripts is ";
	static PROGMEM prog_uchar SDthree[] =   "required.  See      ";
	static PROGMEM prog_uchar SDfour[]  =   "makerbot.com/repSD  ";

	if(forceRedraw)
	{
		if(buildFailed)
		{
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
		else
			interface::popScreen();
	}
}

bool SDSpecialBuild::startBuild(){

	if (host::getHostState() != host::HOST_STATE_READY) {
		return true;
	}
		
	char* buildName = host::getBuildName();

    strcpy(buildName, buildType);

    sdcard::SdErrorCode e;
	e = host::startBuildFromSD();
	
	if (e != sdcard::SD_SUCCESS) {
		return true;
	}
	
	return false;
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
	uint8_t idx = 0;
	sdcard::SdErrorCode e;	

	// First, reset the directory index
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		cardNotFound = true;
		return 0;
	}

	///TODO:: error handling for s3g: if the filename is longer than 64, 
	/// does it truncate and keep the extension? or is the extension lost?  
	int maxFileLength = 64; /// SD card max lenghth
	char fnbuf[maxFileLength];

	// Count the files
	do {
		e = sdcard::directoryNextEntry(fnbuf,maxFileLength, &idx);
		if (fnbuf[0] == '\0') {
			break;
		}

		// If it's a dot file, don't count it.
		if (fnbuf[0] == '.') {
		}
		
		else {
			if ((fnbuf[idx-3] == 's') && (fnbuf[idx-2] == '3') && (fnbuf[idx-1] == 'g'))
				count++;
		}

	} while (e == sdcard::SD_SUCCESS);

	// TODO: Check for error again?

	return count;
}

bool SDMenu::getFilename(uint8_t index, char buffer[], uint8_t buffer_size) {
	sdcard::SdErrorCode e;
	uint8_t idx;

	// First, reset the directory list
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
                return false;
	}
	
	int maxFileLength = 64; /// SD card max lenghth
	char fnbuf[maxFileLength];

	for(uint8_t i = 0; i < index+1; i++) {
		// Ignore dot-files
		do {
			e = sdcard::directoryNextEntry(fnbuf,maxFileLength, &idx);
			if (fnbuf[0] == '\0') {
                      return false;
			}
			
		} while ((e == sdcard::SD_SUCCESS) && ((fnbuf[0] == '.')) || 
			!((fnbuf[idx-3] == 's') && (fnbuf[idx-2] == '3') && (fnbuf[idx-1] == 'g')));
			
		if (e != sdcard::SD_SUCCESS) {
                        return false;
		}
	}
	uint8_t bufSize = (buffer_size <= idx) ? buffer_size-1 : idx; 	
	for(uint8_t i = 0; i < bufSize; i++)
		buffer[i] = fnbuf[i];
	buffer[bufSize] = 0;
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

	uint8_t maxFileLength = LCD_SCREEN_WIDTH-1;
	char fnbuf[maxFileLength];

        if ( !getFilename(index, fnbuf, maxFileLength)) {
                Motherboard::getBoard().errorResponse("SD card read error");
		return;
	}

	uint8_t idx;
	for (idx = 0; (idx < maxFileLength) && (fnbuf[idx] != 0); idx++) {
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
		Motherboard::getBoard().errorResponse("I'm already building");
		return;
	}
		
	char* buildName = host::getBuildName();

    if ( !getFilename(index, buildName, host::MAX_FILE_LEN) ) {
		Motherboard::getBoard().errorResponse("SD card read error");
		return;
	}

    sdcard::SdErrorCode e;
	e = host::startBuildFromSD();
	
	if (e != sdcard::SD_SUCCESS) {
		Motherboard::getBoard().errorResponse("SD card read error");
		return;
	}
}

#endif
