#include "Menu.hh"
#include "Menu.hh"
#include "Configuration.hh"

// TODO: Kill this, should be hanlded by build system.
#ifdef HAS_INTERFACE_BOARD

#include "Steppers.hh"
#include "Commands.hh"
#include "Errors.hh"
#include "Host.hh"
#include "Timeout.hh"
#include "InterfaceBoard.hh"
#include "Interface.hh"
#include <util/delay.h>
#include <stdlib.h>
#include "SDCard.hh"
#include <string.h>
#include "Version.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>
#include "RGB_LED.hh"
#include "Planner.hh"
#include "stdio.h"


//#define HOST_PACKET_TIMEOUT_MS 20
//#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

//#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
//#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

#define FILAMENT_HEAT_TEMP 220

bool ready_fail = false;

enum sucessState{
	SUCCESS,
	FAIL,
	SECOND_FAIL
};

uint8_t levelSuccess;
uint8_t filamentSuccess;

void SplashScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar splash3[] = "                    ";
	static PROGMEM prog_uchar splash1[] = "  The Replicator    ";
	static PROGMEM prog_uchar splash2[] = "    ----------      ";
	static PROGMEM prog_uchar splash4[] = "Firmware Version b. ";


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
/*
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
*/

HeaterPreheat::HeaterPreheat(){
	itemCount = 4;
	reset();
}

void HeaterPreheat::resetState(){
    uint8_t heatSet = eeprom::getEeprom8(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET, 0);
	_rightActive = (heatSet & (1 << HEAT_MASK_RIGHT)) != 0;
    _platformActive = (heatSet & (1 << HEAT_MASK_PLATFORM)) != 0;
	_leftActive = (heatSet & (1 << HEAT_MASK_LEFT)) != 0;
	singleTool = eeprom::isSingleTool();
    Motherboard &board = Motherboard::getBoard();
    if(((board.getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0) || !_rightActive) &&
        ((board.getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0) || !_leftActive) &&
        ((board.getPlatformHeater().get_set_temperature() >0) || !_platformActive))
       preheatActive = true;
    else
       preheatActive = false;
}

void HeaterPreheat::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar go[] = "Start Preheat!";
    static PROGMEM prog_uchar stop[] = "Cool!";
	static PROGMEM prog_uchar right[] = "Right Tool";
	static PROGMEM prog_uchar left[] = "Left Tool";
	static PROGMEM prog_uchar platform[] = "Platform";
    static PROGMEM prog_uchar tool[] = "Extruder";
	
	switch (index) {
	case 0:
        if(preheatActive)
            lcd.writeFromPgmspace(stop);
        else
            lcd.writeFromPgmspace(go);
		break;
	case 1:
		if(!singleTool){
		lcd.writeFromPgmspace(right);
        lcd.setCursor(16,1);
		if(_rightActive)
			lcd.writeString("ON ");
		else
			lcd.writeString("OFF");
		}
		break;
	case 2:
        
		if(singleTool){
			lcd.writeFromPgmspace(tool);
			lcd.setCursor(16,2);
			if(_rightActive)
				lcd.writeString("ON ");
			else
				lcd.writeString("OFF");
		}
		else{
			lcd.writeFromPgmspace(left);
			lcd.setCursor(16,2);
			if(_leftActive)
				lcd.writeString("ON ");
			else
				lcd.writeString("OFF");
		}
		break;
	case 3:
		lcd.writeFromPgmspace(platform);
        lcd.setCursor(16,3);
		if(_platformActive)
			lcd.writeString("ON ");
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
            preheatActive = !preheatActive;
            if(preheatActive){
                Motherboard::getBoard().resetUserInputTimeout();
                temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET,0) *_rightActive; 
                Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
                if(!singleTool){
                    temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET,0) *_leftActive;
                    Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
                }
                temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET,0) *_platformActive;
                Motherboard::getBoard().getPlatformHeater().set_target_temperature(temp);
            }
            else{
                Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
                Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
                Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
            }
            interface::popScreen();
            interface::pushScreen(&monitorMode);
            //needsRedraw = true;
			break;
		case 1:
			if(!singleTool){
				_rightActive  = !_rightActive;
				storeHeatByte();
				if(preheatActive){
				  needsRedraw = true;
				}else{
				  lineUpdate = true;
				}
                preheatActive = false;
			}
			break;
		case 2:
			if(singleTool)
				_rightActive  = !_rightActive;
			else
				_leftActive  = !_leftActive;
            storeHeatByte();
            if(preheatActive){
              needsRedraw = true;
            }else{
			  lineUpdate = true;
		    }
            preheatActive = false; 
			break;
		case 3:
			_platformActive = !_platformActive;
            storeHeatByte();
            if(preheatActive){
				needsRedraw = true;
			}else{
			  lineUpdate = true;
		    }
            preheatActive = false;
			break;
		}
}

void WelcomeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar start[] =    "Welcome!            I'm the Replicator. Press the red M to  get started!        ";
	static PROGMEM prog_uchar buttons1[] = "A blinking 'M' meansI'm waiting and willcontinue when you   press the button... ";
    static PROGMEM prog_uchar buttons2[] = "A solid 'M' means   I'm working and willupdate my status    when I'm finished...";
	static PROGMEM prog_uchar explain[] =  "Our next steps will get me set up to    print! First, we'll restore my build... ";                                  
    static PROGMEM prog_uchar level[] =    "platform so it's    nice and level. It'sprobably a bit off  from shipping...    ";
    static PROGMEM prog_uchar better[] =   "Aaah, that feels    much better.        Let's go on and loadsome plastic!       ";
    static PROGMEM prog_uchar tryagain[] = "We'll try again!                                                                ";
    static PROGMEM prog_uchar go_on[]   =  "We'll keep going    and load some       plastic! For help goto makerbot.com/help";     
    
    static PROGMEM prog_uchar sd_menu[] =  "Awesome!            We'll go to the SD  card Menu and you   can select a print! ";
    static PROGMEM prog_uchar fail[] =     "We'll go to the mainmenu. If you need   help go to:         makerbot.com/help   ";
    
    
	if (forceRedraw || needsRedraw) {
	//	waiting = true;
		lcd.setCursor(0,0);
        switch (welcomeState){
            case WELCOME_START:
                lcd.writeFromPgmspace(start);
                _delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
            case WELCOME_BUTTONS1:
				lcd.writeFromPgmspace(buttons1);
				_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                 break;
            case WELCOME_BUTTONS2:
				lcd.writeFromPgmspace(buttons2);
				_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
			case WELCOME_EXPLAIN:
                lcd.writeFromPgmspace(explain);
                _delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                break;
            case WELCOME_LEVEL:
                lcd.writeFromPgmspace(level);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                eeprom_write_byte((uint8_t*)eeprom_offsets::FIRST_BOOT_FLAG, 1);

                break;
            case WELCOME_LEVEL_OK:
			//	waiting = false;
				interface::pushScreen(&levelOK);
				_delay_us(1000000);
				welcomeState++;
				break;
            case WELCOME_LOAD_PLASTIC:
				if(levelSuccess == SUCCESS){
					lcd.writeFromPgmspace(better);
				} else if(levelSuccess == FAIL){
					lcd.writeFromPgmspace(tryagain);
					welcomeState = WELCOME_LEVEL;
				} else if(levelSuccess == SECOND_FAIL){
					lcd.writeFromPgmspace(go_on);
				}
				_delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);            
                break;
            case WELCOME_READY:
                interface::pushScreen(&ready);
                welcomeState++;
                break;
            case WELCOME_LOAD_SD:
                if(ready_fail){
                    lcd.writeFromPgmspace(fail);
                    welcomeState++;
                }
                else
                 lcd.writeFromPgmspace(sd_menu);
                 _delay_us(1000000);
                 Motherboard::getBoard().interfaceBlink(25,15);
                
                break;
            case WELCOME_DONE:
				host::stopBuild();
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
              //  case WELCOME_TOOL_SELECT:
              //      Motherboard::getBoard().interfaceBlink(0,0);
               //     welcomeState++;
               //     interface::pushScreen(&tool_select);
               //     break;
                case WELCOME_LEVEL_ACTION:
					Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++; 
					if(levelSuccess == FAIL)
						host::startOnboardBuild(utility::LEVEL_PLATE_SECOND);
					else
						host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
                    break;
                case WELCOME_LOAD_ACTION:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
                        filament.setScript(FILAMENT_STARTUP_SINGLE);
					else
                        filament.setScript(FILAMENT_STARTUP_DUAL);
					interface::pushScreen(&filament);
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
			welcomeState--;
			if(welcomeState < WELCOME_START){
				welcomeState = WELCOME_START;
			//	Motherboard::getBoard().interfaceBlink(0,0);
			//	interface::popScreen();
			}
			switch (welcomeState){
                case WELCOME_LEVEL_ACTION:
					Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++; 
                    if(levelSuccess == FAIL)
						host::startOnboardBuild(utility::LEVEL_PLATE_SECOND);
					else
						host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
                    break;
                case WELCOME_LOAD_ACTION:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    
                    if(eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
                        filament.setScript(FILAMENT_STARTUP_SINGLE);
					else
                        filament.setScript(FILAMENT_STARTUP_DUAL);
					interface::pushScreen(&filament);
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
    levelSuccess = SUCCESS;
    level_offset = 0;
}

void NozzleCalibrationScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar start[] =    "I'm going to print  a series of lines sowe can find my      nozzle alignment.   ";
	static PROGMEM prog_uchar explain1[] = "Look for the best   matched line in eachaxis set. Lines are numbered 1-13 and...";
	static PROGMEM prog_uchar explain2[] = "line one is extra   long. The Y axis setis left on the plateand X axis is right.";
	static PROGMEM prog_uchar end  [] =    "Great!  I've saved  these settings and  I'll use them to    make nice prints!   ";
    
	if (forceRedraw || needsRedraw) {
		lcd.setCursor(0,0);
        switch (alignmentState){
            case ALIGNMENT_START:
                lcd.writeFromPgmspace(start);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN1:
				lcd.writeFromPgmspace(explain1);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN2:
				lcd.writeFromPgmspace(explain2);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_SELECT:
				Motherboard::getBoard().interfaceBlink(0,0);
				interface::pushScreen(&align);
				 alignmentState++;
                 break;
            case ALIGNMENT_END:
				lcd.writeFromPgmspace(end);
				_delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);
                 break;  
        }
        needsRedraw = false;
	}
}

void NozzleCalibrationScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	Point position;
	
	switch (button) {
		case ButtonArray::CENTER:
           alignmentState++;
           
            switch (alignmentState){
                case ALIGNMENT_PRINT:
					Motherboard::getBoard().interfaceBlink(0,0); 
					host::startOnboardBuild(utility::TOOLHEAD_CALIBRATE);
					alignmentState++;
                    break;
                case ALIGNMENT_QUIT:
					Motherboard::getBoard().interfaceBlink(0,0); 
                     interface::popScreen();
                    break;
                default:
                    needsRedraw = true;
                    break;
            }
			break;
        case ButtonArray::LEFT:
			interface::pushScreen(&cancelBuildMenu);
			break;
			
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;

	}
}

void NozzleCalibrationScreen::reset() {
    needsRedraw = false;
    Motherboard::getBoard().interfaceBlink(25,15);
    alignmentState=ALIGNMENT_START;
}

SelectAlignmentMenu::SelectAlignmentMenu() {
	itemCount = 4;
    reset();
}

void SelectAlignmentMenu::resetState(){
	itemIndex = 1;
	firstItemIndex = 1;
	lastSelectIndex = 2;
	xCounter = 7;
	yCounter = 7;
}

void SelectAlignmentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	
	static PROGMEM prog_uchar select[] = "Select best lines.";
	static PROGMEM prog_uchar done[]   = "Done!";
	static PROGMEM prog_uchar xaxis[] = "X Axis Line";
	static PROGMEM prog_uchar yaxis[] = "Y Axis Line";
    
    int32_t offset;
	switch (index) {
		case 0:
			lcd.writeFromPgmspace(select);
			break;
        case 1:
			lcd.writeFromPgmspace(xaxis);
			lcd.setCursor(13,1);
			if(selectIndex == 1)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(17,1);
            lcd.writeInt(xCounter,2);
            break;
         case 2:
			lcd.writeFromPgmspace(yaxis);
			 lcd.setCursor(13,2);
			if(selectIndex == 2)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(17,2);
            lcd.writeInt(yCounter,2);
            break;
         case 3:
			lcd.writeFromPgmspace(done);
			break;
 	}
}

void SelectAlignmentMenu::handleCounterUpdate(uint8_t index, bool up){
   
    switch (index) {
        case 1:
            // update platform counter
            // update right counter
            if(up)
                xCounter++;
            else
                xCounter--;
            // keep within appropriate boundaries    
            if(xCounter > 13)
                xCounter = 13;
            else if(xCounter < 1)
				xCounter = 1;			
            break;
        case 2:
            // update right counter
           if(up)
                yCounter++;
            else
                yCounter--;
           // keep within appropriate boundaries    
            if(yCounter > 13)
                yCounter = 13;
            else if(yCounter < 1)
				yCounter = 1;	
            break;
	}  
}


void SelectAlignmentMenu::handleSelect(uint8_t index) {
	
	int32_t offset;
	switch (index) {
		case 1:
			// update toolhead offset (tool tolerance setting) 
			// this is summed with previous offset setting
			offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0)) + (int32_t)((xCounter-7)*XSTEPS_PER_MM *0.1f * 10);
            eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 4);
            lineUpdate = 1;
			break;
		case 2:
			// update toolhead offset (tool tolerance setting)
			offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 0)) + (int32_t)((yCounter-7)*YSTEPS_PER_MM *0.1f * 10);
			eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 4);
			lineUpdate = 1;
			break;
		case 3:
			interface::popScreen();
			break;
    }
}

void FilamentScreen::startMotor(){
    int32_t interval = 300000000;  // 5 minutes
    int32_t steps = interval / 6250;
    if(forward)
        steps *= -1;
    Point target = Point(0,0,0,0,0);
    target[axisID] = steps;
    
    planner::abort();
    planner::setAccelerationOn(false);
    planner::addMoveToBufferRelative(target, interval, 0x1f);
    filamentTimer.clear();
    filamentTimer.start(300000000); //5 minutes
}
void FilamentScreen::stopMotor(){
    
    planner::abort();
    for(int i = 0; i < STEPPER_COUNT; i++)
        steppers::enableAxis(i, false);

}

void FilamentScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar heater_error[]= "My extruders are    not heating up.     Check my            connections!        ";
	static PROGMEM prog_uchar explain_one[] = "Press down on the   grey rings at top ofthe extruders and   pull the black...   ";
	static PROGMEM prog_uchar explain_two[] = "guide tubes out. Nowfeed filament from  the back through thetubes until it...   ";
	static PROGMEM prog_uchar explain_oneS[]= "Press down on the   grey ring at top of the extruder and    pull the black...   ";
	static PROGMEM prog_uchar explain_twoS[]= "guide tube out.  Nowfeed filament from  the back through thetube until it...   ";
	static PROGMEM prog_uchar explain_thre[]= "pops out in front.  I'm heating up my   extruder so we can  load the filament...";
	static PROGMEM prog_uchar explain_four[]= "This might take a   few minutes.        And watch out, the  nozzle will get HOT!";
	static PROGMEM prog_uchar heating_bar[] = "Heating Progress:                                                               ";
	static PROGMEM prog_uchar heating[] 	= "I'm heating up my   extruder!           Please wait.                            ";
	static PROGMEM prog_uchar ready_right[] = "OK I'm ready!       First we'll load theright extruder.     Push filament in... ";
	static PROGMEM prog_uchar ready_single[]= "OK I'm ready!       Pop the guide tube  off and push the    filament down...    "; 
	static PROGMEM prog_uchar ready_rev[]   = "OK I'm ready!       Pop the guide tube  off and pull        filament gently...  ";
    static PROGMEM prog_uchar ready_left[]  = "Great! Now we'll    load the left       extruder. Push      filament down...    ";
    static PROGMEM prog_uchar tug[]         = "through the grey    ring until you feel the motor tugging   the plastic in...   ";
    static PROGMEM prog_uchar stop[]        = "When filament is    extruding out of thenozzle, Press 'M'   to stop extruding.  "; 
    static PROGMEM prog_uchar stop_exit[]   = "When filament is    extruding out of thenozzle, Press 'M'   to exit             "; 
    static PROGMEM prog_uchar stop_reverse[]= "When my filament is released,           Press 'M' to exit.                      ";            
    static PROGMEM prog_uchar tryagain[]    = "OK! I'll keep my    motor running. You  may need to push    harder...           ";                                                      
    static PROGMEM prog_uchar go_on[]       = "We'll keep going.   If you're having    trouble, check out  makerbot.com/help   ";  
    static PROGMEM prog_uchar finish[]      = "Great!  I'll stop   running my extruder.Press M to continue.                    ";  
    static PROGMEM prog_uchar go_on_left[]  = "We'll keep going.   Lets try the left   extruder. Push      filament down...    ";
    
    
    Point target = Point(0,0,0, 0,0);
    int32_t interval;
    
	if(filamentState == FILAMENT_WAIT){
		
		if(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().has_reached_target_temperature()){
			
			int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			// check for externally manipulated temperature (eg by RepG)
			if(setTemp < FILAMENT_HEAT_TEMP){
					Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
					interface::popScreen();
					Motherboard::getBoard().errorResponse("My temperature was  changed externally. Reselect filament   menu to try again.");
					return;
			}
			
			filamentState++;
			needsRedraw= true;
			RGB_LED::setDefaultColor();
			LEDClear = true;
			startMotor();
			if(!helpText && !startup)
				filamentState = FILAMENT_STOP;
		}
		else if (filamentTimer.hasElapsed()){
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(heater_error);
            Motherboard::getBoard().interfaceBlink(25,15);
            filamentState = FILAMENT_DONE;
		}
		else{
            int16_t currentTemp = Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().getDelta();
            int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
            // check for externally manipulated temperature (eg by RepG)
			if(setTemp < FILAMENT_HEAT_TEMP){
					Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
					interface::popScreen();
					Motherboard::getBoard().errorResponse("My temperature was  changed externally. Reselect filament   menu to try again.");
					return;
			}
				
			uint8_t heatIndex = (abs((setTemp - currentTemp)) * 20) / setTemp;
			
			int32_t mult = 255;
			if(heatLights){
				RGB_LED::setColor((mult*(setTemp - currentTemp))/setTemp, 0, (mult*currentTemp)/setTemp, LEDClear);
				LEDClear = false;
			}
            
            if (lastHeatIndex > heatIndex){
				lcd.setCursor(0,3);
				lcd.writeString("                    ");
				lastHeatIndex = 0;
			}
            
            lcd.setCursor(lastHeatIndex,3);
            for (int i = lastHeatIndex; i < heatIndex; i++)
                lcd.write(0xFF);
            lastHeatIndex = heatIndex;
            
            toggleCounter++;
            if(toggleCounter > 6){
				toggleBlink = !toggleBlink;
				if(toggleBlink)
					lcd.writeString(" ");
				else
					lcd.write(0xFF);
				toggleCounter = 0;
			}
                          
			}
	}
	if((filamentState == FILAMENT_STOP) && filamentTimer.hasElapsed()){
		if(startup){
			filamentState = FILAMENT_OK;
			interface::pushScreen(&filamentOK);
		}
		else{
			filamentState = FILAMENT_DONE;
			needsRedraw = true;
		}
    }
	
	if (forceRedraw || needsRedraw) {
        //	waiting = true;
		lcd.setCursor(0,0);
		lastHeatIndex = 0;
        switch (filamentState){
			case FILAMENT_HEATING:
				Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().set_target_temperature(FILAMENT_HEAT_TEMP);
				if(dual)
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(FILAMENT_HEAT_TEMP);
				if(startup){
					if(dual)
						lcd.writeFromPgmspace(explain_one);
					else
						lcd.writeFromPgmspace(explain_oneS);
					Motherboard::getBoard().interfaceBlink(25,15);
					_delay_us(1000000);
				}
				else{
					lcd.writeFromPgmspace(heating);
					lastHeatIndex = 0;
					heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
					filamentState = FILAMENT_WAIT;
				}
				filamentTimer.clear();
				filamentTimer.start(300000000); //5 minutes
				
				break;
			case FILAMENT_EXPLAIN2:
				if(dual)
					lcd.writeFromPgmspace(explain_two);
				else
					lcd.writeFromPgmspace(explain_twoS);
				Motherboard::getBoard().interfaceBlink(25,15);
					_delay_us(1000000);
				break;
			case FILAMENT_EXPLAIN3:
				lcd.writeFromPgmspace(explain_thre);
				Motherboard::getBoard().interfaceBlink(25,15);
			    _delay_us(1000000);
				break;
			case FILAMENT_EXPLAIN4:
				lcd.writeFromPgmspace(explain_four);			
				//_delay_us(1000000);
				// if z stage is at zero, move z stage down
				target = planner::getPosition();
				if(target[2] < 1000){
					target[2] = 60000;
					interval = 5000000;
					planner::addMoveToBufferRelative(target, interval, 0x1f);
				}
				_delay_us(1000000);
				Motherboard::getBoard().interfaceBlink(25,15);
				break;
			case FILAMENT_HEAT_BAR:
				lcd.writeFromPgmspace(heating_bar);
				_delay_us(3000000);
				filamentState++;
				heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
				break;
			case FILAMENT_WAIT:
				if(startup)
					lcd.writeFromPgmspace(heating_bar);
				else
				    lcd.writeFromPgmspace(heating);
				break;
            case FILAMENT_START:
                if(dual){
					if(axisID == 3)
						lcd.writeFromPgmspace(ready_right);
					else
						lcd.writeFromPgmspace(ready_left);
				}
				else if(forward)
					lcd.writeFromPgmspace(ready_single);
				else{
					lcd.writeFromPgmspace(ready_rev);
					filamentState++;
				}	
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(100000);
                break;
            case FILAMENT_TUG:
				lcd.writeFromPgmspace(tug);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(100000);
                break;
            case FILAMENT_STOP:
				if(startup)
					lcd.writeFromPgmspace(stop);
				else{
					if(forward)
						lcd.writeFromPgmspace(stop_exit);
					else 
						lcd.writeFromPgmspace(stop_reverse);
				}
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                break;
            case FILAMENT_DONE:
                stopMotor();
				if(filamentSuccess == SUCCESS){
                    if(dual && (axisID ==3)){
                        axisID = 4;
                        lastHeatIndex = 0;
                        filamentState = FILAMENT_START;
                        startMotor();
                        lcd.writeFromPgmspace(ready_left);
                    }
                    else
						lcd.writeFromPgmspace(finish);
				} else{
                  if(filamentSuccess == FAIL){
					lcd.writeFromPgmspace(tryagain);
                    startMotor();
					filamentState = FILAMENT_TUG;
				  } else if(filamentSuccess == SECOND_FAIL){ 
                      if(dual && (axisID ==3)){
                         axisID = 4;
                         filamentState = FILAMENT_TUG;
                         startMotor();
                         lcd.writeFromPgmspace(go_on_left);
						}
						else{
							lcd.writeFromPgmspace(go_on);
						}
                  }
			  }
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                
                break;
        }
        needsRedraw = false;
	} 
}
void FilamentScreen::setScript(FilamentScript script){
    
    filamentState = FILAMENT_HEATING;
    dual = false;
    startup = false;
    helpText = eeprom::getEeprom8(eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
    switch(script){
        case FILAMENT_STARTUP_DUAL:
            dual = true;
        case FILAMENT_STARTUP_SINGLE:
			startup = true;
        case FILAMENT_RIGHT_FOR:
			toolID = 0;
            axisID = 3;
            forward = true;
            break;
        case FILAMENT_LEFT_FOR:
			toolID = 1;
            axisID = 4;
            forward = true;            
            break;
        case FILAMENT_RIGHT_REV:
			toolID = 0;
            axisID = 3;
            forward = false;
            break;
        case FILAMENT_LEFT_REV:
			toolID = 1;
            axisID = 4;
            forward = false;
            break;
    }
    
}

void FilamentScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	Point position;
	
	switch (button) {
		case ButtonArray::CENTER:
			if(filamentState == FILAMENT_WAIT)
				break;
            filamentState++;
            Motherboard::getBoard().interfaceBlink(0,0);
            switch (filamentState){
                case FILAMENT_OK:
					if(startup){
						filamentState++;
						interface::pushScreen(&filamentOK);
					}
					else{
						stopMotor();
						Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
						Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
						interface::popScreen();
					}
                    break;
                case FILAMENT_EXIT:
					stopMotor();
					Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
					interface::popScreen();
                    break;
                default:
                    needsRedraw = true;
                    break;
            }
			break;
        case ButtonArray::LEFT:
			interface::pushScreen(&cancelBuildMenu);			
            break;			
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
            
	}
}

void FilamentScreen::reset() {
    needsRedraw = false;
    heatLights = true;
    LEDClear = true;
    filamentState=FILAMENT_HEATING;
    filamentSuccess = SUCCESS;
    filamentTimer = Timeout();
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
	static PROGMEM prog_uchar ready1[] = "How'd it go? Ready ";
    static PROGMEM prog_uchar ready2[] = "to try a print?    ";
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

LevelOKMenu::LevelOKMenu() {
	itemCount = 4;
	reset();
}

void LevelOKMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void LevelOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar ready1[] = "Does my nozzle";
    static PROGMEM prog_uchar ready2[] = "height check out?";
    static PROGMEM prog_uchar yes[]   =   "My height is good!";
    static PROGMEM prog_uchar no[]  =   "Let's try again.";
    
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

void LevelOKMenu::handleSelect(uint8_t index) {

	switch (index) {
        case 2:
			levelSuccess = SUCCESS;
            interface::popScreen();
            break;
        case 3:
            // set this as a flag to the welcome menu that the bot is not ready to print
            if(levelSuccess == FAIL)
				levelSuccess = SECOND_FAIL;
			else
				levelSuccess = FAIL;
            interface::popScreen();
            break;
	}
}

FilamentOKMenu::FilamentOKMenu() {
	itemCount = 4;
	reset();
}

void FilamentOKMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void FilamentOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar qone[] = "Did plastic extrude ";
    static PROGMEM prog_uchar qtwo[] = "from the nozzle?";
    static PROGMEM prog_uchar no[]   =   "No";
    static PROGMEM prog_uchar yes[]  =   "Yes!";
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(qone);
            break;
        case 1:
            lcd.writeFromPgmspace(qtwo);
            break;
        case 2:
            lcd.writeFromPgmspace(yes);
            break;
        case 3:
            lcd.writeFromPgmspace(no);
            break;
	}
}

void FilamentOKMenu::handleSelect(uint8_t index) {
	switch (index) {
        case 2:
			filamentSuccess = SUCCESS;
            interface::popScreen();
            break;
        case 3:
            // set this as a flag to the welcome menu that the bot is not ready to print
            if(filamentSuccess == FAIL)
				filamentSuccess = SECOND_FAIL;
			else
				filamentSuccess = FAIL;
            interface::popScreen();
	}
}

FilamentMenu::FilamentMenu() {
	itemCount = 4;
	reset();
}

void FilamentMenu::resetState() {
    singleTool = eeprom::isSingleTool();
    if(singleTool)
        itemCount = 2;
	itemIndex = 0;
	firstItemIndex = 0;
}

void FilamentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar load[] = "Load"; 
    static PROGMEM prog_uchar unload[] = "UnLoad"; 
    
    singleTool = eeprom::isSingleTool();
    if(singleTool)
        itemCount = 2;
    else
		itemCount = 4;
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(load);
            if(!singleTool){
                lcd.setCursor(7,0);
                lcd.writeString("Right");
            }
            break;
        case 1:
            lcd.writeFromPgmspace(unload);
            if(!singleTool){
                lcd.setCursor(9,1);
                lcd.writeString("Right");
            }
            break;
        case 2:
            if(!singleTool){
                lcd.writeFromPgmspace(load);
                lcd.setCursor(7,2);
                lcd.writeString("Left");
            }
            break;
        case 3:
            if(!singleTool){
                lcd.writeFromPgmspace(unload);
                lcd.setCursor(9,3);
                lcd.writeString("Left");
            }

            break;
	}
    
}

void FilamentMenu::handleSelect(uint8_t index) {
    
	switch (index) {
        case 0:
          //  interface::pushNoUpdate(&filament);
            filament.setScript(FILAMENT_RIGHT_FOR);
            interface::pushScreen(&filament);
            break;
        case 1:
          //  interface::pushNoUpdate(&filament);
            filament.setScript(FILAMENT_RIGHT_REV);
            interface::pushScreen(&filament);
            break;
        case 2:
          //  interface::pushNoUpdate(&filament);
            filament.setScript(FILAMENT_LEFT_FOR);
            interface::pushScreen(&filament);
            break;
        case 3:
          //  interface::pushNoUpdate(&filament);
            filament.setScript(FILAMENT_LEFT_REV);
            interface::pushScreen(&filament);
            break;
	}
}



bool MessageScreen::screenWaiting(void){
	return (timeout.isActive() || incomplete);
}

void MessageScreen::addMessage(CircularBuffer& buf, bool msgComplete) {
	char c = buf.pop();
	while (c != '\0' && cursor < BUF_SIZE && buf.getLength() > 0) {
		message[cursor++] = c;
		c = buf.pop();
	}
	// ensure that message is always null-terminated
	if (cursor == BUF_SIZE-1) {
		message[BUF_SIZE-1] = '\0';
	} else {
		message[cursor] = '\0';
		// decrement cursor to prepare for subsequent
		// extensions to the message
		//endCursor--;
	}
	if(msgComplete){
		incomplete = false;
		needsRedraw = true;
	}
	else
		incomplete = true;
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
	if(msgComplete){
		incomplete = false;
		needsRedraw = true;
	}
	else
		incomplete = true;

}

void MessageScreen::clearMessage() {
	x = y = 0;
	message[0] = '\0';
	cursor = 0;
	needsRedraw = true;
	lcdClear = true;
	timeout = Timeout();
	incomplete = false;
	popScreenOn = false;
}

void MessageScreen::setTimeout(uint8_t seconds, bool pop) {
	timeout.start((micros_t)seconds * 1000L * 1000L);
	popScreenOn = pop;
}

void MessageScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	char* b = message;
	int ycursor = y;
	if (timeout.hasElapsed() && popScreenOn) {
		popScreenOn = false;
		interface::popScreen();
		return;
	}
	if (forceRedraw || needsRedraw) {
		needsRedraw = false;
	//	if(lcdClear)
	//	{
			lcd.clear();
	//		lcdClear = false;
	//	}
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
    host::HostState state;
	switch (button) {
		case ButtonArray::CENTER:
			break;
        case ButtonArray::LEFT:
            state = host::getHostState();
            if((state == host::HOST_STATE_BUILDING_ONBOARD) ||
                    (state == host::HOST_STATE_BUILDING) ||
                (state == host::HOST_STATE_BUILDING_FROM_SD)){
                    interface::pushScreen(&cancelBuildMenu);
                }
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
	planner::abort();
	Point position = planner::getPosition();	

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

	planner::addMoveToBuffer(position, interval);
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
	singleTool = eeprom::isSingleTool();
    toggleBlink = false;
    heating = false;
    heatLights = true;
    LEDClear = true;
	
}
void MonitorMode::setBuildPercentage(uint8_t percent){
	
	buildPercentage = percent;
}

#define abs(X) ((X) < 0 ? -(X) : (X)) 

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar build_percent[] =    " --%";
	static PROGMEM prog_uchar extruder1_temp[] =   "R Extruder: ---/---C";
	static PROGMEM prog_uchar extruder2_temp[] =   "L Extruder: ---/---C";
	static PROGMEM prog_uchar platform_temp[]  =   "Platform:   ---/---C";
	static PROGMEM prog_uchar extruder_temp[]  =   "Extruder:   ---/---C";
	static PROGMEM prog_uchar clear[] =            "                    ";
    

    Motherboard& board = Motherboard::getBoard();
    
    if(!heating){
		if(board.getExtruderBoard(0).getExtruderHeater().isHeating() ||
            board.getExtruderBoard(1).getExtruderHeater().isHeating() ||
                board.getPlatformHeater().isHeating()){
            heating = true;
            lastHeatIndex = 0;
            lcd.setCursor(0,0);
            heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
            LEDClear = true;
			if(heating){
				lcd.writeString("Heating:            ");
			}
		}
	}
    
    char * name;
	if (forceRedraw) {
                
		lcd.clear();
		lcd.setCursor(0,0);
		if(heating){
			lcd.writeString("Heating:");
			lastHeatIndex = 0;
		}
		else{
			RGB_LED::setDefaultColor();
			LEDClear = true;
			
            switch(host::getHostState()) {
            case host::HOST_STATE_READY:
            case host::HOST_STATE_BUILDING_ONBOARD:
                lcd.writeString(host::getMachineName());
                break;
            case host::HOST_STATE_BUILDING:
            case host::HOST_STATE_BUILDING_FROM_SD:
                name = host::getBuildName();
                while((*name != '.') && (*name != '\0'))
                    lcd.write(*name++);
                    
                lcd.setCursor(16,0);
                lcd.writeFromPgmspace(build_percent);
                
                break;
            case host::HOST_STATE_ERROR:
                lcd.writeString("error!");
                break;
            }
        }

        if(singleTool){
            lcd.setCursor(0,1);
            lcd.writeFromPgmspace(clear);
            
            lcd.setCursor(0,2);
            lcd.writeFromPgmspace(extruder_temp);
        }else{
            lcd.setCursor(0,1);
            lcd.writeFromPgmspace(extruder1_temp);
            
            lcd.setCursor(0,2);
            lcd.writeFromPgmspace(extruder2_temp);
        }

			lcd.setCursor(0,3);
			lcd.writeFromPgmspace(platform_temp);

	}

	OutPacket responsePacket;
	uint16_t data;
	host::HostState state;
    int16_t currentTemp = 0;
    int16_t setTemp = 0; 
    uint16_t heatIndex = 0;
    
    /// show heating progress
    if(heating){
        if(board.getExtruderBoard(0).getExtruderHeater().isHeating()){
            currentTemp += board.getExtruderBoard(0).getExtruderHeater().getDelta();
            setTemp += (int16_t)(board.getExtruderBoard(0).getExtruderHeater().get_set_temperature());
        }
        if(board.getExtruderBoard(1).getExtruderHeater().isHeating()){
            currentTemp += board.getExtruderBoard(1).getExtruderHeater().getDelta();
            setTemp += (int16_t)(board.getExtruderBoard(1).getExtruderHeater().get_set_temperature());
        }
        if(board.getPlatformHeater().isHeating()){
            currentTemp += board.getPlatformHeater().getDelta()*2;
            setTemp += (int16_t)(board.getPlatformHeater().get_set_temperature())*2;
        }
        
        if(currentTemp == 0){
            heating = false;
            //redraw build name
            lcd.setCursor(0,0);
            lcd.writeFromPgmspace(clear);
            lcd.setCursor(0,0);
            switch(host::getHostState()) {
                case host::HOST_STATE_READY:
                case host::HOST_STATE_BUILDING_ONBOARD:
                    lcd.writeString(host::getMachineName());
                    break;
                case host::HOST_STATE_BUILDING:
                case host::HOST_STATE_BUILDING_FROM_SD:
                    name = host::getBuildName();
                    while((*name != '.') && (*name != '\0'))
                        lcd.write(*name++);
                    
                    lcd.setCursor(16,0);
                    lcd.writeFromPgmspace(build_percent);
                    break;
            }
            RGB_LED::setDefaultColor();
            LEDClear = true;
        }
        else{
            
            if(setTemp > 0){
				int32_t mult = 255;
				heatIndex = (abs((setTemp - currentTemp)) * 12) / setTemp;
				if(heatLights){
					RGB_LED::setColor((mult*(setTemp - currentTemp))/setTemp, 0, (mult*currentTemp)/setTemp, LEDClear);
					LEDClear = false;
				}
			}
			if (lastHeatIndex > heatIndex){
				lcd.setCursor(8,0);
				lcd.writeString("            ");
				lastHeatIndex = 0;
			}
            
            lcd.setCursor(8+ lastHeatIndex,0);
            for (int i = lastHeatIndex; i < heatIndex; i++)
                lcd.write(0xFF);
            lastHeatIndex = heatIndex;
            
            
            toggleBlink = !toggleBlink;
            if(toggleBlink)
                lcd.writeString(" ");
            else
                lcd.write(0xFF);
                          
        }
        
        
    }
    
	// Redraw tool info
	switch (updatePhase) {
	case 0:
        if(!singleTool){
            lcd.setCursor(12,1);
			data = board.getExtruderBoard(0).getExtruderHeater().get_current_temperature();
			if(board.getExtruderBoard(0).getExtruderHeater().has_failed()){
				lcd.writeString("  NA    ");
			} else if(board.getExtruderBoard(0).getExtruderHeater().isPaused()){
				lcd.writeString("waiting ");
			} else
				lcd.writeInt(data,3);
			}
		break;

	case 1:
		if(!singleTool){
            if(!board.getExtruderBoard(0).getExtruderHeater().has_failed() && !board.getExtruderBoard(0).getExtruderHeater().isPaused()){           
                data = board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
                if(data > 0){
					lcd.setCursor(15,1);
					lcd.writeString("/   C");
					lcd.setCursor(16,1);
                    lcd.writeInt(data,3);
				}
                else{
                    lcd.setCursor(15,1);
                    lcd.writeString("C    ");
                }
            }
		}
		break;
	case 2:
            lcd.setCursor(12,2);
            data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_current_temperature();
               
            if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().has_failed()){
				lcd.writeString("  NA    ");
			} else if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().isPaused()){
				lcd.writeString("waiting ");
			} 
            else{
                lcd.writeInt(data,3);
			}
		break;
	case 3:
        if(!board.getExtruderBoard(!singleTool * 1).getExtruderHeater().has_failed() && !board.getExtruderBoard(!singleTool * 1).getExtruderHeater().isPaused()){
            lcd.setCursor(16,2);
            data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_set_temperature();
            if(data > 0){
					lcd.setCursor(15,2);
					lcd.writeString("/   C");
					lcd.setCursor(16,2);
                    lcd.writeInt(data,3);
			}else{
                lcd.setCursor(15,2);
                lcd.writeString("C    ");
            }
        }
		break;

	case 4:
            lcd.setCursor(12,3);
			data = board.getPlatformHeater().get_current_temperature();
			if(board.getPlatformHeater().has_failed()){
				lcd.writeString("  NA    ");
			} else if (board.getPlatformHeater().isPaused()){
				lcd.writeString("waiting ");
			} else {
				lcd.writeInt(data,3);
			}
		break;

	case 5:
        if(!board.getPlatformHeater().has_failed() && !board.getPlatformHeater().isPaused()){
            lcd.setCursor(16,3);
            data = board.getPlatformHeater().get_set_temperature();
            if(data > 0){
					lcd.setCursor(15,3);
					lcd.writeString("/   C");
					lcd.setCursor(16,3);
                    lcd.writeInt(data,3);
				}
            else{
                lcd.setCursor(15,3);
                lcd.writeString("C    ");
            }
        }
		break;
	case 6:
		state = host::getHostState();
		if(!heating && ((state == host::HOST_STATE_BUILDING) || (state == host::HOST_STATE_BUILDING_FROM_SD)))
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
			|| forceRedraw || needsRedraw){
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
	else if (lineUpdate){
		lcd.setCursor(1,itemIndex%LCD_SCREEN_HEIGHT);
		drawItem(itemIndex, lcd);
	}
	else {
		// Only need to clear the previous cursor
		lcd.setCursor(0,(lastDrawIndex%LCD_SCREEN_HEIGHT));
		lcd.write(' ');
	}

	lcd.setCursor(0,(itemIndex%LCD_SCREEN_HEIGHT));
    if(((itemIndex%LCD_SCREEN_HEIGHT) == (LCD_SCREEN_HEIGHT - 1)) && (itemIndex < itemCount-1))
        lcd.write(9); //special char "down"
    else if(((itemIndex%LCD_SCREEN_HEIGHT) == 0) && (itemIndex > 0))
        lcd.write('^');
    else    
        lcd.write(8); //special char "right"
	lastDrawIndex = itemIndex;
	lineUpdate = false;
    needsRedraw = false;
}

void Menu::reset() {
	firstItemIndex = 0;
	itemIndex = 0;
	lastDrawIndex = 255;
	lineUpdate = false;
	resetState();
    needsRedraw = false;
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
    firstSelectIndex = 0;
    lastSelectIndex = 255;
    Menu::reset();
}
void CounterMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
    switch (button) {
        case ButtonArray::CENTER:
            if((itemIndex >= firstSelectIndex) && (itemIndex <= lastSelectIndex)){
                selectMode = !selectMode;
			}
			if(selectMode){
				selectIndex = itemIndex;
				lineUpdate = true;
			}
			else{
				selectIndex = -1;
                handleSelect(itemIndex);
                lineUpdate = true;
			}
            break;
        case ButtonArray::LEFT:
			if(!selectMode)
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
    
    counterRight = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET, 220);
    counterLeft = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET, 220);
    counterPlatform = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET, 100);
    
    singleTool = eeprom::isSingleTool();
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
            if(!singleTool){
                lcd.writeFromPgmspace(right);
                if(selectIndex == 1){
                    lcd.setCursor(14,1);
                    lcd.writeString("-->");
                }
                lcd.setCursor(17,1);
                lcd.writeInt(counterRight,3);
            }
            break;
        case 2:
            if(singleTool){
                lcd.writeFromPgmspace(right);
                lcd.setCursor(17,2);
                lcd.writeInt(counterRight,3);
            }else{
                lcd.writeFromPgmspace(left);
                lcd.setCursor(17,2);
                lcd.writeInt(counterLeft,3);
            }
            if(selectIndex == 2){
                lcd.setCursor(14,2);
                lcd.writeString("-->");
            }
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
            if(singleTool){
                // update right counter
                if(up)
                    counterRight++;
                else
                    counterRight--;
                if(counterRight > 260)
                    counterRight = 260;  
            }else{
                // update left counter
                if(up)
                    counterLeft++;
                else
                    counterLeft--;
                if(counterLeft > 260)
                    counterLeft = 260;
            }
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
            if(singleTool){
                // store right tool setting
                eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), counterRight);
            }else{
                // store left tool setting
                eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET), counterLeft);
            }
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
            RGB_LED::setDefaultColor();
            interface::popScreen();
            break;
	}
}

CancelBuildMenu::CancelBuildMenu() {
	itemCount = 4;
	reset();
}

void CancelBuildMenu::resetState() {
	
	 host::HostState state = host::getHostState();
    if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD) ||
            (state == host::HOST_STATE_BUILDING_ONBOARD)){
			itemIndex = 1;
            firstItemIndex = 1;
		}
    else{
		itemIndex = 2;
        firstItemIndex = 2;
	}
    paused = false;
}

void CancelBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar cancel[] = "Cancel this print?";
    static PROGMEM prog_uchar cancel_process[] = "Quit this process?";
        static PROGMEM prog_uchar no[]   =   "No";
        static PROGMEM prog_uchar yes[]  =   "Yes";
    static PROGMEM prog_uchar pause[] = "Pause";
    static PROGMEM prog_uchar unpause[] = "UnPause";

    host::HostState state = host::getHostState();
   
    
    switch (index) {
	case 0:
		if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD))
            lcd.writeFromPgmspace(cancel);
        else{
			command::pause(true);
            lcd.writeFromPgmspace(cancel_process);
		}
		break;
	case 1:
		if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD))
				lcd.writeFromPgmspace(no);
		break;
    case 2:
		if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD)){
				if(paused)
					lcd.writeFromPgmspace(unpause);
				else
					lcd.writeFromPgmspace(pause);
			}
		else
			lcd.writeFromPgmspace(no);
        break;
    case 3:
        lcd.writeFromPgmspace(yes);
        break;
	}
}
void CancelBuildMenu::pop(void){
	command::pause(false);
}

void CancelBuildMenu::handleSelect(uint8_t index) {
    
    host::HostState state = host::getHostState();
    
	switch (index) {
        case 1:
        if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD)){
				// Don't cancel, just close dialog.
				command::pause(false);
				interface::popScreen();
			}
            break;
        case 2:
			if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD)){
				// pause command execution
				paused = !paused;
				command::pause(paused);
				if(!paused){
					interface::popScreen();
				}else{
					for (int i = 3; i < STEPPER_COUNT; i++) 
						steppers::enableAxis(i, false);
				}
				lineUpdate = true;
			}
			else {
				// Don't cancel, just close dialog.
				command::pause(false);
				interface::popScreen();
			}
            break;
        case 3:
            // Cancel build
            command::pause(false);
            host::stopBuild();
            //interface::popScreen();
            break;
	}
}


MainMenu::MainMenu() {
	itemCount = 4;
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
		}
}


UtilitiesMenu::UtilitiesMenu() {
	itemCount = 12;
	stepperEnable = false;
	blinkLED = false;
	reset();
}
void UtilitiesMenu::resetState(){
	singleTool = eeprom::isSingleTool();
}

void UtilitiesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar monitor[] = "Monitor Mode";
	static PROGMEM prog_uchar jog[]   =   "Jog Mode";
	static PROGMEM prog_uchar calibration[] = "Calibrate Axes";
	static PROGMEM prog_uchar home_axes[] = "Home Axes";
	static PROGMEM prog_uchar filament_options[] = "Filament Options";
	static PROGMEM prog_uchar startup[] = "Run Startup Script";
	static PROGMEM prog_uchar Dsteps[] = "Disable Steppers";
	static PROGMEM prog_uchar Esteps[] = "Enable Steppers  ";
	static PROGMEM prog_uchar plate_level[] = "Level Build Plate";
	static PROGMEM prog_uchar led[] = "Blink LEDs       ";
	static PROGMEM prog_uchar led_stop[] = "Stop Blinking!";
	static PROGMEM prog_uchar preheat_settings[] = "Preheat Settings";
    static PROGMEM prog_uchar settings[] = "General Settings";
    static PROGMEM prog_uchar reset[] = "Restore Defaults";
    static PROGMEM prog_uchar nozzles[] = "Calibrate Nozzles";

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(monitor);
		break;
	case 1:
		lcd.writeFromPgmspace(jog);
		break;
	case 2:
		if(stepperEnable)
			lcd.writeFromPgmspace(Esteps);
		else
			lcd.writeFromPgmspace(Dsteps);
		break;
	case 3:
		lcd.writeFromPgmspace(filament_options);
		break;
	case 4:
		lcd.writeFromPgmspace(home_axes);
		break;
	case 5:
		lcd.writeFromPgmspace(plate_level);
		break;
	case 6:
		lcd.writeFromPgmspace(preheat_settings);
		break;
	case 7:
		if(blinkLED)
			lcd.writeFromPgmspace(led_stop);
		else
			lcd.writeFromPgmspace(led);
		break;
	case 8:
		lcd.writeFromPgmspace(startup);
		break;
	case 9:
		lcd.writeFromPgmspace(settings);
		break;
	case 10:
		singleTool = eeprom::isSingleTool();
		if(singleTool)
			lcd.writeFromPgmspace(reset);
		else
			lcd.writeFromPgmspace(nozzles);
		break;	
	case 11:
		if(!singleTool)
			lcd.writeFromPgmspace(reset);
		break;
	}
}

void UtilitiesMenu::handleSelect(uint8_t index) {
	
//	if((index > 3) && singleTool)
//		index++;
		
	switch (index) {
		case 0:
			// Show monitor build screen
                        interface::pushScreen(&monitorMode);
			break;
		case 1:
			// Show build from SD screen
                       interface::pushScreen(&jogger);
			break;
		case 2:
			for (int i = 0; i < STEPPER_COUNT; i++) 
					steppers::enableAxis(i, stepperEnable);
			lineUpdate = true;
			stepperEnable = !stepperEnable;
			break;
		case 3:
			// load filament script
                   interface::pushScreen(&filament);
			break;
		case 4:
			// home axes script
                    host::startOnboardBuild(utility::HOME_AXES);
			break;
		case 5:
			// level_plate script
                    host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
			break;
		case 6:
			interface::pushScreen(&preheat);
			break;
		case 7:
			blinkLED = !blinkLED;
			if(blinkLED)
				RGB_LED::setLEDBlink(150);
			else
				RGB_LED::setLEDBlink(0);
			lineUpdate = true;		 
			 break;
		case 8:
			// startup wizard script
            interface::pushScreen(&welcome);
			break;
		case 9:
			// settings menu
            interface::pushScreen(&set);
			break;
		case 10:
			if(singleTool)
				// restore defaults
				interface::pushScreen(&reset_settings);
			else
				interface::pushScreen(&alignment);
			break;
		case 11:
			if(!singleTool)
				// restore defaults
				interface::pushScreen(&reset_settings);
			break;
		}
}

SettingsMenu::SettingsMenu() {
	itemCount = 5;
    reset();
}

void SettingsMenu::resetState(){
	singleExtruder = eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1);
    soundOn = eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS, 1);
    LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
    heatingLEDOn = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
    helpOn = eeprom::getEeprom8(eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	static PROGMEM prog_uchar tool_count[]   =   "Tool Count ";
	static PROGMEM prog_uchar sound[] =       "Sound";
	static PROGMEM prog_uchar LED[] =             "LED Color     ";
	static PROGMEM prog_uchar LED_heat[] = "Heat LEDs";
	static PROGMEM prog_uchar help_screens[] = "Help Text  ";
    
	switch (index) {
        case 0:
			lcd.writeFromPgmspace(sound);
			 lcd.setCursor(11,0);
			if(selectIndex == 0)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(14,0);
            if(soundOn)
                lcd.writeString("ON ");
            else
                lcd.writeString("OFF");
            break;
        case 1:
            lcd.writeFromPgmspace(LED);
             lcd.setCursor(11,1);
			if(selectIndex == 1)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(14,1);
            switch(LEDColor){
                case LED_DEFAULT_RED:
                    lcd.writeString("RED   ");
                    break;
                case LED_DEFAULT_ORANGE:
                    lcd.writeString("ORANGE ");
                    break;
                case LED_DEFAULT_PINK:
                    lcd.writeString("PINK  ");
                    break;
                case LED_DEFAULT_GREEN:
                    lcd.writeString("GREEN ");
                    break;
                case LED_DEFAULT_BLUE:
                    lcd.writeString("BLUE  ");
                    break;
                case LED_DEFAULT_PURPLE:
                    lcd.writeString("PURPLE");
                    break;
                case LED_DEFAULT_WHITE:
                    lcd.writeString("WHITE ");
                    break;
                case LED_DEFAULT_CUSTOM:
					lcd.writeString("CUSTOM");
					break;
            }
            break;
        case 2:
			lcd.writeFromPgmspace(tool_count);
			lcd.setCursor(11,2);
			if(selectIndex == 2)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(14,2);
            if(singleExtruder == 1)
                lcd.writeString("SINGLE");
            else
                lcd.writeString("DUAL  ");
            break;
         case 3:
			lcd.writeFromPgmspace(LED_heat);
			 lcd.setCursor(11,3);
			if(selectIndex == 3)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(14,3);
            if(heatingLEDOn)
                lcd.writeString("ON ");
            else
                lcd.writeString("OFF");
            break;
          case 4:
			lcd.writeFromPgmspace(help_screens);
			 lcd.setCursor(11,0);
			if(selectIndex == 4)
                lcd.writeString("-->");
            else
				lcd.writeString("   ");
            lcd.setCursor(14,0);
            if(helpOn)
                lcd.writeString("ON ");
            else
                lcd.writeString("OFF");
            break;
 	}
}

void SettingsMenu::handleCounterUpdate(uint8_t index, bool up){
    switch (index) {
        case 0:
            // update right counter
            if(up)
                soundOn++;
            else
                soundOn--;
            // keep within appropriate boundaries    
            if(soundOn > 1)
                soundOn = 0;
            else if(soundOn < 0)
				soundOn = 1;
            break;
        case 1:
            // update left counter
            if(up)
                LEDColor++;
            else
                LEDColor--;
            // keep within appropriate boundaries
            if(LEDColor > 6)
                LEDColor = 0;
            else if(LEDColor < 0)
				LEDColor = 6;
			
			eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
            RGB_LED::setDefaultColor();	
			
            break;
        case 2:
            // update platform counter
            // update right counter
            if(up)
                singleExtruder++;
            else
                singleExtruder--;
            // keep within appropriate boundaries    
            if(singleExtruder > 2)
                singleExtruder = 1;
            else if(singleExtruder < 1)
				singleExtruder = 2;			
            break;
        case 3:
            // update right counter
            if(up)
                heatingLEDOn++;
            else
                heatingLEDOn--;
            // keep within appropriate boundaries    
            if(heatingLEDOn > 1)
                heatingLEDOn = 0;
            else if(heatingLEDOn < 0)
				heatingLEDOn = 1;
            break;
        case 4:
            // update right counter
            if(up)
                helpOn++;
            else
                helpOn--;
            // keep within appropriate boundaries    
            if(helpOn > 1)
                helpOn = 0;
            else if(helpOn < 0)
				helpOn = 1;
			break;
	}
    
}


void SettingsMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// update sound preferences
            eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS, soundOn);
            lineUpdate = 1;
			break;
		case 1:
			// update LED preferences
            eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS, LEDColor);
            RGB_LED::setDefaultColor();
            lineUpdate = 1;
			break;
		case 2:
			// update tool count
            eeprom::setToolHeadCount(singleExtruder);
            if(singleExtruder)
				Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
            lineUpdate = 1;
			break;
		case 3:
			// update LEDHeatingflag
			eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, heatingLEDOn);
			lineUpdate = 1;
			break;
		case 4:
			eeprom_write_byte((uint8_t*)eeprom_offsets::FILAMENT_HELP_SETTINGS, helpOn);
			lineUpdate = 1;
			break;
    }
}

/*SDSpecialBuild::SDSpecialBuild(){
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
*/
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
        interface::popScreen();
        Motherboard::getBoard().errorResponse(" SD card read error");
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
		Motherboard::getBoard().errorResponse(" I'm already building");
		return;
	}
		
	char* buildName = host::getBuildName();

    if ( !getFilename(index, buildName, host::MAX_FILE_LEN) ) {
        interface::popScreen();
		Motherboard::getBoard().errorResponse(" SD card read error");
		return;
	}

    sdcard::SdErrorCode e;
	e = host::startBuildFromSD();
	
	if (e != sdcard::SD_SUCCESS) {
        interface::popScreen();
		Motherboard::getBoard().errorResponse(" SD card read error");
		return;
	}
}

#endif
