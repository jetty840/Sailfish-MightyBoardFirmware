#include "Menu.hh"
#include "Configuration.hh"

// TODO: Kill this, should be hanlded by build system.
#ifdef HAS_INTERFACE_BOARD

#include "Main.hh"
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
#include "stdio.h"
#include "Piezo.hh"
#include "Menu_locales.hh"


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

uint32_t homePosition[PROFILES_HOME_POSITIONS_STORED];

void SplashScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {


	if (forceRedraw || hold_on) {
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(SPLASH1_MSG);

		lcd.setCursor(0,1);
#ifdef STACK_PAINT
		if ( hold_on ) {
			lcd.writeFromPgmspace(CLEAR_MSG);
			lcd.setCursor(0,1);
               		lcd.writeString((char *)"Free SRAM ");
                	lcd.writeFloat((float)StackCount(), 0, LCD_SCREEN_WIDTH);
		}
		else lcd.writeFromPgmspace(SPLASH2_MSG);
#else
		lcd.writeFromPgmspace(SPLASH2_MSG);
#endif

		// display internal version number if it exists
		if (internal_version != 0){
			lcd.setCursor(0,2);
			lcd.writeFromPgmspace(SPLASH5_MSG);
			
			lcd.setCursor(17,2);
			lcd.writeInt((uint16_t)internal_version,3);
		} else {
			lcd.setCursor(0,2);
			lcd.writeFromPgmspace(SPLASH3_MSG);
		}

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(SPLASH4_MSG);
		
		/// get major firmware version number
		uint16_t major_digit = firmware_version / 100;
		/// get minor firmware version number
		uint16_t minor_digit = firmware_version % 100;
		lcd.setCursor(17,3);
		lcd.writeInt(major_digit, 1);
		/// period is written as part of SLASH4_MSG
		lcd.setCursor(19,3);
		lcd.writeInt(minor_digit, 1);
	}
	else if (!hold_on) {
	//	 The machine has started, so we're done!
                interface::popScreen();
    }
}

void SplashScreen::SetHold(bool on){
	hold_on = on;
}

void SplashScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	// We can't really do anything, since the machine is still loading, so ignore.
	switch (button) {
		case ButtonArray::CENTER:
			interface::popScreen();
			SetHold(false);
			break;
        case ButtonArray::LEFT:
			interface::popScreen();
			SetHold(false);
			break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
		default:
			break;

	}
}

void SplashScreen::reset() {
	
}

HeaterPreheat::HeaterPreheat(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4),
	monitorMode((uint8_t)0) {
	reset();
}

void HeaterPreheat::resetState(){
    uint8_t heatSet = eeprom::getEeprom8(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET, 0);
	_rightActive = (heatSet & (1 << HEAT_MASK_RIGHT)) != 0;
    _platformActive = (heatSet & (1 << HEAT_MASK_PLATFORM)) != 0;
	_leftActive = (heatSet & (1 << HEAT_MASK_LEFT)) != 0;
	singleTool = eeprom::isSingleTool();
	if(singleTool){ _leftActive = false; }
    Motherboard &board = Motherboard::getBoard();
    if(((board.getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0) || !_rightActive) &&
        ((board.getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0) || !_leftActive) &&
        ((board.getPlatformHeater().get_set_temperature() >0) || !_platformActive))
       preheatActive = true;
    else
       preheatActive = false;
}

void HeaterPreheat::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	switch (index) {
	case 0:
        if(preheatActive)
            lcd.writeFromPgmspace(STOP_MSG);
        else
            lcd.writeFromPgmspace(GO_MSG);
		break;
	case 1:
		if(!singleTool){
			lcd.writeFromPgmspace(RIGHT_TOOL_MSG);
			lcd.setCursor(16,1);
			if(_rightActive)
				lcd.writeFromPgmspace(ON_MSG);
			else
				lcd.writeFromPgmspace(OFF_MSG);
		}
		break;
	case 2:
        
		if(singleTool){
			lcd.writeFromPgmspace(TOOL_MSG);
			lcd.setCursor(16,2);
			if(_rightActive)
				lcd.writeFromPgmspace(ON_MSG);
			else
				lcd.writeFromPgmspace(OFF_MSG);
		}
		else{
			lcd.writeFromPgmspace(LEFT_TOOL_MSG);
			lcd.setCursor(16,2);
			if(_leftActive)
				lcd.writeFromPgmspace(ON_MSG);
			else
				lcd.writeFromPgmspace(OFF_MSG);
		}
		break;
	case 3:
		lcd.writeFromPgmspace(PLATFORM_MSG);
        lcd.setCursor(16,3);
		if(_platformActive)
			lcd.writeFromPgmspace(ON_MSG);
		else
			lcd.writeFromPgmspace(OFF_MSG);
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
            // clear paused state if any
            Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().Pause(false);
            Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().Pause(false);
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
    
    
    
	if (forceRedraw || needsRedraw) {
	//	waiting = true;
		lcd.setCursor(0,0);
        switch (welcomeState){
            case WELCOME_START:
                lcd.writeFromPgmspace(START_MSG);
                _delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
            case WELCOME_BUTTONS1:
				lcd.writeFromPgmspace(BUTTONS1_MSG);
				_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                 break;
            case WELCOME_BUTTONS2:
				lcd.writeFromPgmspace(BUTTONS2_MSG);
				_delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);
                
                 break;
			case WELCOME_EXPLAIN:
                lcd.writeFromPgmspace(EXPLAIN_MSG);
                _delay_us(1000000);
                Motherboard::getBoard().interfaceBlink(25,15);

                break;
            case WELCOME_LEVEL:
                lcd.writeFromPgmspace(LEVEL_MSG);
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
					lcd.writeFromPgmspace(BETTER_MSG);
				} else if(levelSuccess == FAIL){
					lcd.writeFromPgmspace(TRYAGAIN_MSG);
					welcomeState = WELCOME_LEVEL;
				} else if(levelSuccess == SECOND_FAIL){
					lcd.writeFromPgmspace(GO_ON_MSG);
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
                    lcd.writeFromPgmspace(FAIL_MSG);
                    welcomeState++;
                }
                else
                 lcd.writeFromPgmspace(SD_MENU_MSG);
                 _delay_us(1000000);
                 Motherboard::getBoard().interfaceBlink(25,15);
                
                break;
            case WELCOME_DONE:
				host::stopBuildNow();
                interface::popScreen();
                break;
            
        }
        needsRedraw = false;
	}
}

void WelcomeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	
	Point position;

	FilamentScreen *filament = &Motherboard::getBoard().mainMenu.utils.filament.filament;
	
	switch (button) {
		case ButtonArray::CENTER:
           welcomeState++;
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
                        filament->setScript(FILAMENT_STARTUP_SINGLE);
					else
                        filament->setScript(FILAMENT_STARTUP_DUAL);
					interface::pushScreen(filament);
                    break;
                case WELCOME_PRINT_FROM_SD:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    interface::pushScreen(&Motherboard::getBoard().mainMenu.sdMenu);
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
                        filament->setScript(FILAMENT_STARTUP_SINGLE);
					else
                        filament->setScript(FILAMENT_STARTUP_DUAL);
					interface::pushScreen(filament);
                    break;
                case WELCOME_PRINT_FROM_SD:
                     Motherboard::getBoard().interfaceBlink(0,0);
                    welcomeState++;
                    interface::pushScreen(&Motherboard::getBoard().mainMenu.sdMenu);
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
	default:
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
    
	if (forceRedraw || needsRedraw) {
		lcd.setCursor(0,0);
        switch (alignmentState){
            case ALIGNMENT_START:
                lcd.writeFromPgmspace(START_TEST_MSG);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN1:
				lcd.writeFromPgmspace(EXPLAIN1_MSG);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_EXPLAIN2:
				lcd.writeFromPgmspace(EXPLAIN2_MSG);
                _delay_us(500000);
                Motherboard::getBoard().interfaceBlink(25,15);    
                 break;
            case ALIGNMENT_SELECT:
				Motherboard::getBoard().interfaceBlink(0,0);
				interface::pushScreen(&align);
				 alignmentState++;
                 break;
            case ALIGNMENT_END:
				lcd.writeFromPgmspace(END_MSG);
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
			interface::pushScreen(&Motherboard::getBoard().mainMenu.utils.monitorMode.cancelBuildMenu);
			break;
			
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
	default:
		break;
	}
}

void NozzleCalibrationScreen::reset() {
    needsRedraw = false;
    Motherboard::getBoard().interfaceBlink(25,15);
    alignmentState=ALIGNMENT_START;
}

SelectAlignmentMenu::SelectAlignmentMenu(uint8_t optionsMask) :
	CounterMenu(optionsMask, (uint8_t)4) {
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
	
    
	switch (index) {
		case 0:
			lcd.writeFromPgmspace(SELECT_MSG);
			break;
        case 1:
			lcd.writeFromPgmspace(XAXIS_MSG);
			lcd.setCursor(13,1);
			if(selectIndex == 1)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(17,1);
            lcd.writeInt(xCounter,2);
            break;
         case 2:
			lcd.writeFromPgmspace(YAXIS_MSG);
			 lcd.setCursor(13,2);
			if(selectIndex == 2)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(17,2);
            lcd.writeInt(yCounter,2);
            break;
         case 3:
			lcd.writeFromPgmspace(DONE_MSG);
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
			offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0)) + (int32_t)((xCounter-7)*stepperAxisStepsPerMM(X_AXIS) *0.1f * 10);
            eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 4);
            lineUpdate = 1;
			break;
		case 2:
			// update toolhead offset (tool tolerance setting)
			offset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 0)) + (int32_t)((yCounter-7)*stepperAxisStepsPerMM(Y_AXIS) *0.1f * 10);
			eeprom_write_block((uint8_t*)&offset, (uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, 4);
			lineUpdate = 1;
			break;
		case 3:
			interface::popScreen();
			break;
    }
}

void FilamentScreen::startMotor(){
    Piezo::playTune(TUNE_FILAMENT_START);

    //So we don't prime after a pause
    command::pauseUnRetractClear();

    int32_t interval = 300000000;  // 5 minutes
    int32_t steps = interval / 6250;
    if(forward)
        steps *= -1;
    Point target = Point(0,0,0,0,0);
    target[axisID] += steps;

    //Backup the digi pot entry and switch the pot to high
    //This is because we might be changing filament when we paused during
    //the heating phase when the pots were turned low
    digiPotOnEntry = steppers::getAxisPotValue(axisID);
    steppers::resetAxisPot(axisID);

    steppers::deprimeEnable(false);
    //All axis are relative

    //Backup the currently enabled axis, we need to return
    //to this state when we're done with loading/unloading filament
    restoreAxesEnabled = steppers::allAxesEnabled();

    steppers::setTargetNew(target, interval, 0x1f);
    filamentTimer.clear();
    filamentTimer.start(300000000); //5 minutes
}

void FilamentScreen::stopMotor(){
    steppers::abort(); 
    steppers::deprimeEnable(true);

    //Restore the enabled axis
    for(int i = 0; i < STEPPER_COUNT; i++)
        steppers::enableAxis(i, restoreAxesEnabled & _BV(i));

    //Restore the digi pot setting from entry
    steppers::setAxisPotValue(axisID, digiPotOnEntry);
}

//Renamed to zabs because of conflict with stdlib.h abs
#define zabs(X) ((X) < 0 ? -(X) : (X)) 

void FilamentScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    
    
    Point target = Point(0,0,0, 0,0);
    int32_t interval;
    
	if(filamentState == FILAMENT_WAIT){
		
		/// if extruder has reached hot temperature, start extruding
		if(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().has_reached_target_temperature()){
			
			int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			/// check for externally manipulated temperature (eg by RepG)
			if(setTemp < FILAMENT_HEAT_TEMP){
					Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
					interface::popScreen();
					Motherboard::getBoard().errorResponse(EXTEMP_CHANGE_MSG);
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
		/// if heating timer has eleapsed, alert user that the heater is not getting hot as expected
		else if (filamentTimer.hasElapsed()){
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(HEATER_ERROR_MSG);
            Motherboard::getBoard().interfaceBlink(25,15);
            filamentState = FILAMENT_DONE;
		}
		/// if extruder is still heating, update heating bar status
		else{
            int16_t currentTemp = Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().getDelta();
            int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
            // check for externally manipulated temperature (eg by RepG)
			if(setTemp < FILAMENT_HEAT_TEMP){
					Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
					interface::popScreen();
					Motherboard::getBoard().errorResponse(EXTEMP_CHANGE_MSG);
					return;
			}
				
			uint8_t heatIndex = (zabs((setTemp - currentTemp)) * 20) / setTemp;
			
			int32_t mult = 255;
			if(heatLights){
				RGB_LED::setColor((mult*(setTemp - currentTemp))/setTemp, 0, (mult*currentTemp)/setTemp, LEDClear);
				LEDClear = false;
			}
            
            if (lastHeatIndex > heatIndex){
				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(CLEAR_MSG);
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
					lcd.writeFromPgmspace(BLANK_CHAR_MSG);
				else
					lcd.write(0xFF);
				toggleCounter = 0;
			}
                          
			}
	}
	/// if not in FILAMENT_WAIT state and the motor times out (5 minutes) alert the user
	else if(filamentTimer.hasElapsed()){
		if(startup){
			filamentState = FILAMENT_OK;
			interface::pushScreen(&filamentOK);
		}
		else {
			filamentState = FILAMENT_TIMEOUT;
			filamentTimer = Timeout();
			needsRedraw = true;
		}
    }

	
	if (forceRedraw || needsRedraw) {
        //	waiting = true;
		lcd.setCursor(0,0);
		lastHeatIndex = 0;
        switch (filamentState){
			/// starting state - set hot temperature for desired tool and start heat up timer
			case FILAMENT_HEATING:
				Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().Pause(false);
				Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().set_target_temperature(FILAMENT_HEAT_TEMP);
				if(dual){
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().Pause(false);
					Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(FILAMENT_HEAT_TEMP);			
				}
				/// if running the startup script, go through the explanatory text
				if(startup){
					if(dual)
						lcd.writeFromPgmspace(EXPLAIN_ONE_MSG);
					else
						lcd.writeFromPgmspace(EXPLAIN_ONE_S_MSG);
					Motherboard::getBoard().interfaceBlink(25,15);
					_delay_us(1000000);
				}
				else{
					lcd.writeFromPgmspace(HEATING_BAR_MSG);
					lastHeatIndex = 0;
					heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
					filamentState = FILAMENT_WAIT;
				}
				filamentTimer.clear();
				filamentTimer.start(300000000); //5 minutes
				
				break;
			/// startup script explanation screen
			case FILAMENT_EXPLAIN2:
				if(dual)
					lcd.writeFromPgmspace(EXPLAIN_TWO_MSG);
				else
					lcd.writeFromPgmspace(EXPLAIN_TWO_S_MSG);
				Motherboard::getBoard().interfaceBlink(25,15);
					_delay_us(1000000);
				break;
			/// startup script explanation screen
			case FILAMENT_EXPLAIN3:
				lcd.writeFromPgmspace(EXPLAIN_THRE_MSG);
				Motherboard::getBoard().interfaceBlink(25,15);
			    _delay_us(1000000);
				break;
			/// startup script explanation screen
			case FILAMENT_EXPLAIN4:
				lcd.writeFromPgmspace(EXPLAIN_FOUR_MSG);			
				//_delay_us(1000000);
				// if z stage is at zero, move z stage down
				target = steppers::getPlannerPosition();
				if(target[2] < 1000){
					target[2] = 60000;
					interval = 5000000;
					steppers::setTargetNew(target, interval, 0x1f);
				}
				_delay_us(1000000);
				Motherboard::getBoard().interfaceBlink(25,15);
				break;
			/// show heating bar status after explanations are complete
			case FILAMENT_HEAT_BAR:
				lcd.writeFromPgmspace(HEATING_BAR_MSG);
				_delay_us(3000000);
				/// go to FILAMENT_WAIT state
				filamentState++;
				heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
				break;
			/// show heating bar status
			case FILAMENT_WAIT:
				if(startup)
					lcd.writeFromPgmspace(HEATING_BAR_MSG);
				else
				    lcd.writeFromPgmspace(HEATING_PROG_MSG);
				break;
			/// alert user that filament is ready to extrude
            case FILAMENT_START:
                if(dual){
					if(axisID == 3)
						lcd.writeFromPgmspace(READY_RIGHT_MSG);
					else
						lcd.writeFromPgmspace(READY_LEFT_MSG);
				}
				else if(forward)
					lcd.writeFromPgmspace(READY_SINGLE_MSG);
				else{
					lcd.writeFromPgmspace(READY_REV_MSG);
					filamentState++;
				}	
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(100000);
                break;
            /// alert user that filament is reversing
            case FILAMENT_TUG:
				lcd.writeFromPgmspace(TUG_MSG);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(100000);
                break;
            /// alert user to press M to stop extusion / reversal
            case FILAMENT_STOP:
				if(startup)
					lcd.writeFromPgmspace(STOP_MSG_MSG);
				else{
					if(forward)
						lcd.writeFromPgmspace(STOP_EXIT_MSG);
					else 
						lcd.writeFromPgmspace(STOP_REVERSE_MSG);
				}
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                break;
            case FILAMENT_DONE:
				/// user indicated that filament has extruded
                stopMotor();
                if(startup){
					if(filamentSuccess == SUCCESS){
						if(dual && (axisID ==3)){
							axisID = 4;
							lastHeatIndex = 0;
							filamentState = FILAMENT_START;
							startMotor();
							lcd.writeFromPgmspace(READY_LEFT_MSG);
						}
						else
							lcd.writeFromPgmspace(FINISH_MSG);
					} else{
					  if(filamentSuccess == FAIL){
						lcd.writeFromPgmspace(PUSH_HARDER_MSG);
						startMotor();
						filamentState = FILAMENT_TUG;
					  } else if(filamentSuccess == SECOND_FAIL){ 
						  if(dual && (axisID ==3)){
							 axisID = 4;
							 filamentState = FILAMENT_TUG;
							 startMotor();
							 lcd.writeFromPgmspace(GO_ON_LEFT_MSG);
							}
							else{
								lcd.writeFromPgmspace(KEEP_GOING_MSG);
							}
					  }
					}
				}
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                
                break;
            case FILAMENT_TIMEOUT:
				/// filament motor has been running for 5 minutes
				stopMotor();
				lcd.writeFromPgmspace(TIMEOUT_MSG);
				filamentState = FILAMENT_DONE;
				Motherboard::getBoard().interfaceBlink(25,15);
                
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
    /// load settings for correct tool and direction
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
				/// go to interactive 'OK' scrreen
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
                /// exit out of filament menu system
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
			interface::pushScreen(&Motherboard::getBoard().mainMenu.utils.monitorMode.cancelBuildMenu);			
            break;			
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
        default:
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

ReadyMenu::ReadyMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4) {
	reset();
}

void ReadyMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ReadyMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(READY1_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(READY2_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(YES_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(NO_MSG);
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

LevelOKMenu::LevelOKMenu(uint8_t optionsMask) :
	Menu(optionsMask | IS_STICKY_MASK, (uint8_t)4) {
	reset();
}

void LevelOKMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void LevelOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(NOZZLE_MSG_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(HEIGHT_CHK_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(HEIGHT_GOOD_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(TRY_AGAIN_MSG);
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

FilamentOKMenu::FilamentOKMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4) {
	reset();
}

void FilamentOKMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void FilamentOKMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(QONE_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(QTWO_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(YES_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(NO_MSG);
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"

FilamentMenu::FilamentMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4),
	filament((uint8_t)0) {
	reset();
}

#pragma GCC diagnostic pop

void FilamentMenu::resetState() {
    singleTool = eeprom::isSingleTool();
    if(singleTool)
        itemCount = 2;
	itemIndex = 0;
	firstItemIndex = 0;
}

void FilamentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
    
    singleTool = eeprom::isSingleTool();
    if(singleTool)
        itemCount = 2;
    else
		itemCount = 4;
    
	switch (index) {
        case 0:
            if(!singleTool){
                lcd.writeFromPgmspace(LOAD_RIGHT_MSG);
            } else {
                lcd.writeFromPgmspace(LOAD_SINGLE_MSG);
            }
            break;
        case 1:
            if(!singleTool){
                lcd.writeFromPgmspace(UNLOAD_RIGHT_MSG);
            } else {
                lcd.writeFromPgmspace(UNLOAD_SINGLE_MSG);
            }
            break;
        case 2:
            if(!singleTool){
                lcd.writeFromPgmspace(LOAD_LEFT_MSG);
            }
            break;
        case 3:
            if(!singleTool){
                lcd.writeFromPgmspace(UNLOAD_LEFT_MSG);
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

void MessageScreen::addMessage(CircularBuffer& buf) {
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
	}
}


void MessageScreen::addMessage(const prog_uchar msg[]) {

	cursor += strlcpy_P(message + cursor, (const prog_char *)msg, BUF_SIZE - cursor);
		
	// ensure that message is always null-terminated
	if (cursor == BUF_SIZE) {
		message[BUF_SIZE-1] = '\0';
	} else {
		message[cursor] = '\0';
	}
}

#if 0
void MessageScreen::addMessage(char msg[]) {

	char* letter = msg;
	while (*letter != 0) {
		message[cursor++] = *letter;
		letter++;
	}
		
	// ensure that message is always null-terminated
	if (cursor == BUF_SIZE) {
		message[BUF_SIZE-1] = '\0';
	} else {
		message[cursor] = '\0';
	}
}

#endif

void MessageScreen::clearMessage() {
	x = y = 0;
	message[0] = '\0';
	cursor = 0;
	needsRedraw = false;
	timeout = Timeout();
	incomplete = false;
}

void MessageScreen::setTimeout(uint8_t seconds) {
	timeout.start((micros_t)seconds * 1000L * 1000L);
}
void MessageScreen::refreshScreen(){
	needsRedraw = true;
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
	timeout = Timeout();
	buttonsDisabled = false;
}

void MessageScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	if ( buttonsDisabled )	return;

    host::HostState state;
	switch (button) {
		case ButtonArray::CENTER:
			break;
        case ButtonArray::LEFT:
            state = host::getHostState();
            if((state == host::HOST_STATE_BUILDING_ONBOARD) ||
                    (state == host::HOST_STATE_BUILDING) ||
                (state == host::HOST_STATE_BUILDING_FROM_SD)){
                    interface::pushScreen(&Motherboard::getBoard().mainMenu.utils.monitorMode.cancelBuildMenu);
                }
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
        default:
                break;
	}
}

void JogMode::reset() {
	jogDistance = DISTANCE_CONT;
	jogging = false;
	distanceChanged = modeChanged = false;
	JogModeScreen = JOG_MODE_X;
}


void JogMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	//Stop all movement when a button is released
	if ((jogging) && (!interface::isButtonPressed(ButtonArray::DOWN)) &&
	    (!interface::isButtonPressed(ButtonArray::UP)))
	{
		jogging = false;
		steppers::abort();
	}

	if (forceRedraw || distanceChanged || modeChanged) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(JOG1_MSG);

		switch (JogModeScreen){
			case JOG_MODE_X:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(JOG2X_MSG);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(JOG3X_MSG);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(JOG4X_MSG);
				break;
			case JOG_MODE_Y:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(JOG2Y_MSG);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(JOG3Y_MSG);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(JOG4Y_MSG);
				break;
			case JOG_MODE_Z:
				lcd.setCursor(0,1);
				lcd.writeFromPgmspace(JOG2Z_MSG);

				lcd.setCursor(0,2);
				lcd.writeFromPgmspace(JOG3Z_MSG);

				lcd.setCursor(0,3);
				lcd.writeFromPgmspace(JOG4Z_MSG);
				break;
		}

		distanceChanged = false;
		modeChanged = false;
	}
}

void JogMode::jog(ButtonArray::ButtonName direction) {
	steppers::abort();
	Point position = steppers::getStepperPosition();	

	int32_t interval = 500;
	int32_t steps = 20;

	switch(jogDistance) {
	case DISTANCE_SHORT:
		steps = 20;
		break;
	case DISTANCE_LONG:
		steps = 3000;
		break;
	case DISTANCE_CONT:	//Continuous movement, no clunks
		steps = (INT32_MAX - 1) >> 1;
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
		        default:
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
		        default:
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
        		default:
                		break;
		}
	}
	
	if ( direction == ButtonArray::UP || direction == ButtonArray::DOWN )
		steppers::setTarget(position, interval);
}

void JogMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
		case ButtonArray::CENTER:
           interface::popScreen();
           for(int i = 0; i < STEPPER_COUNT; i++)
			steppers::enableAxis(i, false);
		break;
        case ButtonArray::DOWN:
        case ButtonArray::UP:
		jogging = true;
        case ButtonArray::LEFT:
        case ButtonArray::RIGHT:
		jog(button);
		break;
        default:
                break;
	}
}

void FilamentOdometerScreen::reset() {
	needsRedraw = false;
}


// Print the filament used, right justified.  Written in C to save space as it's
// used 3 times.  Takes filamentUsed in millimeters

void printFilamentUsed(float filamentUsed, uint8_t yOffset, LiquidCrystalSerial& lcd) {
	uint8_t precision;

	filamentUsed /= 1000.0; //convert to meters
	if      ( filamentUsed < 0.1 )  {
		filamentUsed *= 1000.0;        //Back to mm's
		precision = 1;
	}
	else if ( filamentUsed < 10.0 )  precision = 4;
	else if ( filamentUsed < 100.0 ) precision = 3;
	else                             precision = 2;

	lcd.setCursor(0, yOffset);
	lcd.writeFloat(filamentUsed, precision, LCD_SCREEN_WIDTH - ((precision == 1) ? 2 : 1));
	lcd.writeFromPgmspace((precision == 1) ? MILLIMETERS_MSG : METERS_MSG);
}

void FilamentOdometerScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw || needsRedraw) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(FILAMENT_ODOMETER_MSG);
		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(FILAMENT_LIFETIME_MSG);
		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(FILAMENT_TRIP_MSG);
		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(FILAMENT_RESET_TRIP_MSG);

		// Get lifetime filament used for A & B axis and sum them into filamentUsed
		float filamentUsedA, filamentUsedB;
                filamentUsedA = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0),                  A_AXIS);
		filamentUsedB = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t),0), B_AXIS);
		printFilamentUsed(filamentUsedA + filamentUsedB, 1, lcd);

		// Get trip filament used for A & B axis and sum them into filamentUsed
                filamentUsedA -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP, 0),                  A_AXIS);
		filamentUsedB -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t),0), B_AXIS);
		printFilamentUsed(filamentUsedA + filamentUsedB, 2, lcd);

		needsRedraw = false;
	}
}

void FilamentOdometerScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
	case ButtonArray::CENTER:
		eeprom::setEepromInt64(eeprom_offsets::FILAMENT_TRIP, eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0));
		eeprom::setEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t), eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t), 0));
		needsRedraw = true;
		break;
        case ButtonArray::DOWN:
        case ButtonArray::UP:
        case ButtonArray::RIGHT:
		break;
        case ButtonArray::LEFT:
        	interface::popScreen();
		break;
        default:
                break;
	}
}

void SnakeMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {

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
			lcd.writeFromPgmspace(GAMEOVER_MSG);
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
        default:
                break;
	}
}


void MonitorMode::reset() {
	updatePhase = 0;
	singleTool = eeprom::isSingleTool();
    toggleBlink = false;
    heating = false;
    heatLights = true;
    LEDClear = true;
	
}

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    

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
				lcd.writeFromPgmspace(HEATING_SPACES_MSG);
			}
		}
	}
    
    char * name;
	if (forceRedraw) {
                
		lcd.clear();
		lcd.setCursor(0,0);
		if(heating){
			lcd.writeFromPgmspace(HEATING_MSG);
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
                lcd.writeFromPgmspace(BUILD_PERCENT_MSG);
                
                break;
            case host::HOST_STATE_ERROR:
                lcd.writeFromPgmspace(ERROR_MSG);
                break;
	    default:
		break;
            }
        }

        if(singleTool){
            lcd.setCursor(0,1);
            lcd.writeFromPgmspace(CLEAR_MSG);
            
            lcd.setCursor(0,2);
            lcd.writeFromPgmspace(EXTRUDER_TEMP_MSG);
        }else{
            lcd.setCursor(0,1);
            lcd.writeFromPgmspace(EXTRUDER1_TEMP_MSG);
            
            lcd.setCursor(0,2);
            lcd.writeFromPgmspace(EXTRUDER2_TEMP_MSG);
        }

			lcd.setCursor(0,3);
			lcd.writeFromPgmspace(PLATFORM_TEMP_MSG);

	}

	OutPacket responsePacket;
	uint16_t data;
	host::HostState state;
    int16_t currentTemp = 0;
    int16_t setTemp = 0; 
    uint16_t heatIndex = 0;
    
    /// show heating progress
    if(heating){
        if(board.getExtruderBoard(0).getExtruderHeater().isHeating()  && !board.getExtruderBoard(0).getExtruderHeater().isPaused()){
            currentTemp += board.getExtruderBoard(0).getExtruderHeater().getDelta();
            setTemp += (int16_t)(board.getExtruderBoard(0).getExtruderHeater().get_set_temperature());
        }
        if(board.getExtruderBoard(1).getExtruderHeater().isHeating() && !board.getExtruderBoard(1).getExtruderHeater().isPaused()){
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
            lcd.writeFromPgmspace(CLEAR_MSG);
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
                    lcd.writeFromPgmspace(BUILD_PERCENT_MSG);
                    break;
	        default:
          	    break;

            }
            RGB_LED::setDefaultColor();
            LEDClear = true;
        }
        else{
            
            if(setTemp > 0){
				int32_t mult = 255;
				heatIndex = (zabs((setTemp - currentTemp)) * 12) / setTemp;
				if(heatLights){
					RGB_LED::setColor((mult*(setTemp - currentTemp))/setTemp, 0, (mult*currentTemp)/setTemp, LEDClear);
					LEDClear = false;
				}
			}
			if (lastHeatIndex > heatIndex){
				lcd.setCursor(8,0);
				lcd.writeString((char *)"            ");
				lastHeatIndex = 0;
			}
            
            lcd.setCursor(8+ lastHeatIndex,0);
            for (uint16_t i = lastHeatIndex; i < heatIndex; i++)
                lcd.write(0xFF);
            lastHeatIndex = heatIndex;
            
            
            toggleBlink = !toggleBlink;
            if(toggleBlink)
                lcd.writeFromPgmspace(BLANK_CHAR_MSG);
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
				lcd.writeFromPgmspace(NA_MSG);
			} else if(board.getExtruderBoard(0).getExtruderHeater().isPaused()){
				lcd.writeFromPgmspace(WAITING_MSG);
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
					lcd.writeFromPgmspace(ON_CELCIUS_MSG);
					lcd.setCursor(16,1);
                    lcd.writeInt(data,3);
				}
                else{
                    lcd.setCursor(15,1);
                    lcd.writeFromPgmspace(CELCIUS_MSG);
                }
            }
		}
		break;
	case 2:
            lcd.setCursor(12,2);
            data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_current_temperature();
               
            if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().has_failed()){
				lcd.writeFromPgmspace(NA_MSG);
			} else if(board.getExtruderBoard(!singleTool * 1).getExtruderHeater().isPaused()){
				lcd.writeFromPgmspace(WAITING_MSG);
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
					lcd.writeFromPgmspace(ON_CELCIUS_MSG);
					lcd.setCursor(16,2);
                    lcd.writeInt(data,3);
			}else{
                lcd.setCursor(15,2);
                lcd.writeFromPgmspace(CELCIUS_MSG);
            }
        }
		break;

	case 4:
            lcd.setCursor(12,3);
			data = board.getPlatformHeater().get_current_temperature();
			if(board.getPlatformHeater().has_failed()){
				lcd.writeFromPgmspace(NA_MSG);
			} else if (board.getPlatformHeater().isPaused()){
				lcd.writeFromPgmspace(WAITING_MSG);
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
					lcd.writeFromPgmspace(ON_CELCIUS_MSG);
					lcd.setCursor(16,3);
                    lcd.writeInt(data,3);
				}
            else{
                lcd.setCursor(15,3);
                lcd.writeFromPgmspace(CELCIUS_MSG);
            }
        }
		break;
	case 6:
		state = host::getHostState();
		if(!heating && ((state == host::HOST_STATE_BUILDING) || (state == host::HOST_STATE_BUILDING_FROM_SD)))
		{

			uint8_t buildPercentage = command::getBuildPercentage();

			if(buildPercentage < 100)
			{
				if ( command::getPauseAtZPos() != 0 ) {
					lcd.setCursor(16,0);
					lcd.write('*');
				}
				lcd.setCursor(17,0);
				lcd.writeInt(buildPercentage,2);
			}
			else if(buildPercentage == 100)
			{
				if ( command::getPauseAtZPos() != 0 ) {
					lcd.setCursor(15,0);
					lcd.write('*');
				}
				lcd.setCursor(16,0);
				lcd.writeFromPgmspace(DONE_MSG);
			}
		}
		break;
	}

	updatePhase++;
	if (updatePhase > 6) {
		updatePhase = 0;
	}

#ifdef DEBUG_ONSCREEN
	lcd.setCursor(0, 0);
	lcd.writeString((char *)"DOS1: ");
	lcd.writeFloat(debug_onscreen1, 3, 0);
	lcd.writeString((char *)" ");

	lcd.setCursor(0, 1);
	lcd.writeString((char *)"DOS2: ");
	lcd.writeFloat(debug_onscreen2, 3, 0);
	lcd.writeString((char *)" ");
#endif
}

void MonitorMode::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::CENTER:
        case ButtonArray::LEFT:
            switch(host::getHostState()) {
            case host::HOST_STATE_BUILDING:
            case host::HOST_STATE_BUILDING_FROM_SD:
				interface::pushScreen(&active_build_menu);
				break;
            case host::HOST_STATE_BUILDING_ONBOARD:
                interface::pushScreen(&cancelBuildMenu);
                break;
            default:
                interface::popScreen();
                break;
            }
        default:
                break;
	}
}


void Menu::update(LiquidCrystalSerial& lcd, bool forceRedraw) {

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
		//Wrap around to bottom of menu
		else {
			itemIndex = itemCount - 1;
		}
		break;
        case ButtonArray::DOWN:
		// decrement index
		if (itemIndex < itemCount - 1) {
			itemIndex++;
		}
		//Wrap around to top of menu
		else {
			itemIndex = firstItemIndex;
		}
		break;
        default:
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
                } else itemIndex = itemCount - 1;
		}
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
                } else itemIndex = 0;
		}
            break;
        default:
                break;
	}
}

PreheatSettingsMenu::PreheatSettingsMenu(uint8_t optionsMask) :
	CounterMenu(optionsMask, (uint8_t)4) {
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
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(PREHEAT_SET_MSG);
            break;
        case 1:
            if(!singleTool){
                lcd.writeFromPgmspace(RIGHT_SPACES_MSG);
                if(selectIndex == 1){
                    lcd.setCursor(14,1);
                    lcd.writeFromPgmspace(ARROW_MSG);
                }
                lcd.setCursor(17,1);
                lcd.writeInt(counterRight,3);
            }
            break;
        case 2:
            if(singleTool){
                lcd.writeFromPgmspace(RIGHT_SPACES_MSG);
                lcd.setCursor(17,2);
                lcd.writeInt(counterRight,3);
            }else{
                lcd.writeFromPgmspace(LEFT_SPACES_MSG);
                lcd.setCursor(17,2);
                lcd.writeInt(counterLeft,3);
            }
            if(selectIndex == 2){
                lcd.setCursor(14,2);
                lcd.writeFromPgmspace(ARROW_MSG);
            }
            break;
         case 3:
            lcd.writeFromPgmspace(PLATFORM_SPACES_MSG);
            if(selectIndex == 3){
                lcd.setCursor(14,3);
                lcd.writeFromPgmspace(ARROW_MSG);
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


ResetSettingsMenu::ResetSettingsMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4) {
	reset();
}

void ResetSettingsMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ResetSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
    
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(RESET1_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(RESET2_MSG);
            break;
        case 2:
            lcd.writeFromPgmspace(NO_MSG);
            break;
        case 3:
            lcd.writeFromPgmspace(YES_MSG);
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

void writeProfileToEeprom(uint8_t pIndex, uint8_t *pName, uint32_t *homeOffsets,
                          uint16_t hbpTemp, uint16_t rightTemp, uint16_t leftTemp) {
        uint16_t offset = eeprom_offsets::PROFILES_BASE + (uint16_t)(pIndex * PROFILE_SIZE);

        cli();

        //Write profile name
        if ( pName )    eeprom_write_block(pName,(uint8_t*)(offset + profile_offsets::PROFILE_NAME), PROFILE_NAME_SIZE);

        //Write home axis
        eeprom_write_block((void *)homeOffsets,(void *)(offset + profile_offsets::PROFILE_HOME_POSITIONS_STEPS), PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));

        //Write temps
        eeprom_write_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_RIGHT_TEMP), rightTemp);
        eeprom_write_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_LEFT_TEMP), leftTemp);
        eeprom_write_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_PLATFORM_TEMP), hbpTemp);

        sei();
}

void readProfileFromEeprom(uint8_t pIndex, uint8_t *pName, uint32_t *homeOffsets,
			   uint16_t *hbpTemp, uint16_t *rightTemp, uint16_t *leftTemp) {
        uint16_t offset = eeprom_offsets::PROFILES_BASE + (uint16_t)(pIndex * PROFILE_SIZE);

        cli();

        //Read profile name
        if ( pName )    eeprom_read_block(pName,(uint8_t*)offset, PROFILE_NAME_SIZE);

        //Read home axis
        eeprom_read_block((void *)homeOffsets,(void *)(offset + profile_offsets::PROFILE_HOME_POSITIONS_STEPS), PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));

        //Read temps
        *rightTemp      = eeprom_read_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_RIGHT_TEMP));
        *leftTemp       = eeprom_read_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_LEFT_TEMP));
        *hbpTemp        = eeprom_read_word((uint16_t*)(offset + profile_offsets::PROFILE_PREHEAT_PLATFORM_TEMP));

        sei();
}

//buf should have length PROFILE_NAME_SIZE + 1
void getProfileName(uint8_t pIndex, uint8_t *buf) {
        uint16_t offset = eeprom_offsets::PROFILES_BASE + (uint16_t)(pIndex * PROFILE_SIZE);

        cli();
        eeprom_read_block(buf,(void *)offset,PROFILE_NAME_SIZE);
        sei();

	//Fill out the name with white space
	bool addWhiteSpace = false;
	for (uint8_t i = 0; i < PROFILE_NAME_SIZE; i ++ ) {
		if ( buf[i] == '\0' ) addWhiteSpace = true;
		if ( addWhiteSpace )	buf[i] = ' ';
	}
        buf[PROFILE_NAME_SIZE] = '\0';
}

#define PROFILE_NAME_CHAR_LOWER_LIMIT 32
#define PROFILE_NAME_CHAR_UPPER_LIMIT 126

ProfilesMenu::ProfilesMenu(uint8_t optionsMask) :
	Menu((uint8_t)0, (uint8_t)PROFILES_QUANTITY),
	profileSubMenu((uint8_t)0) {
        reset();

        //Setup defaults if required
       //Initialize a flag to tell us profiles have been initialized
        if ( eeprom_read_byte((uint8_t*)eeprom_offsets::PROFILES_INIT) != PROFILES_INITIALIZED )
		eeprom::setDefaultsProfiles(eeprom_offsets::PROFILES_BASE);
}

void ProfilesMenu::resetState() {
        firstItemIndex = 0;
        itemIndex = 0;
}

void ProfilesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
        uint8_t buf[PROFILE_NAME_SIZE + 1];

        getProfileName(index, buf);

        lcd.writeString((char *)buf);
}

void ProfilesMenu::handleSelect(uint8_t index) {
        profileSubMenu.profileIndex = index;
        interface::pushScreen(&profileSubMenu);
}

ProfileSubMenu::ProfileSubMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)4),
	profileChangeNameMode((uint8_t)0),
	profileDisplaySettingsMenu((uint8_t)0) {
        reset();
}

void ProfileSubMenu::resetState() {
        itemIndex = 0;
        firstItemIndex = 0;
}

void ProfileSubMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
        switch (index) {
        case 0:
                lcd.writeFromPgmspace(PROFILE_RESTORE_MSG);
                break;
        case 1:
                lcd.writeFromPgmspace(PROFILE_DISPLAY_CONFIG_MSG);
                break;
        case 2:
                lcd.writeFromPgmspace(PROFILE_CHANGE_NAME_MSG);
                break;
        case 3:
                lcd.writeFromPgmspace(PROFILE_SAVE_TO_PROFILE_MSG);
                break;
        }
}

void ProfileSubMenu::handleSelect(uint8_t index) {
        uint16_t hbpTemp, rightTemp, leftTemp;

        switch (index) {
                case 0:
                        //Restore
                        //Read settings from eeprom
                        readProfileFromEeprom(profileIndex, NULL, homePosition, &hbpTemp, &rightTemp, &leftTemp);

                        //Write out the home offsets
			cli();
			eeprom_write_block(homePosition, (void*)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, sizeof(uint32_t) * PROFILES_HOME_POSITIONS_STORED);
                        eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET),    rightTemp);
                        eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET),     leftTemp);
                        eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET), hbpTemp);
                        sei();

                        interface::popScreen();
                        interface::popScreen();

                        //Reset
                        host::stopBuildNow();
                        break;
                case 1:
                        //Display settings
                        profileDisplaySettingsMenu.profileIndex = profileIndex;
                        interface::pushScreen(&profileDisplaySettingsMenu);
                        break;
                case 2:
                        //Change Profile Name
                        profileChangeNameMode.profileIndex = profileIndex;
                        interface::pushScreen(&profileChangeNameMode);
                        break;
                case 3: //Save To Profile
                        //Get the home axis positions
                        cli();
        		eeprom_read_block((void *)homePosition,(void *)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));
                        rightTemp = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET));
                        leftTemp  = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET));
                        hbpTemp   = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET));
			sei();

                        writeProfileToEeprom(profileIndex, NULL, homePosition, hbpTemp, rightTemp, leftTemp);

                        interface::popScreen();
                        break;
        }
}

void ProfileChangeNameMode::reset() {
        cursorLocation = 0;
        getProfileName(profileIndex, profileName);
}

void ProfileChangeNameMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
        if (forceRedraw) {
                lcd.clear();

                lcd.setCursor(0,0);
                lcd.writeFromPgmspace(PROFILE_PROFILE_NAME_MSG);

                lcd.setCursor(0,3);
                lcd.writeFromPgmspace(UPDNLRM_MSG);
        }

        lcd.setCursor(0,1);
        lcd.writeString((char *)profileName);

        //Draw the cursor
        lcd.setCursor(cursorLocation,2);
        lcd.write('^');

        //Write a blank before and after the cursor if we're not at the ends
        if ( cursorLocation >= 1 ) {
                lcd.setCursor(cursorLocation-1, 2);
                lcd.writeFromPgmspace(BLANK_CHAR_MSG);
        }
        if ( cursorLocation < PROFILE_NAME_SIZE ) {
                lcd.setCursor(cursorLocation+1, 2);
                lcd.writeFromPgmspace(BLANK_CHAR_MSG);
        }
}

void ProfileChangeNameMode::notifyButtonPressed(ButtonArray::ButtonName button) {
        uint16_t offset;

        switch (button) {
                case ButtonArray::CENTER:
                        //Write the profile name
                        offset = eeprom_offsets::PROFILES_BASE + (uint16_t)(profileIndex * PROFILE_SIZE);

                        cli();
                        eeprom_write_block(profileName,(uint8_t*)offset, PROFILE_NAME_SIZE);
                        sei();

                        interface::popScreen();
                        break;
                case ButtonArray::LEFT:
                        if ( cursorLocation > 0 )        cursorLocation --;
			else				 interface::popScreen();
                        break;
                case ButtonArray::RIGHT:
                        if ( cursorLocation < (PROFILE_NAME_SIZE-1) ) cursorLocation ++;
                        break;
                case ButtonArray::UP:
                        profileName[cursorLocation] += 1;
			break;
                case ButtonArray::DOWN:
                        profileName[cursorLocation] -= 1;
                        break;
	        default:
			break;
        }

        //Hard limits
        if ( profileName[cursorLocation] < PROFILE_NAME_CHAR_LOWER_LIMIT )      profileName[cursorLocation] = PROFILE_NAME_CHAR_LOWER_LIMIT;
        if ( profileName[cursorLocation] > PROFILE_NAME_CHAR_UPPER_LIMIT )      profileName[cursorLocation] = PROFILE_NAME_CHAR_UPPER_LIMIT;
}

ProfileDisplaySettingsMenu::ProfileDisplaySettingsMenu(uint8_t optionsMask):
	Menu(optionsMask, (uint8_t)8) {
        reset();
}

void ProfileDisplaySettingsMenu::resetState() {
        readProfileFromEeprom(profileIndex, profileName, home, &hbpTemp, &rightTemp, &leftTemp);
        itemIndex = 2;
        firstItemIndex = 2;
}

void ProfileDisplaySettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
        switch (index) {
        case 0:
                lcd.writeString((char *)profileName);
                break;
        case 2:
                lcd.writeFromPgmspace(XOFFSET_MSG);
                lcd.writeFloat(stepperAxisStepsToMM(home[X_AXIS], X_AXIS), 3, LCD_SCREEN_WIDTH - 2);
		lcd.writeFromPgmspace(MILLIMETERS_MSG);
                break;
        case 3:
                lcd.writeFromPgmspace(YOFFSET_MSG);
                lcd.writeFloat(stepperAxisStepsToMM(home[Y_AXIS], Y_AXIS), 3, LCD_SCREEN_WIDTH - 2);
		lcd.writeFromPgmspace(MILLIMETERS_MSG);
                break;
        case 4:
                lcd.writeFromPgmspace(ZOFFSET_MSG);
                lcd.writeFloat(stepperAxisStepsToMM(home[Z_AXIS], Z_AXIS), 3, LCD_SCREEN_WIDTH - 2);
		lcd.writeFromPgmspace(MILLIMETERS_MSG);
                break;
        case 5:
                lcd.writeFromPgmspace(PROFILE_RIGHT_MSG);
                lcd.writeFloat((float)rightTemp, 0, LCD_SCREEN_WIDTH);
                break;
        case 6:
                lcd.writeFromPgmspace(PROFILE_LEFT_MSG);
                lcd.writeFloat((float)leftTemp, 0, LCD_SCREEN_WIDTH);
                break;
        case 7:
                lcd.writeFromPgmspace(PROFILE_PLATFORM_MSG);
                lcd.writeFloat((float)hbpTemp, 0, LCD_SCREEN_WIDTH);
                break;
        }
}

void ProfileDisplaySettingsMenu::handleSelect(uint8_t index) {
}

#ifdef EEPROM_MENU_ENABLE

EepromMenu::EepromMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)3) {
	reset();
}

void EepromMenu::resetState() {
        itemIndex = 0;
        firstItemIndex = 0;
	safetyGuard = 0;
	itemSelected = -1;
	warningScreen = true;
}

void EepromMenu::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if ( warningScreen ) {
		if ( forceRedraw ) {
			const static PROGMEM prog_uchar msg1[]		= "This menu can make";
			const static PROGMEM prog_uchar msg2[]		= "your bot inoperable.";
			const static PROGMEM prog_uchar msg4[]		= "Press UP to cont";

			lcd.setCursor(0,0);
			lcd.writeFromPgmspace(msg1);

			lcd.setCursor(0,1);
			lcd.writeFromPgmspace(msg2);

			lcd.setCursor(0,3);
			lcd.writeFromPgmspace(msg4);
		}
	}
	else {
		if ( itemSelected != -1 ) {
			lcd.clear();
			lcd.setCursor(0,0);
		}

		const static PROGMEM prog_uchar message_dump[]		= "Saving...";
		const static PROGMEM prog_uchar message_restore[]	= "Restoring...";
		const static PROGMEM prog_uchar message_erase[]		= "Erasing...";

		const char dumpFilename[] = "eeprom_dump.bin";

		const static PROGMEM prog_uchar message_error[]		= "Error:";

		switch ( itemSelected ) {
			case 0:	//Dump
				if ( ! sdcard::fileExists(dumpFilename) ) {
					lcd.writeFromPgmspace(message_dump);
					if ( ! eeprom::saveToSDFile(dumpFilename) ) {
						const static PROGMEM prog_uchar msg1[]	= "Write Failed!";
						lcd.clear();
						lcd.setCursor(0,0);
						lcd.writeFromPgmspace(msg1);
						_delay_us(5000000);
					}
				} else {
					const static PROGMEM prog_uchar msg3[]	= "File exists!";
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.writeFromPgmspace(message_error);
					lcd.setCursor(0,1);
					lcd.writeString((char *)dumpFilename);
					lcd.setCursor(0,2);
					lcd.writeFromPgmspace(msg3);
					_delay_us(5000000);
				}
				interface::popScreen();
				break;

			case 1: //Restore
				if ( sdcard::fileExists(dumpFilename) ) {
					lcd.writeFromPgmspace(message_restore);
					if ( ! eeprom::restoreFromSDFile(dumpFilename) ) {
						const static PROGMEM prog_uchar msg1[]	= "Read Failed!";
						const static PROGMEM prog_uchar msg2[]	= "Eeprom maybe";
						const static PROGMEM prog_uchar msg3[]	= "corrupt";
						lcd.clear();
						lcd.setCursor(0,0);
						lcd.writeFromPgmspace(msg1);
						lcd.setCursor(0,1);
						lcd.writeFromPgmspace(msg2);
						lcd.setCursor(0,2);
						lcd.writeFromPgmspace(msg3);
						_delay_us(5000000);
					}
					host::stopBuildNow();
				} else {
					const static PROGMEM prog_uchar msg3[]	= "File not found!";
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.writeFromPgmspace(message_error);
					lcd.setCursor(0,1);
					lcd.writeString((char *)dumpFilename);
					lcd.setCursor(0,2);
					lcd.writeFromPgmspace(msg3);
					_delay_us(5000000);
					interface::popScreen();
				}
				break;

			case 2: //Erase
				lcd.writeFromPgmspace(message_erase);
				_delay_us(5000000);
				eeprom::erase();
				interface::popScreen();
				host::stopBuildNow();
				break;
			default:
				Menu::update(lcd, forceRedraw);
				break;
		}

		lcd.setCursor(0,3);
		if ( safetyGuard >= 1 ) {
			const static PROGMEM prog_uchar msg1[]	= "* Press ";
			const static PROGMEM prog_uchar msg2[]	= "x more!";

			lcd.writeFromPgmspace(msg1);
			lcd.writeInt((uint16_t)(4-safetyGuard),1);
			lcd.writeFromPgmspace(msg2);
		} else {
			const static PROGMEM prog_uchar blank[]	= "                ";
			lcd.writeFromPgmspace(blank);
		}

		itemSelected = -1;
	}
}

void EepromMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
        switch (index) {
        case 0:
                lcd.writeFromPgmspace(EEPROM_DUMP_MSG);
                break;
        case 1:
                lcd.writeFromPgmspace(EEPROM_RESTORE_MSG);
                break;
        case 2:
                lcd.writeFromPgmspace(EEPROM_ERASE_MSG);
                break;
        }
}

void EepromMenu::handleSelect(uint8_t index) {
	switch (index)
	{
		case 0:
			//Dump
			safetyGuard = 0;
			itemSelected = 0;
			break;
		case 1:
			//Restore
			safetyGuard ++;
			if ( safetyGuard > 3 ) {
				safetyGuard = 0;
				itemSelected = 1;
			}
			break;
		case 2:
			//Erase
			safetyGuard ++;
			if ( safetyGuard > 3 ) {
				safetyGuard = 0;
				itemSelected = 2;
			}
			break;
	}
}

void EepromMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
	if ( warningScreen ) {
		switch (button) {
			case ButtonArray::UP:
				warningScreen = false;
				return;
			default:
				Menu::notifyButtonPressed(ButtonArray::LEFT);
				return;
		}

		return;
	}

        if ( button == ButtonArray::DOWN || button == ButtonArray::LEFT ||
	     button == ButtonArray::RIGHT )
		safetyGuard = 0;

	Menu::notifyButtonPressed(button);
}

#endif

void HomeOffsetsMode::reset() {
	cli();
	eeprom_read_block(homePosition, (void *)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));
	sei();

        lastHomeOffsetState = HOS_NONE;
        homeOffsetState     = HOS_OFFSET_X;
	valueChanged = false;
}

void HomeOffsetsMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
        if ( homeOffsetState != lastHomeOffsetState )   forceRedraw = true;

        if (forceRedraw) {
                lcd.clear();

                lcd.setCursor(0,0);
                switch(homeOffsetState) {
                        case HOS_OFFSET_X:
                                lcd.writeFromPgmspace(XOFFSET_MSG);
                                break;
                        case HOS_OFFSET_Y:
                                lcd.writeFromPgmspace(YOFFSET_MSG);
                                break;
                        case HOS_OFFSET_Z:
                                lcd.writeFromPgmspace(ZOFFSET_MSG);
                                break;
			default:
				break;
                }

                lcd.setCursor(0,3);
                lcd.writeFromPgmspace(UPDNLM_MSG);
        }

        float position = 0.0;

        switch(homeOffsetState) {
                case HOS_OFFSET_X:
                        position = stepperAxisStepsToMM(homePosition[0], X_AXIS);
                        break;
                case HOS_OFFSET_Y:
                        position = stepperAxisStepsToMM(homePosition[1], Y_AXIS);
                        break;
                case HOS_OFFSET_Z:
                        position = stepperAxisStepsToMM(homePosition[2], Z_AXIS);
                        break;
	        default:
         	        break;
        }

        lcd.setCursor(0,1);
	lcd.writeFloat((float)position, 3, 0);
        lcd.writeFromPgmspace(MILLIMETERS_MSG);
	lcd.writeFromPgmspace(BLANK_CHAR_MSG);

        lastHomeOffsetState = homeOffsetState;
}

void HomeOffsetsMode::notifyButtonPressed(ButtonArray::ButtonName button) {
        uint8_t currentIndex = homeOffsetState - HOS_OFFSET_X;

        switch (button) {
                case ButtonArray::LEFT:
                        interface::popScreen();
                        break;
                case ButtonArray::CENTER:
			if ( valueChanged ) {
                        	cli();
                        	eeprom_write_block((void *)&homePosition[currentIndex],
						   (void*)(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + sizeof(uint32_t) * currentIndex) ,
						   sizeof(uint32_t));
                        	sei();
			}

                        if      ( homeOffsetState == HOS_OFFSET_X )     homeOffsetState = HOS_OFFSET_Y;
                        else if ( homeOffsetState == HOS_OFFSET_Y )     homeOffsetState = HOS_OFFSET_Z;
			else						interface::popScreen();
			valueChanged = false;
                        break;
                case ButtonArray::UP:
                        // increment less
                        homePosition[currentIndex] += 1;
			valueChanged = true;
                        break;
                case ButtonArray::DOWN:
                        // decrement less
                        homePosition[currentIndex] -= 1;
			valueChanged = true;
                        break;
	        default:
			break;
        }
}

void PauseAtZPosScreen::reset() {
	int32_t currentPause = command::getPauseAtZPos();

	if ( currentPause == 0 ) {
		Point position = steppers::getPlannerPosition();
		pauseAtZPos = stepperAxisStepsToMM(position[2], Z_AXIS);
		if ( pauseAtZPos < 0 )	pauseAtZPos = 0;
	} else	pauseAtZPos = stepperAxisStepsToMM(currentPause, Z_AXIS);
}

void PauseAtZPosScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw) {
		lcd.clear();

		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(PAUSE_AT_ZPOS_MSG);

                lcd.setCursor(0,3);
                lcd.writeFromPgmspace(UPDNLM_MSG);
        }

        lcd.setCursor(0,1);
	lcd.writeFloat(pauseAtZPos, 2, 0);
        lcd.writeFromPgmspace(MILLIMETERS_MSG);
	lcd.writeFromPgmspace(BLANK_CHAR_4_MSG);
}

void PauseAtZPosScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	uint16_t repetitions;

        switch (button) {
                case ButtonArray::CENTER:
			//Set the pause
			command::pauseAtZPos(stepperAxisMMToSteps(pauseAtZPos, Z_AXIS));
                case ButtonArray::LEFT:
                        interface::popScreen();
                        break;
                case ButtonArray::UP:
                        // increment less
			repetitions = Motherboard::getBoard().getInterfaceBoard().getButtonRepetitions();
			if	( repetitions > 18 )	pauseAtZPos += 10.0;
			else if ( repetitions > 12 )	pauseAtZPos += 1.0;
			else if ( repetitions > 6 )	pauseAtZPos += 0.1;
			else				pauseAtZPos += 0.01;
                        break;
                case ButtonArray::DOWN:
                        // decrement less
			repetitions = Motherboard::getBoard().getInterfaceBoard().getButtonRepetitions();
			if	( repetitions > 18 )	pauseAtZPos -= 10.0;
			else if ( repetitions > 12 )	pauseAtZPos -= 1.0;
			else if ( repetitions > 6 )	pauseAtZPos -= 0.1;
			else				pauseAtZPos -= 0.01;
                        break;
	        default:
			break;
        }

	//Range clamping
	if ( pauseAtZPos < 0.0 )	pauseAtZPos = 0.0;

	float maxMM = stepperAxisStepsToMM(stepperAxis[Z_AXIS].max_axis_steps_limit, Z_AXIS) + 1.0;	//+1.0 to allow for rounding as
													//steps per mm stored in eeprom isn't as high
													//resolution as the xml in RepG
	if ( pauseAtZPos > maxMM)	pauseAtZPos = maxMM;
}

ActiveBuildMenu::ActiveBuildMenu(uint8_t optionsMask) :
	Menu(optionsMask, (uint8_t)0),
	build_stats_screen((uint8_t)0),
	pauseAtZPosScreen((uint8_t)0) {
	reset();
}
    
void ActiveBuildMenu::resetState(){
	itemIndex = 0;
	firstItemIndex = 0;
	is_paused = command::isPaused();
	if ( is_paused )	itemCount = 6;
	else			itemCount = 5;

	//If any of the heaters are on, we provide another
	//menu options, "Heaters Off"
	if (( is_paused ) && 
	    ( Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0 || 
	      Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0 ||
	      Motherboard::getBoard().getPlatformHeater().get_set_temperature() > 0 )) {
		itemCount ++;
	}
}

void ActiveBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd){
	
	switch (index) {
        case 0:
            lcd.writeFromPgmspace(BACK_TO_MONITOR_MSG);
            break;
        case 1:
			lcd.writeFromPgmspace(STATS_MSG);
            break;
        case 2:
	    resetState();	//Required to update the pause state if we were previously in
				//another menu on top of this one

            if(is_paused){
				lcd.writeFromPgmspace(UNPAUSE_MSG);
		    }else {
				lcd.writeFromPgmspace(PAUSE_MSG);
		    }
            break;
        case 3:
            lcd.writeFromPgmspace(CANCEL_BUILD_MSG);
            break;
	case 4:
	    lcd.writeFromPgmspace(PAUSEATZPOS_MSG);
	    break;
	case 5:
	    lcd.writeFromPgmspace(FILAMENT_OPTIONS_MSG);
	    break;
	case 6:
	    lcd.writeFromPgmspace(HEATERS_OFF_MSG);
	    break;
	}
}

void ActiveBuildMenu::handleSelect(uint8_t index){
	
	switch (index) {
		case 0:
			interface::popScreen();
			break;
        case 1:
			interface::pushScreen(&build_stats_screen);
            break;
        case 2:
			// pause command execution
			is_paused = !is_paused;
			host::pauseBuild(is_paused);
			if(is_paused){
				for (int i = 3; i < STEPPER_COUNT; i++) 
					steppers::enableAxis(i, false);	
			}else{
				interface::popScreen();
			}
			lineUpdate = true;
            break;
        case 3:
            // Cancel build
			interface::pushScreen(&Motherboard::getBoard().mainMenu.utils.monitorMode.cancelBuildMenu);
            break;
		case 4:
			//Handle Pause At ZPos
			interface::pushScreen(&pauseAtZPosScreen);
			break;
		case 5:
			//Handle filament
			interface::pushScreen(&Motherboard::getBoard().mainMenu.utils.filament);
			break;
		case 6:
			//Switch all the heaters off
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
                	Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
                	Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
			reset();
			break;
	}
}

void BuildStats::update(LiquidCrystalSerial& lcd, bool forceRedraw){
	
	if (forceRedraw) {
		lcd.clear();

		lcd.setCursor(0,0);
		lcd.writeFromPgmspace(BUILD_TIME_MSG);

		lcd.setCursor(0,1);
		lcd.writeFromPgmspace(Z_POSITION_MSG);

		lcd.setCursor(0,2);
		lcd.writeFromPgmspace(FILAMENT_MSG);

		lcd.setCursor(0,3);
		lcd.writeFromPgmspace(LINE_NUMBER_MSG);
	}
	 
	Point position;
	float filamentUsed, lastFilamentUsed;

	switch (update_count){
		
		case 0:
			uint8_t build_hours;
			uint8_t build_minutes;
			host::getPrintTime(build_hours, build_minutes);
			
			lcd.setCursor(14,0);
			lcd.writeInt(build_hours,2);
				
			lcd.setCursor(17,0);
			lcd.writeInt(build_minutes,2);
			
			break;
		case 1:
			uint32_t line_number;
			line_number = command::getLineNumber();
			/// if line number greater than counted, print an indicator that we are over count
			if(line_number > command::MAX_LINE_COUNT){
				lcd.setCursor(9,3);
				//Replaced with this message, because writeInt / writeInt32 can't display it anyway.
				//and 1,000,000,000 lines would represent 115 days of printing at 100 moves per second
				lcd.writeFromPgmspace(PRINTED_TOO_LONG_MSG);
				lcd.setCursor(19,3);
				lcd.writeString((char *)"+");
			}else{
				
				uint8_t digits = 1;
				for (uint32_t i = 10; i < command::MAX_LINE_COUNT; i*=10){
					if(line_number / i == 0){ break; }
					digits ++;
				}			
				lcd.setCursor(20-digits,3);
				lcd.writeInt(line_number, digits);
			}
			break;
	
		case 2:
			position = steppers::getPlannerPosition();
			lcd.setCursor(10, 1);
			lcd.writeFromPgmspace(BLANK_CHAR_4_MSG);
			lcd.writeFloat(stepperAxisStepsToMM(position[Z_AXIS], Z_AXIS), 3, LCD_SCREEN_WIDTH - 2);
			lcd.writeFromPgmspace(MILLIMETERS_MSG);
			break;	

		case 3:
			lastFilamentUsed = stepperAxisStepsToMM(command::getLastFilamentLength(0), A_AXIS) + stepperAxisStepsToMM(command::getLastFilamentLength(1), B_AXIS);
			if ( lastFilamentUsed != 0.0 )	filamentUsed = lastFilamentUsed;
                        else				filamentUsed = stepperAxisStepsToMM(command::getFilamentLength(0), A_AXIS) + 
								       stepperAxisStepsToMM(command::getFilamentLength(1), B_AXIS); 
			printFilamentUsed(filamentUsed, 2, lcd);
			break;
		default:
			break;
	}
	update_count++;
	/// make the update_count max higher than actual updateable fields because
	/// we don't need to update these stats every half second
	if (update_count > UPDATE_COUNT_MAX){
		update_count = 0;
	}
}

void BuildStats::reset(){

	update_count = 0;
}

void BuildStats::notifyButtonPressed(ButtonArray::ButtonName button){
	
	switch (button) {
		case ButtonArray::CENTER:
			interface::popScreen();
			break;
        case ButtonArray::LEFT:
			interface::popScreen();
			break;
        case ButtonArray::RIGHT:
        case ButtonArray::DOWN:
        case ButtonArray::UP:
			break;
        default:
                break;
	}
}
	
CancelBuildMenu::CancelBuildMenu(uint8_t optionsMask) :
	Menu(optionsMask | IS_CANCEL_SCREEN_MASK, (uint8_t)4) {
	reset();
}

void CancelBuildMenu::resetState() {

	itemIndex = 2;
	firstItemIndex = 2;
	
}

void CancelBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

    host::HostState state = host::getHostState();
   
    
    switch (index) {
	case 0:
		if((state == host::HOST_STATE_BUILDING) ||
            (state == host::HOST_STATE_BUILDING_FROM_SD) ||
            (state == host::HOST_STATE_BUILDING_ONBOARD))
            lcd.writeFromPgmspace(CANCEL_MSG);
        else{
			// host::pauseBuild(true);
            		//lcd.writeFromPgmspace(CANCEL_PROCESS_MSG);

            		lcd.writeFromPgmspace(CANCEL_MSG);
		}
		break;
    case 2:
		lcd.writeFromPgmspace(NO_MSG);
        break;
    case 3:
        lcd.writeFromPgmspace(YES_MSG);
        break;
	}
}

void CancelBuildMenu::handleSelect(uint8_t index) {
    
    //host::HostState state = host::getHostState();
    
	switch (index) {
        case 2:
			interface::popScreen();
            break;
        case 3:
            // Cancel build
	    command::addFilamentUsed();
            host::stopBuild();
            break;
	}
}


MainMenu::MainMenu(uint8_t optionsMask) :
	Menu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)4),
	     utils((uint8_t)0),
	     sdMenu((uint8_t)0),
	     preheat((uint8_t)0) {
	reset();
}
void MainMenu::resetState() {
	itemIndex = 1;
	firstItemIndex = 1;
}

void MainMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	char * name;
	
	switch (index) {
	case 0:
		name = host::getMachineName();
		lcd.setCursor((20 - strlen(name))/2,0);
		lcd.writeString(host::getMachineName());
		break;
	case 1:
		lcd.writeFromPgmspace(BUILD_MSG);
		break;
	case 2:
		lcd.writeFromPgmspace(PREHEAT_MSG);
		break;
	case 3:
		lcd.writeFromPgmspace(UTILITIES_MSG);
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


UtilitiesMenu::UtilitiesMenu(uint8_t optionsMask) :
	     Menu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN),
		  (uint8_t)16
#ifdef EEPROM_MENU_ENABLE
		  +1
#endif
		     ),
	     monitorMode((uint8_t)0),
	     welcome((uint8_t)0),
	     splash((uint8_t)0),
	     filament((uint8_t)0),
	     jogger((uint8_t)0),
	     set((uint8_t)0),
	     preheat((uint8_t)0),
	     profilesMenu((uint8_t)0),
	     homeOffsetsMode((uint8_t)0),
	     reset_settings((uint8_t)0),
	     alignment((uint8_t)0),
	     filamentOdometer((uint8_t)0)
#ifdef EEPROM_MENU_ENABLE
	     , eepromMenu((uint8_t)0)
#endif
 		{
	blinkLED = false;
	reset();
}
void UtilitiesMenu::resetState(){
	singleTool = eeprom::isSingleTool();
	stepperEnable = ( axesEnabled ) ? false : true;
}

void UtilitiesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(MONITOR_MSG);
		break;
	case 1:
		lcd.writeFromPgmspace(FILAMENT_OPTIONS_MSG);
		break;
	case 2:
		lcd.writeFromPgmspace(PREHEAT_SETTINGS_MSG);
		break;
	case 3:
		lcd.writeFromPgmspace(PLATE_LEVEL_MSG);
		break;
	case 4:
		lcd.writeFromPgmspace(HOME_AXES_MSG);
		break;	
	case 5:
		lcd.writeFromPgmspace(JOG_MSG);
		break;
	case 6:
		lcd.writeFromPgmspace(PROFILES_MSG);
		break;
	case 7:
		lcd.writeFromPgmspace(FILAMENT_ODOMETER_MSG);
		break;
	case 8:
		if(stepperEnable)
			lcd.writeFromPgmspace(ESTEPS_MSG);
		else
			lcd.writeFromPgmspace(DSTEPS_MSG);
		break;
	case 9:
		if(blinkLED)
			lcd.writeFromPgmspace(LED_STOP_MSG);
		else
			lcd.writeFromPgmspace(LED_BLINK_MSG);
		break;
	case 10:
                lcd.writeFromPgmspace(HOME_OFFSETS_MSG);
		break;
	case 11:
		lcd.writeFromPgmspace(STARTUP_MSG);
		break;
	case 12:
		lcd.writeFromPgmspace(VERSION_MSG);
		break;
	case 13:
		lcd.writeFromPgmspace(SETTINGS_MSG);
		break;
	case 14:
		singleTool = eeprom::isSingleTool();
		if(singleTool)
			lcd.writeFromPgmspace(RESET_MSG);
		else
			lcd.writeFromPgmspace(NOZZLES_MSG);
		break;	
	case 15:
		if(!singleTool)
			lcd.writeFromPgmspace(RESET_MSG);
		break;
#ifdef EEPROM_MENU_ENABLE
	case 16:
		lcd.writeFromPgmspace(EEPROM_MSG);
		break;
#endif
	}
}

void UtilitiesMenu::handleSelect(uint8_t index) {
		
	switch (index) {
		case 0:
			// Show monitor build screen
                        interface::pushScreen(&monitorMode);
			break;
		case 1:
			// load filament script
                   interface::pushScreen(&filament);
			break;
		case 2:
			interface::pushScreen(&preheat);
			break;
		case 3:
			// level_plate script
                    host::startOnboardBuild(utility::LEVEL_PLATE_STARTUP);
			break;
		case 4:
			// home axes script
                    host::startOnboardBuild(utility::HOME_AXES);
			break;
		case 5:
			// Show build from SD screen
                       interface::pushScreen(&jogger);
			break;
		case 6:
			// Profiles
			interface::pushScreen(&profilesMenu);
			break;
		case 7:
			// Filament Odometer
			interface::pushScreen(&filamentOdometer);
			break;
		case 8:
			for (int i = 0; i < STEPPER_COUNT; i++) 
					steppers::enableAxis(i, stepperEnable);
			lineUpdate = true;
			stepperEnable = !stepperEnable;
			break;
		case 9:
			blinkLED = !blinkLED;
			if(blinkLED)
				RGB_LED::setLEDBlink(150);
			else
				RGB_LED::setLEDBlink(0);
			lineUpdate = true;		 
			 break;
		case 10:
			// Home Offsets
			interface::pushScreen(&homeOffsetsMode);
			break;
		case 11:
			// startup wizard script
            interface::pushScreen(&welcome);
			break;
		case 12:
			splash.SetHold(true);
			interface::pushScreen(&splash);
			break;
		case 13:
			// settings menu
            interface::pushScreen(&set);
			break;
		case 14:
			if(singleTool)
				// restore defaults
				interface::pushScreen(&reset_settings);
			else
				interface::pushScreen(&alignment);
			break;
		case 15:
			if(!singleTool)
				// restore defaults
				interface::pushScreen(&reset_settings);
			break;
#ifdef EEPROM_MENU_ENABLE
		case 16:
			//Eeprom Menu
			interface::pushScreen(&eepromMenu);
			break;
#endif
		}
}

SettingsMenu::SettingsMenu(uint8_t optionsMask) :
	CounterMenu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)8
#ifdef DITTO_PRINT
		+1
#endif
		) {
    reset();
}

void SettingsMenu::resetState(){
	singleExtruder = eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1);
    soundOn = eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS, 1);
    LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
    heatingLEDOn = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
    helpOn = eeprom::getEeprom8(eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
    accelerationOn = eeprom::getEeprom8(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::ACTIVE_OFFSET, 0x01);
    overrideGcodeTempOn = eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0);
    pauseHeatOn = eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, 1);

#ifdef DITTO_PRINT
    dittoPrintOn = eeprom::getEeprom8(eeprom_offsets::DITTO_PRINT_ENABLED, 0);
    if ( singleExtruder == 1 ) dittoPrintOn = false;
#endif
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

    
	switch (index) {
        case 0:
			lcd.writeFromPgmspace(SOUND_MSG);
			 lcd.setCursor(11,0);
			if(selectIndex == 0)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,0);
            if(soundOn)
                lcd.writeFromPgmspace(ON_MSG);
            else
                lcd.writeFromPgmspace(OFF_MSG);
            break;
        case 1:
            lcd.writeFromPgmspace(LED_MSG);
             lcd.setCursor(11,1);
			if(selectIndex == 1)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,1);
            switch(LEDColor){
                case LED_DEFAULT_RED:
                    lcd.writeFromPgmspace(RED_COLOR_MSG);
                    break;
                case LED_DEFAULT_ORANGE:
                    lcd.writeFromPgmspace(ORANGE_COLOR_MSG);
                    break;
                case LED_DEFAULT_PINK:
                    lcd.writeFromPgmspace(PINK_COLOR_MSG);
                    break;
                case LED_DEFAULT_GREEN:
                    lcd.writeFromPgmspace(GREEN_COLOR_MSG);
                    break;
                case LED_DEFAULT_BLUE:
                    lcd.writeFromPgmspace(BLUE_COLOR_MSG);
                    break;
                case LED_DEFAULT_PURPLE:
                    lcd.writeFromPgmspace(PURPLE_COLOR_MSG);
                    break;
                case LED_DEFAULT_WHITE:
                    lcd.writeFromPgmspace(WHITE_COLOR_MSG);
                    break;
                case LED_DEFAULT_CUSTOM:
					lcd.writeFromPgmspace(CUSTOM_COLOR_MSG);
					break;
            }
            break;
        case 2:
			lcd.writeFromPgmspace(TOOL_COUNT_MSG);
			lcd.setCursor(11,2);
			if(selectIndex == 2)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,2);
            if(singleExtruder == 1)
                lcd.writeFromPgmspace(TOOL_SINGLE_MSG);
            else
                lcd.writeFromPgmspace(TOOL_DUAL_MSG);
            break;
         case 3:
			lcd.writeFromPgmspace(LED_HEAT_MSG);
			 lcd.setCursor(11,3);
			if(selectIndex == 3)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,3);
            if(heatingLEDOn)
                lcd.writeFromPgmspace(ON_MSG);
            else
                lcd.writeFromPgmspace(OFF_MSG);
            break;
          case 4:
			lcd.writeFromPgmspace(HELP_SCREENS_MSG);
			 lcd.setCursor(11,0);
			if(selectIndex == 4)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,0);
            if(helpOn)
                lcd.writeFromPgmspace(ON_MSG);
            else
                lcd.writeFromPgmspace(OFF_MSG);
            break;
          case 5:
			lcd.writeFromPgmspace(ACCELERATE_MSG);
			 lcd.setCursor(11,1);
			if(selectIndex == 5)
                lcd.writeFromPgmspace(ARROW_MSG);
            else
				lcd.writeFromPgmspace(NO_ARROW_MSG);
            lcd.setCursor(14,1);
            if(accelerationOn)
                lcd.writeFromPgmspace(ON_MSG);
            else
                lcd.writeFromPgmspace(OFF_MSG);
            break;
	 case 6:
	    lcd.writeFromPgmspace(OVERRIDE_GCODE_TEMP_MSG);
	    lcd.setCursor(11,2);
	    if(selectIndex == 6)
		lcd.writeFromPgmspace(ARROW_MSG);
	    else
		lcd.writeFromPgmspace(NO_ARROW_MSG);
	    lcd.setCursor(14,2);
	    if(overrideGcodeTempOn)
		lcd.writeFromPgmspace(ON_MSG);
	    else
	    lcd.writeFromPgmspace(OFF_MSG);
	    break;
	 case 7:
	    lcd.writeFromPgmspace(PAUSE_HEAT_MSG);
	    lcd.setCursor(11,3);
	    if(selectIndex == 7)
		lcd.writeFromPgmspace(ARROW_MSG);
	    else
		lcd.writeFromPgmspace(NO_ARROW_MSG);
	    lcd.setCursor(14,3);
	    if(pauseHeatOn)
		lcd.writeFromPgmspace(ON_MSG);
	    else
	    lcd.writeFromPgmspace(OFF_MSG);
	    break;
#ifdef DITTO_PRINT
	 case 8:
	    lcd.writeFromPgmspace(DITTO_PRINT_MSG);
	    lcd.setCursor(11,0);
	    if(selectIndex == 8)
		lcd.writeFromPgmspace(ARROW_MSG);
	    else
		lcd.writeFromPgmspace(NO_ARROW_MSG);
	    lcd.setCursor(14,0);
	    if(dittoPrintOn)
		lcd.writeFromPgmspace(ON_MSG);
	    else
	    lcd.writeFromPgmspace(OFF_MSG);
	    break;
#endif
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
		case 5:
            // update right counter
            if(up)
                accelerationOn++;
            else
                accelerationOn--;
            // keep within appropriate boundaries    
            if(accelerationOn > 1)
                accelerationOn = 0;
            else if(accelerationOn < 0)
				accelerationOn = 1;
			break;
	 case 6:
            // update right counter
            if(up)
                overrideGcodeTempOn++;
            else
                overrideGcodeTempOn--;
            // keep within appropriate boundaries    
            if(overrideGcodeTempOn > 1)
                overrideGcodeTempOn = 0;
            else if(overrideGcodeTempOn < 0)
		overrideGcodeTempOn = 1;
	    break;
	 case 7:
            // update right counter
            if(up)
                pauseHeatOn++;
            else
                pauseHeatOn--;
            // keep within appropriate boundaries    
            if(pauseHeatOn > 1)
                pauseHeatOn = 0;
            else if(pauseHeatOn < 0)
		pauseHeatOn = 1;
	    break;
#ifdef DITTO_PRINT
	 case 8:
	    if ( singleExtruder == 1 )	break;
            // update right counter
            if(up)
                dittoPrintOn++;
            else
                dittoPrintOn--;
            // keep within appropriate boundaries    
            if(dittoPrintOn > 1)
                dittoPrintOn = 0;
            else if(dittoPrintOn < 0)
		dittoPrintOn = 1;
	    break;
#endif
	}
    
}


void SettingsMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// update sound preferences
			eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS, soundOn);
			Piezo::reset();
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
		case 5:
			eeprom_write_byte((uint8_t*)eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::ACTIVE_OFFSET, accelerationOn);
			steppers::reset();
			lineUpdate = 1;
			break;
		case 6:
			eeprom_write_byte((uint8_t*)eeprom_offsets::OVERRIDE_GCODE_TEMP, overrideGcodeTempOn);
			lineUpdate = 1;
			break;
		case 7:
			eeprom_write_byte((uint8_t*)eeprom_offsets::HEAT_DURING_PAUSE, pauseHeatOn);
			lineUpdate = 1;
			break;
#ifdef DITTO_PRINT
		case 8:
			eeprom_write_byte((uint8_t*)eeprom_offsets::DITTO_PRINT_ENABLED, dittoPrintOn);
			command::reset();
			lineUpdate = 1;
#endif
    }
}

SDMenu::SDMenu(uint8_t optionsMask) :
	Menu(optionsMask  | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)0) {
	reset();
	updatePhase = 0;
	updatePhaseDivisor = 0;
	drawItemLockout = false;
}

void SDMenu::resetState() {
	cardNotFound = false;
	cardBadFormat = false;
	cardReadError = false;
	itemCount = countFiles() + 1;
	updatePhase = 0;	
	updatePhaseDivisor = 0;
	lastItemIndex = 0;
	drawItemLockout = false;
}

//Returns true if the file is an s3g file
//Keeping this in C instead of C++ saves 20 bytes

bool isS3GFile(char *filename, uint8_t len) {
	if ((len >= 4) && 
	    (filename[len-4]== '.') && (filename[len-3]== 's') &&
	    (filename[len-2]== '3') && (filename[len-1]== 'g')) return true;
	return false;
}

// Count the number of files on the SD card
uint8_t SDMenu::countFiles() {
	uint8_t count = 0;
	uint8_t idx = 0;
	sdcard::SdErrorCode e;	

	// First, reset the directory index
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
		switch(e) {
			case sdcard::SD_ERR_NO_CARD_PRESENT:
				cardNotFound = true;
				break;
			case sdcard::SD_ERR_OPEN_FILESYSTEM:
				cardBadFormat = true;
				break;
			case sdcard::SD_ERR_PARTITION_READ:
			case sdcard::SD_ERR_INIT_FAILED:
			case sdcard:: SD_ERR_NO_ROOT:
				cardReadError = true;
				break;
			case sdcard::SD_ERR_VOLUME_TOO_BIG:
				cardTooBig = true;
				break;
            		default:
                		break;
		}
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

		//Only count it if it ends in .s3g
		if (isS3GFile(fnbuf,idx)) count++;

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
        switch(e) {
			case sdcard::SD_ERR_NO_CARD_PRESENT:
				cardNotFound = true;
				break;
			case sdcard::SD_ERR_OPEN_FILESYSTEM:
				cardBadFormat = true;
				break;
			case sdcard::SD_ERR_PARTITION_READ:
			case sdcard::SD_ERR_INIT_FAILED:
		    case sdcard:: SD_ERR_NO_ROOT:
				cardReadError = true;
				break;
			case sdcard::SD_ERR_VOLUME_TOO_BIG:
				cardTooBig = true;
				break;
	                default:
                 		break;
		}
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
			
		} while ((e == sdcard::SD_SUCCESS) && ( ! isS3GFile(fnbuf,idx)));

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
       
       // print error message if no SD card found;
       if(cardNotFound == true) {
            lcd.writeFromPgmspace(NOCARD_MSG);
			return;
		}else if (cardReadError){
			lcd.writeFromPgmspace(CARDERROR_MSG);
			return;
		}else if (cardBadFormat){
			lcd.writeFromPgmspace(CARDFORMAT_MSG);
			return;
		}else if (cardTooBig){
			lcd.writeFromPgmspace(CARDSIZE_MSG);
			return;
		}
		// print last line for SD card - an exit option
       if (index >= itemCount - 1) {
               lcd.writeFromPgmspace(EXIT_MSG);
			return;
		}

	const uint8_t maxFileLength = host::MAX_FILE_LEN;
	char fnbuf[maxFileLength];

    if ( !getFilename(index, fnbuf, maxFileLength)) {
        interface::popScreen();
        Motherboard::getBoard().errorResponse(CARDERROR2_MSG);
        return;
	}


        //Figure out length of filename
        uint8_t filenameLength;
        for (filenameLength = 0; (filenameLength < maxFileLength) && (fnbuf[filenameLength] != 0); filenameLength++) ;

        uint8_t idx;
        uint8_t longFilenameOffset = 0;
        const uint8_t displayWidth = LCD_SCREEN_WIDTH - 1;

        //Support scrolling filenames that are longer than the lcd screen
        if (filenameLength >= displayWidth) longFilenameOffset = updatePhase % (filenameLength - displayWidth + 1);

        for (idx = 0; (idx < displayWidth) && ((longFilenameOffset + idx) < maxFileLength) &&
			(fnbuf[longFilenameOffset + idx] != 0); idx++)
                lcd.write(fnbuf[longFilenameOffset + idx]);

        //Clear out the rest of the line
        while ( idx < displayWidth ) {
                lcd.write(' ');
                idx ++;
        }

}

void SDMenu::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
        const uint8_t height = LCD_SCREEN_HEIGHT;

        if (( ! forceRedraw ) && ( ! drawItemLockout )) {
                //Redraw the last item if we have changed
                if (((itemIndex/height) == (lastDrawIndex/height)) &&
                     ( itemIndex != lastItemIndex ))  {
                        lcd.setCursor(1,lastItemIndex % height);
                        drawItem(lastItemIndex, lcd);
                }
                lastItemIndex = itemIndex;

                lcd.setCursor(1,itemIndex % height);
                drawItem(itemIndex, lcd);
        }

        Menu::update(lcd, forceRedraw);

	//Continuous Buttons Fires every 0.1seconds, but the default
	//menu update is every 0.5 seconds, which is slow for a continuous press.
	//Menu update rate is changed to 0.1seconds in Menu.hh, we divide
	//by 5 here to give a filename scrolling update rate of 0.5seconds.
	if ( ++updatePhaseDivisor >= 5 ) {
        	updatePhase ++;
		updatePhaseDivisor = 0;
	}
}

void SDMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
        updatePhase = 0;
	updatePhaseDivisor = 0;
        Menu::notifyButtonPressed(button);
}

void SDMenu::handleSelect(uint8_t index) {
	
	if(index >= itemCount -1)
	{
		interface::popScreen();
		return;
	}
	if (host::getHostState() != host::HOST_STATE_READY) {
	        Motherboard::getBoard().errorResponse(BUILDING_MSG);
		return;
	}
		
        drawItemLockout = true;

	const prog_uchar *buildError = NULL;

	char *buildName = host::getBuildName();

	if ( getFilename(index, buildName, host::MAX_FILE_LEN) ) {
		//Files can only be opened if their filename is <=30 characters in length
		//However the filename can be obtained from the directory entry if it's >30 chars
		//Because of this truncation, the file extension will be corrupted, so we
		//check for the file extension again, if it's corrupted, it was too long
		if ( isS3GFile(buildName, strlen(buildName)) ) {
			//Do the build
			sdcard::SdErrorCode e;
			e = host::startBuildFromSD();
	
			if (e != sdcard::SD_SUCCESS) {
				buildError = CARDERROR2_MSG;
			}
		}
		else buildError = FTOOLONG_MSG;
	}
	else buildError = CARDERROR2_MSG;

	if ( buildError ) {
		interface::popScreen();
		Motherboard::getBoard().errorResponse(buildError);
	}
}

#endif
