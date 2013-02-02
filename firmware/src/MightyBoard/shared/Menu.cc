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

#define FILAMENT_HEAT_TEMP 230
#define SD_MAXFILELENGTH 64

bool ready_fail = false;
static bool singleTool = false;
static bool hasHBP = true;

const static PROGMEM prog_uchar units_mm[] = "mm";
static const char dumpFilename[] = "eeprom_dump.bin";
static void timedMessage(LiquidCrystalSerial& lcd, uint8_t which);

enum sucessState{
	SUCCESS,
	FAIL,
	SECOND_FAIL
};

uint8_t levelSuccess;
uint8_t filamentSuccess;

uint32_t homePosition[PROFILES_HOME_POSITIONS_STORED];

//Macros to expand SVN revision macro into a str
#define STR_EXPAND(x) #x        //Surround the supplied macro by double quotes
#define STR(x) STR_EXPAND(x)

#ifdef MODEL_REPLICATOR2

//  Assumes room for up to 7 + NUL
static void formatTime(char *buf, uint32_t val)
{
	bool hasdigit = false;
	uint8_t idx = 0;

	uint8_t radidx = 0;
	const uint8_t radixcount = 5;
	const uint8_t houridx = 2;
	const uint8_t minuteidx = 4;
	uint32_t radixes[radixcount] = {360000, 36000, 3600, 600, 60};
	if (val >= 3600000)
		val %= 3600000;

	for (radidx = 0; radidx < radixcount; radidx++) {
		char digit = '0';
		uint8_t bit = 8;
		uint32_t radshift = radixes[radidx] << 3;
		for (; bit > 0; bit >>= 1, radshift >>= 1) {
			if (val > radshift) {
				val -= radshift;
				digit += bit;
			}
		}
		if (hasdigit || digit != '0' || radidx >= houridx) {
			buf[idx++] = digit;
			hasdigit = true;
		} else
			buf[idx++] = ' ';
		if (radidx == houridx)
			buf[idx++] = 'h';
		else if (radidx == minuteidx)
			buf[idx++] = 'm';
	}

	buf[idx] = '\0';
}


//  Assumes at least 3 spare bytes
static void digits3(char *buf, uint8_t val)
{
	uint8_t v;

	if ( val >= 100 )
	{
		v = val / 100;
		buf[0] = v + '0';
		val -= v * 100;
	}
	else
		buf[0] = ' ';

	if ( val >= 10 || buf[0] != ' ')
	{
		v = val / 10;
		buf[1] = v + '0';
		val -= v * 10;
	}
	else
		buf[1] = ' ';

	buf[2] = val + '0';
	buf[3] = '\0';
}

#endif

void SplashScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {


	if (forceRedraw || hold_on) {
		lcd.setRow(0);
#ifdef MODEL_REPLICATOR2
		lcd.writeFromPgmspace(eeprom::isSingleTool() ? SPLASH1_MSG : SPLASH12_MSG);
#else
		lcd.writeFromPgmspace(SPLASH1_MSG);
#endif

		lcd.setRow(1);
#ifdef STACK_PAINT
		if ( hold_on ) {
			lcd.writeFromPgmspace(CLEAR_MSG);
			lcd.setRow(1);
               		lcd.writeString((char *)"Free SRAM ");
                	lcd.writeFloat((float)StackCount(), 0, LCD_SCREEN_WIDTH);
		}
		else lcd.writeFromPgmspace(SPLASH2_MSG);
#else
		lcd.writeFromPgmspace(SPLASH2_MSG);
#endif

		// display internal version number if it exists
		if (internal_version != 0){
			lcd.setRow(2);
			lcd.writeFromPgmspace(SPLASH5_MSG);
			
			lcd.setCursor(17,2);
			lcd.writeInt((uint16_t)internal_version,3);
		} else {
			lcd.setRow(2);
			lcd.writeFromPgmspace(SPLASH3_MSG);
		}

		lcd.setRow(3);
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
    singleTool = eeprom::isSingleTool();
    hasHBP = eeprom::hasHBP();

    uint8_t heatSet = eeprom::getEeprom8(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET, 0);
    _rightActive    = (heatSet & (1 << HEAT_MASK_RIGHT)) != 0;
    _leftActive     = (heatSet & (1 << HEAT_MASK_LEFT)) != 0;
    _platformActive = (heatSet & (1 << HEAT_MASK_PLATFORM)) != 0;

    Motherboard &board = Motherboard::getBoard();
    // It's preheating if any of the heaters are active
    //   MBI has some more convoluted logic which considers preheat NOT active
    //   when a heater is on but not enabled for preheat.  That then makes
    //   this menu useless for cancelling heating
    preheatActive =
	    (board.getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0) ||
	    (board.getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0) ||
	    (board.getPlatformHeater().get_set_temperature() > 0);
    itemCount = 4;
    if ( singleTool ) itemCount--;
    if ( !hasHBP ) itemCount--;
}

void HeaterPreheat::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	const prog_uchar *msg;
	bool test;

	switch (index) {
	default:
		return;
	case 0:
		lcd.writeFromPgmspace(preheatActive ? STOP_MSG : GO_MSG);
		return;
	case 1:
		msg = singleTool ? TOOL_MSG : RIGHT_TOOL_MSG;
		test = _rightActive;
		break;
	case 2:
		if ( !singleTool ) {
			msg = LEFT_TOOL_MSG;
			test = _leftActive;
		}
		else if ( hasHBP ) {
			msg = PLATFORM_MSG;
			test = _platformActive;
		}
		else
			return;
		break;
	case 3:
		if ( !singleTool && hasHBP ) {
			msg = PLATFORM_MSG;
			test = _platformActive;
		}
		else
			return;
		break;
	}
	lcd.writeFromPgmspace(msg);
	lcd.setCursor(16, index);  // index % 4
	lcd.writeFromPgmspace(test ? ON_MSG : OFF_MSG);
}
         
void HeaterPreheat::storeHeatByte() {
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
		if ( preheatActive ) {
			Motherboard::getBoard().resetUserInputTimeout();
			temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET,0) *_rightActive; 
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
			if ( !singleTool ) {
				temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET,0) *_leftActive;
				Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
			}
			if ( hasHBP ) {
				temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET,0) *_platformActive;
				Motherboard::getBoard().getPlatformHeater().set_target_temperature(temp);
			}
		}
		else {
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
			Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
			Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
		}
		interface::popScreen();
		interface::pushScreen(&monitorMode);
		//needsRedraw = true;
		return;
	case 1:
		_rightActive = !_rightActive;
		break;
	case 2:
		if ( !singleTool )
			_leftActive = !_leftActive;
		else if ( hasHBP )
			_platformActive = !_platformActive;
		else
			return;
		break;
	case 3:
		if ( !singleTool && hasHBP )
			_platformActive = !_platformActive;
		else
			return;
		break;
	}

	storeHeatByte();
	if ( preheatActive )
		needsRedraw = true;
	else
		lineUpdate = true;
	preheatActive = false;
}

void NozzleCalibrationScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if ( forceRedraw || needsRedraw ) {
		lcd.setRow(0);
		switch (alignmentState) {
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
		lcd.setCursor(15,1);
		lcd.write((selectIndex == 1) ? LCD_CUSTOM_CHAR_RIGHT : ' ');
		lcd.setCursor(17,1);
		lcd.writeInt(xCounter,2);
		break;
	case 2:
		lcd.writeFromPgmspace(YAXIS_MSG);
		lcd.setCursor(15,2);
		lcd.write((selectIndex == 2) ? LCD_CUSTOM_CHAR_RIGHT : ' ');
		lcd.setCursor(17,2);
		lcd.writeInt(yCounter,2);
		break;
	case 3:
		lcd.writeFromPgmspace(DONE_MSG);
		break;
 	}
}

void SelectAlignmentMenu::handleCounterUpdate(uint8_t index, int8_t up) {
   
    switch (index) {
        case 1:
		// update platform counter
		// update right counter
		xCounter += up;
		// keep within appropriate boundaries    
		if ( xCounter > 13 )
			xCounter = 13;
		else if ( xCounter < 1 )
			xCounter = 1;			
		break;
    case 2:
            // update right counter
	    yCounter += up;
	    // keep within appropriate boundaries    
            if ( yCounter > 13 )
		    yCounter = 13;
            else if( yCounter < 1 )
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
    int32_t steps = interval / 3250;
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
    	if(filamentState == FILAMENT_WAIT){
		
		/// if extruder has reached hot temperature, start extruding
		if(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().has_reached_target_temperature()){
			
			int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			/// check for externally manipulated temperature (eg by RepG)
			if(setTemp < filamentTemp[toolID]){
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
			filamentState = FILAMENT_STOP;
		}
		/// if heating timer has eleapsed, alert user that the heater is not getting hot as expected
		else if (filamentTimer.hasElapsed()){
			lcd.clearHomeCursor();
			lcd.writeFromPgmspace(HEATER_ERROR_MSG);
            Motherboard::getBoard().interfaceBlink(25,15);
            filamentState = FILAMENT_DONE;
		}
		/// if extruder is still heating, update heating bar status
		else{
            int16_t currentTemp = Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().getDelta();
            int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
            // check for externally manipulated temperature (eg by RepG)
			if(setTemp < filamentTemp[toolID]){
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
				lcd.setRow(3);
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
		filamentState = FILAMENT_TIMEOUT;
		filamentTimer = Timeout();
		needsRedraw = true;
	}

	
	if (forceRedraw || needsRedraw) {
        //	waiting = true;
		lcd.setRow(0);
		lastHeatIndex = 0;
        switch (filamentState){
			/// starting state - set hot temperature for desired tool and start heat up timer
			case FILAMENT_HEATING:
			        { uint16_t offset = (toolID == 0) ?
					preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET :
				        preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET;
			          filamentTemp[toolID] = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + offset, FILAMENT_HEAT_TEMP);
			        }
				Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().Pause(false);
				Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().set_target_temperature(filamentTemp[toolID]);
				lcd.writeFromPgmspace(HEATING_BAR_MSG);
				lastHeatIndex = 0;
				heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
				filamentState = FILAMENT_WAIT;
				filamentTimer.clear();
				filamentTimer.start(300000000); //5 minutes

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
				lcd.writeFromPgmspace(HEATING_PROG_MSG);
				break;
			/// alert user that filament is ready to extrude
            /// alert user to press M to stop extusion / reversal
            case FILAMENT_STOP:
		    lcd.writeFromPgmspace(STOP_EXIT_MSG);
                Motherboard::getBoard().interfaceBlink(25,15);
                _delay_us(1000000);
                break;
            case FILAMENT_DONE:
				/// user indicated that filament has extruded
                stopMotor();
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

    /// load settings for correct tool and direction
    switch(script) {
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
		if( filamentState == FILAMENT_WAIT )
			break;
            filamentState++;
            Motherboard::getBoard().interfaceBlink(0,0);
            switch (filamentState){
		    /// go to interactive 'OK' scrreen
	    case FILAMENT_OK:
		    stopMotor();
		    Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
		    Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
		    interface::popScreen();
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
    for (int i = 0; i < EXTRUDERS; i++)
	filamentTemp[i] = FILAMENT_HEAT_TEMP;
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
    if ( singleTool )
	    itemCount = 2;
    else
	    itemCount = 4;
    
    switch (index) {
    case 0:
	    lcd.writeFromPgmspace(singleTool ? UNLOAD_SINGLE_MSG : UNLOAD_RIGHT_MSG);
            break;
    case 1:
	    lcd.writeFromPgmspace(singleTool ? LOAD_SINGLE_MSG : LOAD_RIGHT_MSG);
            break;
    case 2:
	    if ( !singleTool )
		    lcd.writeFromPgmspace(UNLOAD_LEFT_MSG);
	    break;
    case 3:
            if ( !singleTool )
		    lcd.writeFromPgmspace(LOAD_LEFT_MSG);
            break;
    }
}

void FilamentMenu::handleSelect(uint8_t index) {
	FilamentScript script;

	switch (index) {
	default:
		return;
        case 0:
		//  interface::pushNoUpdate(&filament);
		script = FILAMENT_RIGHT_REV;
		break;
        case 1:
		//  interface::pushNoUpdate(&filament);
		script = FILAMENT_RIGHT_FOR;
		break;
        case 2:
		//  interface::pushNoUpdate(&filament);
		script = FILAMENT_LEFT_REV;
		break;
        case 3:
		//  interface::pushNoUpdate(&filament);
		script = FILAMENT_LEFT_FOR;
		break;
	}
	filament.setScript(script);
	interface::pushScreen(&filament);
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
	if (cursor < BUF_SIZE-1)
		message[cursor] = '\0';
	else
		message[BUF_SIZE-1] = '\0';
}


void MessageScreen::addMessage(const prog_uchar msg[]) {

	cursor += strlcpy_P(message + cursor, (const prog_char *)msg, BUF_SIZE - cursor);
		
	// ensure that message is always null-terminated
	if (cursor < BUF_SIZE - 1)
		message[cursor] = '\0';
	else
		message[BUF_SIZE-1] = '\0';
}

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
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(JOG1_MSG);

		switch (JogModeScreen){
			case JOG_MODE_X:
				lcd.setRow(1);
				lcd.writeFromPgmspace(JOG2X_MSG);

				lcd.setRow(2);
				lcd.writeFromPgmspace(JOG3X_MSG);

				lcd.setRow(3);
				lcd.writeFromPgmspace(JOG4X_MSG);
				break;
			case JOG_MODE_Y:
				lcd.setRow(1);
				lcd.writeFromPgmspace(JOG2Y_MSG);

				lcd.setRow(2);
				lcd.writeFromPgmspace(JOG3Y_MSG);

				lcd.setRow(3);
				lcd.writeFromPgmspace(JOG4Y_MSG);
				break;
			case JOG_MODE_Z:
				lcd.setRow(1);
				lcd.writeFromPgmspace(JOG2Z_MSG);

				lcd.setRow(2);
				lcd.writeFromPgmspace(JOG3Z_MSG);

				lcd.setRow(3);
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
		// fallthrough
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

	lcd.setRow(yOffset);
	lcd.writeFloat(filamentUsed, precision, LCD_SCREEN_WIDTH - ((precision == 1) ? 2 : 1));
	lcd.writeFromPgmspace((precision == 1) ? MILLIMETERS_MSG : METERS_MSG);
}

void filamentOdometers(bool odo, uint8_t yOffset, LiquidCrystalSerial &lcd) {

	// Get lifetime filament used for A & B axis and sum them
	// into filamentUsed
	lcd.setRow(yOffset);
	lcd.writeFromPgmspace(odo ? FILAMENT_LIFETIME1_MSG : FILAMENT_LIFETIME2_MSG);

	float filamentUsedA, filamentUsedB;
	filamentUsedA = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0),                  A_AXIS);
	filamentUsedB = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t),0), B_AXIS);
	printFilamentUsed(filamentUsedA + filamentUsedB, yOffset, lcd);

	// Get trip filament used for A & B axis and sum them into filamentUsed
	lcd.setRow(++yOffset);
	lcd.writeFromPgmspace(odo ? FILAMENT_TRIP1_MSG : FILAMENT_TRIP2_MSG);

	filamentUsedA -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP, 0),                  A_AXIS);
	filamentUsedB -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t),0), B_AXIS);
	printFilamentUsed(filamentUsedA + filamentUsedB, yOffset, lcd);
}

void FilamentOdometerScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw || needsRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(FILAMENT_ODOMETER_MSG);
		filamentOdometers(true, 1, lcd);
		lcd.setRow(3);
		lcd.writeFromPgmspace(FILAMENT_RESET_TRIP_MSG);
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
        case ButtonArray::LEFT:
        	interface::popScreen();
		break;
        default:
                break;
	}
}

void MonitorMode::reset() {
	updatePhase = 0;
	singleTool = eeprom::isSingleTool();
	hasHBP = eeprom::hasHBP();
    toggleBlink = false;
    heating = false;
    heatLights = true;
    LEDClear = true;
#ifdef MODEL_REPLICATOR2
    buildTimePhase = BUILD_TIME_PHASE_FIRST;
    lastBuildTimePhase = BUILD_TIME_PHASE_FIRST;
    lastElapsedSeconds = 0.0;
#endif
}

void MonitorMode::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
#ifdef MODEL_REPLICATOR2
    const static PROGMEM prog_uchar mon_elapsed_time[]       = "Elapsed:       0h00m";
    const static PROGMEM prog_uchar mon_time_left[]          = "TimeLeft:      0h00m";
    const static PROGMEM prog_uchar mon_time_left_secs[]     = "secs";
    const static PROGMEM prog_uchar mon_time_left_none[]     = "   none";
    const static PROGMEM prog_uchar mon_zpos[] 	             = "ZPos:               ";
    const static PROGMEM prog_uchar mon_filament[]           = "Filament:      0.00m";
#ifdef ACCEL_STATS
    const static PROGMEM prog_uchar mon_speed[] 	     = "Acc:                ";
#endif
#endif
    Motherboard& board = Motherboard::getBoard();
   
    if(!heating){
		if(board.getExtruderBoard(0).getExtruderHeater().isHeating() ||
            board.getExtruderBoard(1).getExtruderHeater().isHeating() ||
                board.getPlatformHeater().isHeating()){
            heating = true;
            lastHeatIndex = 0;
            lcd.setRow(0);
            heatLights = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
            LEDClear = true;
	    lcd.writeFromPgmspace(HEATING_SPACES_MSG);
		}
	}

    char * name;
	if (forceRedraw) {
                
		lcd.clearHomeCursor();
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
		{
			uint8_t i = 0;
			while((*name != '.') && (*name != '\0') && (++i <= LCD_SCREEN_WIDTH))
				lcd.write(*name++);
		}
                    
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

		uint8_t row;
		if (hasHBP) {
			lcd.setRow(3);
			lcd.writeFromPgmspace(PLATFORM_TEMP_MSG);
			row = 2;
		}
		else row = 3;

		if(singleTool){
			lcd.setRow(row--);
			lcd.writeFromPgmspace(EXTRUDER_TEMP_MSG);
		}else{
			lcd.setRow(row--);
			lcd.writeFromPgmspace(EXTRUDER2_TEMP_MSG);
			lcd.setRow(row--);
			lcd.writeFromPgmspace(EXTRUDER1_TEMP_MSG);
		}
		while (row >= 1) {
			lcd.setRow(row--);
			lcd.writeFromPgmspace(CLEAR_MSG);
		}

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
            lcd.setRow(0);
            lcd.writeFromPgmspace(CLEAR_MSG);
            lcd.setRow(0);
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
			lcd.setCursor(12,hasHBP ? 1 : 2);
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
			uint8_t row = hasHBP ? 1 : 2;
			if(!board.getExtruderBoard(0).getExtruderHeater().has_failed() && !board.getExtruderBoard(0).getExtruderHeater().isPaused()){           
				data = board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
				if(data > 0){
					lcd.setCursor(15,row);
					lcd.writeFromPgmspace(ON_CELCIUS_MSG);
					lcd.setCursor(16,row);
					lcd.writeInt(data,3);
				}
				else{
					lcd.setCursor(15,row);
					lcd.writeFromPgmspace(CELCIUS_MSG);
				}
			}
		}
		break;
	case 2:
		lcd.setCursor(12,hasHBP ? 2 : 3);
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
			uint8_t row = hasHBP ? 2 : 3;
			lcd.setCursor(16,row);
			data = board.getExtruderBoard(!singleTool * 1).getExtruderHeater().get_set_temperature();
			if(data > 0){
				lcd.setCursor(15,row);
				lcd.writeFromPgmspace(ON_CELCIUS_MSG);
				lcd.setCursor(16,row);
				lcd.writeInt(data,3);
			}else{
				lcd.setCursor(15,row);
				lcd.writeFromPgmspace(CELCIUS_MSG);
			}
		}
		break;

	case 4:
		if(hasHBP){
			lcd.setCursor(12,3);
			data = board.getPlatformHeater().get_current_temperature();
			if(board.getPlatformHeater().has_failed()){
				lcd.writeFromPgmspace(NA_MSG);
			} else if (board.getPlatformHeater().isPaused()){
				lcd.writeFromPgmspace(WAITING_MSG);
			} else {
				lcd.writeInt(data,3);
			}
		}
		break;

	case 5:
		if(hasHBP){
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
#ifdef MODEL_REPLICATOR2
	case 7 :
		if (hasHBP && !singleTool)
			break;
		enum host::HostState hostState = host::getHostState();
		if ( (hostState != host::HOST_STATE_BUILDING ) && ( hostState != host::HOST_STATE_BUILDING_FROM_SD ))
			break;

		bool okButtonHeld = interface::isButtonPressed(ButtonArray::DOWN);

		//Holding the down button stops rotation
        	if ( okButtonHeld )
			buildTimePhase = lastBuildTimePhase;

		char buf[17];
		float secs;
		int32_t tsecs;
		Point position;
		uint8_t precision;
		float filamentUsed, lastFilamentUsed;

		switch (buildTimePhase) {

		case BUILD_TIME_PHASE_ELAPSED_TIME:
			lcd.setRow(1);
			lcd.writeFromPgmspace(mon_elapsed_time);
			lcd.setCursor(13,1);
			if ( host::isBuildComplete() ) secs = lastElapsedSeconds; //We stop counting elapsed seconds when we are done
			else {
				lastElapsedSeconds = host::getPrintSeconds();
				secs = lastElapsedSeconds;
			}
			formatTime(buf, (uint32_t)secs);
			lcd.writeString(buf);
			break;

		case BUILD_TIME_PHASE_TIME_LEFT:
			tsecs = command::estimatedTimeLeftInSeconds();
			if ( tsecs > 0 ) {
				lcd.setRow(1);
				lcd.writeFromPgmspace(mon_time_left);
				lcd.setCursor(13,1);
				if ( (tsecs > 0 ) && (tsecs < 60) && ( host::isBuildComplete() ) ) {
					digits3(buf, (uint8_t)tsecs);
					lcd.writeString(buf);
					lcd.writeFromPgmspace(mon_time_left_secs);	
				} else if (( tsecs <= 0) || ( host::isBuildComplete()) ) {
					command::addFilamentUsed();
					lcd.writeFromPgmspace(mon_time_left_none);
				} else {
					formatTime(buf, (uint32_t)tsecs);
					lcd.writeString(buf);
				}
				break;
			}
			//We can't display the time left, so we drop into ZPosition instead
			else	buildTimePhase = (enum BuildTimePhase)((uint8_t)buildTimePhase + 1);

		case BUILD_TIME_PHASE_ZPOS:
			lcd.setRow(1);
			lcd.writeFromPgmspace(mon_zpos);
			position = steppers::getStepperPosition();
			// if the position is < -8000, we likely haven't yet defined our Z position
			if (position[Z_AXIS] > -8000) {
				lcd.setCursor(6,1);
			
				//Divide by the axis steps to mm's
				lcd.writeFloat(stepperAxisStepsToMM(position[2], Z_AXIS), 3, LCD_SCREEN_WIDTH-2);
				lcd.writeFromPgmspace(units_mm);
			}
			break;

		case BUILD_TIME_PHASE_FILAMENT:
			lcd.setRow(1);
			lcd.writeFromPgmspace(mon_filament);
			lcd.setCursor(9,1);
			lastFilamentUsed = stepperAxisStepsToMM(command::getLastFilamentLength(0) + command::getLastFilamentLength(1), A_AXIS);
			if ( lastFilamentUsed != 0.0 )	filamentUsed = lastFilamentUsed;
			else				filamentUsed = stepperAxisStepsToMM((command::getFilamentLength(0) + command::getFilamentLength(1)), A_AXIS);
			filamentUsed /= 1000.0;	//convert to meters
			if	( filamentUsed < 0.1 )	{
				filamentUsed *= 1000.0;	//Back to mm's
				precision = 1;
			}
			else if ( filamentUsed < 10.0 )	 precision = 4;
			else if ( filamentUsed < 100.0 ) precision = 3;
			else				 precision = 2;
			if ( precision == 1 ) {
				lcd.writeFloat(filamentUsed, precision, LCD_SCREEN_WIDTH - 2);
				lcd.writeFromPgmspace(units_mm);
			}
			else {
				lcd.writeFloat(filamentUsed, precision, LCD_SCREEN_WIDTH - 1);
				lcd.write('m');
			}
			break;

		case BUILD_TIME_PHASE_LAST:
			break;

#ifdef ACCEL_STATS
		case BUILD_TIME_PHASE_ACCEL_STATS:
			float minSpeed, avgSpeed, maxSpeed;
			accelStatsGet(&minSpeed, &avgSpeed, &maxSpeed);
			lcd.setRow(1);
			lcd.writeFromPgmspace(mon_speed);
			lcd.setCursor(4,1);
			if ( minSpeed < 100.0 )	lcd.write(' ');	//If we have space, pad out a bit
			lcd.writeFloat(minSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write('/');
			lcd.writeFloat(avgSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write('/');
			lcd.writeFloat(maxSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write(' ');
			break;
#endif
		}

        	if ( ! okButtonHeld ) {
			//Advance buildTimePhase and wrap around
			lastBuildTimePhase = buildTimePhase;
			buildTimePhase = (enum BuildTimePhase)((uint8_t)buildTimePhase + 1);

			if ( buildTimePhase >= BUILD_TIME_PHASE_LAST )
				buildTimePhase = BUILD_TIME_PHASE_FIRST;
		}
		break;
#endif
	}

	updatePhase++;
#ifdef MODEL_REPLICATOR2
	if (updatePhase > 7) {
#else
	if (updatePhase > 6) {
#endif
		updatePhase = 0;
	}

#ifdef DEBUG_ONSCREEN
	lcd.setRow(0);
	lcd.writeString((char *)"DOS1: ");
	lcd.writeFloat(debug_onscreen1, 3, LCD_SCREEN_WIDTH);
	lcd.writeString((char *)" ");

	lcd.setRow(1);
	lcd.writeString((char *)"DOS2: ");
	lcd.writeFloat(debug_onscreen2, 3, LCD_SCREEN_WIDTH);
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
	    || forceRedraw || needsRedraw) {
		// Redraw the whole menu
		lcd.clear();
		
		for (uint8_t i = 0; i < LCD_SCREEN_HEIGHT; i++) {
			// Instead of using lcd.clear(), clear one line at a time so there
			// is less screen flickr.

			if (i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT +1 > itemCount) {
				break;
			}

			lcd.setCursor(1, i);
			// Draw one page of items at a time
			drawItem(i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT, lcd);
		}
	}
	else if (lineUpdate) {
		lcd.setCursor(1, itemIndex % LCD_SCREEN_HEIGHT);
		drawItem(itemIndex, lcd);
	}
	else {
		// Only need to clear the previous cursor
		lcd.setRow(lastDrawIndex % LCD_SCREEN_HEIGHT);
		lcd.write(' ');
	}

	lcd.setRow(itemIndex % LCD_SCREEN_HEIGHT);
	if (((itemIndex % LCD_SCREEN_HEIGHT) == (LCD_SCREEN_HEIGHT - 1)) &&
	    (itemIndex < itemCount-1))
		lcd.write(LCD_CUSTOM_CHAR_DOWN);
	else if (((itemIndex % LCD_SCREEN_HEIGHT) == 0) && (itemIndex > 0))
		 lcd.write(LCD_CUSTOM_CHAR_UP);
	else    
		lcd.write(LCD_CUSTOM_CHAR_RIGHT);
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
        	// decrement index
        	if ( itemIndex > firstItemIndex )
			itemIndex--;
		//Wrap around to bottom of menu
		else
			itemIndex = itemCount - 1;
		break;
        case ButtonArray::DOWN:
		// increment index
		if ( ++itemIndex >= itemCount )
			itemIndex = firstItemIndex;
		break;
        default:
                break;
	}
}

void CounterMenu::reset() {
	selectMode = false;
	selectIndex = -1;
	firstSelectIndex = 0;
	lastSelectIndex = 255;
	Menu::reset();
}

void CounterMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
        case ButtonArray::CENTER:
		if ( (itemIndex >= firstSelectIndex) &
		     (itemIndex <= lastSelectIndex ))
			selectMode = !selectMode;
		if ( selectMode ) {
			selectIndex = itemIndex;
			lineUpdate = true;
		}
		else {
			selectIndex = -1;
			handleSelect(itemIndex);
			lineUpdate = true;
		}
		break;
        case ButtonArray::LEFT:
		if ( !selectMode )
			interface::popScreen();
		break;
        case ButtonArray::RIGHT:
		break;
        case ButtonArray::UP:
		if ( selectMode ) {
			handleCounterUpdate(itemIndex, 1);
			lineUpdate = true;
		}
		// increment index
		else {
			if (itemIndex > firstItemIndex)
				itemIndex--;
			else
				itemIndex = itemCount - 1;
		}
		break;
        case ButtonArray::DOWN:
		if ( selectMode ) {
			handleCounterUpdate(itemIndex, -1);
			lineUpdate = true;
		}
		// decrement index
		else if ( ++itemIndex >= itemCount )
			itemIndex = firstItemIndex;
		break;
        default:
                break;
	}
}

PreheatSettingsMenu::PreheatSettingsMenu(uint8_t optionsMask) :
	CounterMenu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)4) {
	reset();
}
  
void PreheatSettingsMenu::resetState() {
	counterRight = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET, 220);
	counterLeft = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET, 220);
	counterPlatform = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET, 100);
	singleTool = eeprom::isSingleTool();
	hasHBP = eeprom::hasHBP();
	offset = 0;
	if ( singleTool ) offset++;
	if ( !hasHBP ) offset++;
	itemIndex = firstItemIndex = 1 + offset;
	counterRight = 50;
}

void PreheatSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	bool selected = selectIndex == index;
	uint8_t row = index;

	if ( index ) {
		if  ( index < firstItemIndex )
			return;
		index -= offset;
	}

	switch (index) {
	case 0:
		lcd.writeFromPgmspace(PREHEAT_SET_MSG);
		break;
	case 1:
		lcd.writeFromPgmspace(singleTool ? EXTRUDER_SPACES_MSG :
				      RIGHT_SPACES_MSG);
		if ( selected ) {
			lcd.setCursor(16, row);
			lcd.write(LCD_CUSTOM_CHAR_RIGHT);
		}
		lcd.setCursor(17, row);
		lcd.writeInt(counterRight, 3);
		break;
	case 2:
		if ( !singleTool ) {
			lcd.writeFromPgmspace(LEFT_SPACES_MSG);
			if ( selected ) {
				lcd.setCursor(16, row);
				lcd.write(LCD_CUSTOM_CHAR_RIGHT);
			}
			lcd.setCursor(17, row);
			lcd.writeInt(counterLeft, 3);
		}
		else if ( hasHBP ) {
			lcd.writeFromPgmspace(PLATFORM_SPACES_MSG);
			if ( selected ) {
				lcd.setCursor(16, row);
				lcd.write(LCD_CUSTOM_CHAR_RIGHT);
			}
			lcd.setCursor(17, row);
			lcd.writeInt(counterPlatform, 3);
		}
		break;
         case 3:
		 if ( !singleTool && hasHBP ) {
			 lcd.writeFromPgmspace(PLATFORM_SPACES_MSG);
			 if ( selected ) {
				 lcd.setCursor(16, row);
				 lcd.write(LCD_CUSTOM_CHAR_RIGHT);
			 }
			 lcd.setCursor(17, row);
			 lcd.writeInt(counterPlatform, 3);
		 }
		 break;
	}
}

void PreheatSettingsMenu::handleCounterUpdate(uint8_t index, int8_t up) {

	if ( index < firstItemIndex )
		return;

	index -= offset;

	switch (index) {
	case 1:
		// update right counter
		counterRight += up;
		if ( counterRight > 260 )
			counterRight = 260;
		break;
	case 2:
		if ( !singleTool ) {
			// update left counter
			counterLeft += up;
			if ( counterLeft > 260 )
				counterLeft = 260;
		}
		else if ( hasHBP ) {
			// update platform counter
			counterPlatform += up;
			if (counterPlatform > 120 )
				counterPlatform = 120;
		}
		break;
        case 3:
		// update platform counter
		if ( !singleTool && hasHBP ) {
			counterPlatform += up;
			if ( counterPlatform > 120 )
				counterPlatform = 120;
		}
		break;
	}
}

void PreheatSettingsMenu::handleSelect(uint8_t index) {
	if ( index ) {
		if  ( index < firstItemIndex )
			return;
		index -= offset;
	}

	switch (index) {
	case 0:
		break;
        case 1:
            // store right tool setting
            eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), counterRight);
            break;
        case 2:
            if ( !singleTool )
		    // store left tool setting
		    eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET), counterLeft);
	    else if ( hasHBP )
		    eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET), counterPlatform);
            break;
        case 3:
		if ( !singleTool && hasHBP )
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
                lcd.clearHomeCursor();
                lcd.writeFromPgmspace(PROFILE_PROFILE_NAME_MSG);

                lcd.setRow(3);
                lcd.writeFromPgmspace(UPDNLRM_MSG);
        }

        lcd.setRow(1);
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
			const static PROGMEM prog_uchar msg4[]		= "Press UP to proceed.";

			lcd.setRow(0);
			lcd.writeFromPgmspace(msg1);

			lcd.setRow(1);
			lcd.writeFromPgmspace(msg2);

			lcd.setRow(2);
			lcd.writeFromPgmspace(CLEAR_MSG);

			lcd.setRow(3);
			lcd.writeFromPgmspace(msg4);
		}
	}
	else {
		if ( itemSelected != -1 )
			lcd.clearHomeCursor();

		const static PROGMEM prog_uchar message_dump[]		= "Saving...";
		const static PROGMEM prog_uchar message_restore[]	= "Restoring...";

		switch ( itemSelected ) {
			case 0:	//Dupm
				if ( ! sdcard::fileExists(dumpFilename) ) {
					lcd.writeFromPgmspace(message_dump);
					if ( ! eeprom::saveToSDFile(dumpFilename) )
						timedMessage(lcd, 1);
				} else
					timedMessage(lcd, 2);
				interface::popScreen();
				break;

			case 1: //Restore
				if ( sdcard::fileExists(dumpFilename) ) {
					lcd.writeFromPgmspace(message_restore);
					if ( ! eeprom::restoreFromSDFile(dumpFilename) )
						timedMessage(lcd, 3);
					host::stopBuildNow();
				} else {
					timedMessage(lcd, 4);
					interface::popScreen();
				}
				break;

			case 2: //Erase
				timedMessage(lcd, 5);
				eeprom::erase();
				interface::popScreen();
				host::stopBuildNow();
				break;
			default:
				Menu::update(lcd, forceRedraw);
				break;
		}

		lcd.setRow(3);
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
                lcd.clearHomeCursor();
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

                lcd.setRow(3);
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

        lcd.setRow(1);
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
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(PAUSE_AT_ZPOS_MSG);

                lcd.setRow(3);
                lcd.writeFromPgmspace(UPDNLM_MSG);
        }

        lcd.setRow(1);
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
    
void ActiveBuildMenu::resetState() {
	// itemIndex = 0;
	// firstItemIndex = 0;
	is_paused = command::isPaused();
	if ( is_paused )	itemCount = 6;
	else			itemCount = 5;

	//If any of the heaters are on, we provide another
	//menu options, "Heaters Off"
	if (( is_paused ) && 
	    ( Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0 || 
	      Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0 ||
	      Motherboard::getBoard().getPlatformHeater().get_set_temperature() > 0 ))
		itemCount++;
}

void ActiveBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	
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
		lcd.writeFromPgmspace(is_paused ? UNPAUSE_MSG : PAUSE_MSG);
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

void ActiveBuildMenu::handleSelect(uint8_t index) {
	
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
		if ( is_paused ) {
			for (uint8_t i = 3; i < STEPPER_COUNT; i++) 
				steppers::enableAxis(i, false);	
		}
		else
			interface::popScreen();
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
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(BUILD_TIME_MSG);

		lcd.setRow(1);
		lcd.writeFromPgmspace(Z_POSITION_MSG);

		lcd.setRow(2);
		lcd.writeFromPgmspace(FILAMENT_MSG);

		lcd.setRow(3);
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
				lcd.setRow(3);
				//Replaced with this message, because writeInt / writeInt32 can't display it anyway.
				//and 1,000,000,000 lines would represent 115 days of printing at 100 moves per second
				lcd.writeFromPgmspace(PRINTED_TOO_LONG_MSG);
			}else{
				
				uint8_t digits = 1;
				for (uint32_t i = 10; i < 0x19999999; i*=10){
					if (line_number < i) break;
					digits ++;
				}			
				lcd.setCursor(LCD_SCREEN_WIDTH - digits, 3);
				lcd.writeInt32(line_number, digits);
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

	char *name;
	
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
	Menu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN),(uint8_t)18),
	monitorMode((uint8_t)0),
	splash((uint8_t)0),
	filament((uint8_t)0),
	botStats((uint8_t)0),
	jogger((uint8_t)0),
	set((uint8_t)0),
	preheat((uint8_t)0),
	profilesMenu((uint8_t)0),
	homeOffsetsMode((uint8_t)0),
	reset_settings((uint8_t)0),
	alignment((uint8_t)0),
	filamentOdometer((uint8_t)0),
	eepromMenu((uint8_t)0) {
	singleTool = eeprom::isSingleTool();
	if (singleTool) itemCount--; // No nozzleCalibration
	blinkLED = false;
	reset();
}
void UtilitiesMenu::resetState(){
	singleTool = eeprom::isSingleTool();
	itemCount = 18;
	if ( singleTool ) --itemCount;
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
	// ------ next screen ------
	case 4:
		lcd.writeFromPgmspace(HOME_AXES_MSG);
		break;
	case 5:
		lcd.writeFromPgmspace(BOT_STATS_MSG);
		break;
	case 6:
		lcd.writeFromPgmspace(FILAMENT_ODOMETER_MSG);
		break;
	case 7:
		lcd.writeFromPgmspace(SETTINGS_MSG);
		break;
	// ------ next screen ------
	case 8:
		lcd.writeFromPgmspace(PROFILES_MSG);
		break;
	case 9:
                lcd.writeFromPgmspace(HOME_OFFSETS_MSG);
		break;
	case 10:
		lcd.writeFromPgmspace(JOG_MSG);
		break;
	case 11:
		lcd.writeFromPgmspace(stepperEnable ? ESTEPS_MSG : DSTEPS_MSG);
		break;
	// ------ next screen ------
	case 12:
		lcd.writeFromPgmspace(blinkLED ? LED_STOP_MSG : LED_BLINK_MSG);
		break;
	case 13:
		lcd.writeFromPgmspace(singleTool ? RESET_MSG : NOZZLES_MSG);
		break;
	case 14:
		lcd.writeFromPgmspace(singleTool ? EEPROM_MSG : RESET_MSG);
		break;
	case 15:
		lcd.writeFromPgmspace(singleTool ? VERSION_MSG : EEPROM_MSG);
		break;
	// ------ next screen ------
	case 16:
		lcd.writeFromPgmspace(singleTool ? EXIT_MSG : VERSION_MSG);
		break;
	case 17:
		lcd.writeFromPgmspace(EXIT_MSG);
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
		// bot stats
		interface::pushScreen(&botStats);
		break;
	case 6:
		// Filament Odometer
		interface::pushScreen(&filamentOdometer);
		break;
	case 7:
		// settings menu
		interface::pushScreen(&set);
		break;
	case 8:
		// Profiles
		interface::pushScreen(&profilesMenu);
		break;
	case 9:
		// Home Offsets
		interface::pushScreen(&homeOffsetsMode);
		break;
	case 10:
		// Jog axes
		interface::pushScreen(&jogger);
		break;
	case 11:
		for (int i = 0; i < STEPPER_COUNT; i++) 
			steppers::enableAxis(i, stepperEnable);
		lineUpdate = true;
		stepperEnable = !stepperEnable;
		break;
	case 12:
		blinkLED = !blinkLED;
		RGB_LED::setLEDBlink(blinkLED ? 150 : 0);
		lineUpdate = true;		 
		break;
	case 13:
		if ( singleTool ) interface::pushScreen(&reset_settings);
		else interface::pushScreen(&alignment);
		break;
	case 14:
		if ( singleTool ) interface::pushScreen(&eepromMenu);
		else interface::pushScreen(&reset_settings);
		break;
	case 15:
		//Eeprom Menu
		if ( !singleTool )
			interface::pushScreen(&eepromMenu);
		else
		{
			splash.SetHold(true);
			interface::pushScreen(&splash);
		}
		break;
	case 16:
		if ( !singleTool ) {
			splash.SetHold(true);
			interface::pushScreen(&splash);
		}
		else
			interface::popScreen();
		break;
	case 17:
		interface::popScreen();
		break;
	}
}

void BotStatsScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if ( !forceRedraw )
		return;

	/// TOTAL PRINT LIFETIME
	lcd.clearHomeCursor();
	lcd.writeFromPgmspace(TOTAL_TIME_MSG);

	uint16_t total_hours = eeprom::getEeprom16(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::HOURS,0);
	uint8_t digits = 1;
	for (uint32_t i = 10; i < 10000; i *= 10) {
		if ( i > total_hours ) break;
		digits++;
	} 
	lcd.setCursor(19 - digits, 0);
	lcd.writeInt32(total_hours, digits);

	/// LAST PRINT TIME
	uint8_t build_hours;
	uint8_t build_minutes;
	host::getPrintTime(build_hours, build_minutes);

	lcd.setRow(1);
	lcd.writeFromPgmspace(LAST_TIME_MSG);
    
	lcd.setCursor(14, 1);
	lcd.writeInt(build_hours, 2);

	lcd.setCursor(17, 1);
	lcd.writeInt(build_minutes, 2);

	/// TOTAL FILAMENT USED
	filamentOdometers(false, 2, lcd);
}

void BotStatsScreen::reset() {
}

void BotStatsScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	if ( button == ButtonArray::LEFT )
		interface::popScreen();
}

SettingsMenu::SettingsMenu(uint8_t optionsMask) :
	CounterMenu(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)10
#ifdef DITTO_PRINT
		+1
#endif
		) {
    reset();
}

void SettingsMenu::resetState(){
    hasHBP = eeprom::hasHBP();
    singleExtruder = 2 != eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1);
    soundOn = 0 != eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS, 1);
    LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 0);
    heatingLEDOn = 0 != eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
    accelerationOn = 0 != eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACCELERATION_ACTIVE, 0x01);
    overrideGcodeTempOn = 0 != eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0);
    pauseHeatOn = 0 != eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, 1);
    extruderHoldOn = 0 != eeprom::getEeprom8(eeprom_offsets::EXTRUDER_HOLD,
					     DEFAULT_EXTRUDER_HOLD);
    toolOffsetSystemOld = 0 == eeprom::getEeprom8(eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
						  DEFAULT_TOOLHEAD_OFFSET_SYSTEM);
#ifdef DITTO_PRINT
    dittoPrintOn = 0 != eeprom::getEeprom8(eeprom_offsets::DITTO_PRINT_ENABLED, 0);
    if ( singleExtruder ) dittoPrintOn = false;
#endif
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	bool test;
	const prog_uchar *msg;
	uint8_t extra = 3;
	uint8_t selIndex = selectIndex;

	uint8_t row = index % 4;
#ifndef DITTO_PRINT
	index++;
	selIndex++;
	// Anything in the same page with LED color needs a wider right col
	if ( 5 <= index && index <= 8) extra = 0;
#else
	if ( 4 <= index && index <= 7) extra = 0;
#endif

	lcd.setCursor(13 + extra, row);
	lcd.write((selIndex == index) ? LCD_CUSTOM_CHAR_RIGHT : ' ');

	switch (index) {
	default:
		return;
#ifdef DITTO_PRINT
	case 0:
		lcd.setCursor(1, row);
		lcd.writeFromPgmspace(DITTO_PRINT_MSG);
		lcd.setCursor(14 + extra, row);
		if ( singleExtruder )
			lcd.writeFromPgmspace(DISABLED_MSG);
		else
			lcd.writeFromPgmspace(dittoPrintOn ? ON_MSG : OFF_MSG);
		return;
#endif
	case 1:
		msg = OVERRIDE_GCODE_TEMP_MSG;
		test = overrideGcodeTempOn;
		break;
	case 2:
		msg = PAUSE_HEAT_MSG;
		test = pauseHeatOn;
		break;
        case 3:
		msg = SOUND_MSG;
		test = soundOn;
		break;
	case 4:
		msg = LED_HEAT_MSG;
		test = heatingLEDOn;
		break;
	// LED Color should be on page 2 along with the other
        // items needing a wider right column
	case 5:
		lcd.setCursor(1, row);
		lcd.writeFromPgmspace(LED_MSG);
		lcd.setCursor(14 + extra, row);
		switch (LEDColor) {
                case LED_DEFAULT_RED:    msg = RED_COLOR_MSG; break;
                case LED_DEFAULT_ORANGE: msg = ORANGE_COLOR_MSG; break;
                case LED_DEFAULT_PINK:   msg = PINK_COLOR_MSG; break;
                case LED_DEFAULT_GREEN:  msg = GREEN_COLOR_MSG; break;
                case LED_DEFAULT_BLUE:   msg = BLUE_COLOR_MSG; break;
                case LED_DEFAULT_PURPLE: msg = PURPLE_COLOR_MSG; break;
                case LED_DEFAULT_CUSTOM: msg = CUSTOM_COLOR_MSG; break;
		default:
                case LED_DEFAULT_WHITE:  msg = WHITE_COLOR_MSG; break;
		}
		lcd.writeFromPgmspace(msg);
		return;
	case 6:
		msg = ACCELERATE_MSG;
		test = accelerationOn;
		break;
        case 7:
		lcd.setCursor(1, row);
		lcd.writeFromPgmspace(TOOL_COUNT_MSG);
		lcd.setCursor(14 + extra, row);
		lcd.write(singleExtruder ? '1' : '2');
		return;
	case 8:
		msg = EXTRUDER_HOLD_MSG;
		test = extruderHoldOn;
		break;
	case 9:
		lcd.setCursor(1, row);
		lcd.writeFromPgmspace(HBP_MSG);
		lcd.setCursor(14 + extra, row);
		lcd.writeFromPgmspace(hasHBP ? YES_MSG : NO_MSG);
		return;
	case 10:
		lcd.setCursor(1, row);
		lcd.writeFromPgmspace(TOOL_OFFSET_SYSTEM_MSG);
		lcd.setCursor(14 + extra, row);
		lcd.writeFromPgmspace(toolOffsetSystemOld ? OLD_MSG : NEW_MSG);
		return;
 	}
	lcd.setCursor(1, row);
	lcd.writeFromPgmspace(msg);
	lcd.setCursor(14 + extra, row);
	lcd.writeFromPgmspace(test ? ON_MSG : OFF_MSG);
}

void SettingsMenu::handleCounterUpdate(uint8_t index, int8_t up) {

#ifndef DITTO_PRINT
	index++;
#endif
	switch (index) {
#ifdef DITTO_PRINT
	case 0:
		if ( singleExtruder ) break;
		// update right counter
		dittoPrintOn = !dittoPrintOn;
		break;
#endif
	case 1:
		// update right counter
		overrideGcodeTempOn = !overrideGcodeTempOn;
		break;
	case 2:
		// update right counter
		pauseHeatOn = !pauseHeatOn;
		break;
        case 3:
		// update right counter
		soundOn = !soundOn;
		break;
        case 4:
		// update right counter
		heatingLEDOn = !heatingLEDOn;
		break;
        case 5:
		// update left counter
		LEDColor += up;
		// keep within appropriate boundaries
		if( LEDColor > 6 )
			LEDColor = 0;
		else if( LEDColor < 0 )
			LEDColor = 6;
		eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS,
				  LEDColor);
		RGB_LED::setDefaultColor();				
		break;
	case 6:
		 // update right counter
		 accelerationOn = !accelerationOn;
		 break;
        case 7:
		// update platform counter
		// update right counter
		singleExtruder = !singleExtruder;
		break;
	case 8:
		// update right counter
		extruderHoldOn = !extruderHoldOn;
		break;
        case 9:
		// update right counter
		hasHBP = !hasHBP;
		break;
	case 10:
		toolOffsetSystemOld = !toolOffsetSystemOld;
		break;
	}
}


void SettingsMenu::handleSelect(uint8_t index) {
#ifndef DITTO_PRINT
	index++;
#endif
	switch (index) {
	default:
		return; // prevents line_update from changing;
#ifdef DITTO_PRINT
	case 0:
		if ( singleExtruder )
			return;
		eeprom_write_byte((uint8_t*)eeprom_offsets::DITTO_PRINT_ENABLED,
				  dittoPrintOn ? 1 : 0);
		command::reset();
		break;
#endif
	case 1:
		eeprom_write_byte((uint8_t*)eeprom_offsets::OVERRIDE_GCODE_TEMP,
				  overrideGcodeTempOn ? 1 : 0);
		break;
	case 2:
		eeprom_write_byte((uint8_t*)eeprom_offsets::HEAT_DURING_PAUSE,
				  pauseHeatOn ? 1 : 0);
		break;
	case 3:
		// update sound preferences
		eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS,
				  soundOn ? 1 : 0);
		Piezo::reset();
		break;
	case 4:
		// update LEDHeatingflag
		eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS +
				  blink_eeprom_offsets::LED_HEAT_OFFSET,
				  heatingLEDOn ? 1 : 0);
		break;
	case 5:
		// update LED preferences
		eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS,
				  LEDColor);
		RGB_LED::setDefaultColor();
		break;
	case 6:
		eeprom_write_byte((uint8_t*)eeprom_offsets::ACCELERATION_SETTINGS +
				  acceleration_eeprom_offsets::ACCELERATION_ACTIVE,
				  accelerationOn ? 1 : 0);
		steppers::reset();
		break;
	case 7:
		eeprom::setToolHeadCount(singleExtruder ? 1 : 2);
		if ( singleExtruder )
			Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
		command::reset();
		break;
	case 8:
		eeprom_write_byte((uint8_t*)eeprom_offsets::EXTRUDER_HOLD,
				  extruderHoldOn ? 1 : 0);
		command::reset();
		break;
	case 9:
		eeprom_write_byte((uint8_t*)eeprom_offsets::HBP_PRESENT,
				  hasHBP ? 1 : 0);
		if ( !hasHBP )
			Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
		command::reset();
		break;
	case 10:
		eeprom_write_byte((uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
				  toolOffsetSystemOld ? 0 : 1);
		command::reset();
		break;
	}
	lineUpdate = 1;
}

SDMenu::SDMenu(uint8_t optionsMask) :
	Menu(optionsMask  | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)0),
	updatePhase(0), updatePhaseDivisor(0), drawItemLockout(false),
	selectable(false), folderStackIndex(-1) {
	reset();
}

void SDMenu::resetState() {
	itemCount = countFiles();
	if ( !itemCount ) {
		folderStackIndex = -1;
		itemCount  = 1;
		selectable = false;
	}
	else {
		selectable = true;
		++itemCount; // +1 for "exit menu"
	}
	updatePhase = 0;	
	updatePhaseDivisor = 0;
	lastItemIndex = 0;
	drawItemLockout = false;
}

//Returns true if the file is an s3g/j4g file
//Keeping this in C instead of C++ saves 20 bytes

bool isSXGFile(char *filename, uint8_t len) {
	if ((len >= 4) && 
	    (filename[len-4] == '.') &&
	    ((filename[len-3] == 's') || (filename[len-3] == 'x')) &&
	    (filename[len-2] == '3') &&
	    (filename[len-1] == 'g')) return true;
	return false;
}

// Count the number of files on the SD card
uint8_t SDMenu::countFiles() {
	uint8_t count = 0;

	// First, reset the directory index
	if ( sdcard::directoryReset() != sdcard::SD_SUCCESS )
		// TODO: Report 
		return 0;

	char fnbuf[SD_MAXFILELENGTH+1];
	uint8_t flen;

	// Count the files
	do {
		bool isdir;
		sdcard::directoryNextEntry(fnbuf,sizeof(fnbuf),&flen,&isdir);
		if ( fnbuf[0] == 0 )
			return count;
		// Count .. and anyfile which doesn't begin with .
		if ( isdir ) {
			if ( fnbuf[0] != '.' || ( fnbuf[1] == '.' && fnbuf[2] == 0 ) ) count++;
		}
		else if ( isSXGFile(fnbuf, flen) ) count++;
	} while (true);

	// Never reached
	return count;
}

bool SDMenu::getFilename(uint8_t index, char buffer[], uint8_t buffer_size, uint8_t *buflen, bool *isdir) {

	*buflen = 0;
	*isdir = false;

	// First, reset the directory list
	if ( sdcard::directoryReset() != sdcard::SD_SUCCESS )
                return false;

	uint8_t my_buflen = 0; // set to zero in case the for loop never runs
	bool my_isdir;

	for(uint8_t i = 0; i < index+1; i++) {
		do {
			sdcard::directoryNextEntry(buffer, buffer_size, &my_buflen, &my_isdir);
			if ( buffer[0] == 0 )
				// No more files
				return false;
			if ( my_isdir ) {
				if ( buffer[0] != '.' || ( buffer[1] == '.' && buffer[2] == 0 ) )
					break;
			}
			else if ( isSXGFile(buffer, my_buflen) )
				break;
		} while (true);
	}

	*isdir  = my_isdir;
	*buflen = my_buflen;

        return true;
}

void SDMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	uint8_t idx, filenameLength;
	uint8_t longFilenameOffset = 0;
	uint8_t displayWidth = LCD_SCREEN_WIDTH - 1;
	uint8_t offset = 1;
	char fnbuf[SD_MAXFILELENGTH+2]; // extra +1 since we may precede the name with a folder indicator
	bool isdir;

	if ( !selectable )
		return;
	else if ( index >= (itemCount - 1) ) {
		lcd.writeFromPgmspace(EXIT_MSG);
		return;
	}

        if ( !getFilename(index, fnbuf + offset, sizeof(fnbuf), &filenameLength, &isdir) ) {
		selectable = false;
		return;
	}

	if ( isdir )
	{
		fnbuf[0] = ( fnbuf[1] == '.' && fnbuf[2] == '.' ) ? LCD_CUSTOM_CHAR_RETURN : LCD_CUSTOM_CHAR_FOLDER;
		offset = 0;
	}

	//Support scrolling filenames that are longer than the lcd screen
	if (filenameLength >= displayWidth) longFilenameOffset = updatePhase % (filenameLength - displayWidth + 1);

	for (idx = 0; (idx < displayWidth) && ((longFilenameOffset + idx) < sizeof(fnbuf)) &&
                        (fnbuf[offset+longFilenameOffset + idx] != 0); idx++)
		lcd.write(fnbuf[offset+longFilenameOffset + idx]);

	//Clear out the rest of the line
	while ( idx < displayWidth ) {
		lcd.write(' ');
		idx++;
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

	if ( selectable ) {
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
	else {
		// This was actually triggered in drawItem() but popping a screen
		// from there is not a good idea
		const prog_uchar *msg;
		if ( sdcard::sdAvailable == sdcard::SD_SUCCESS ) msg = CARDNOFILES_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_NO_CARD_PRESENT ) msg = NOCARD_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_OPEN_FILESYSTEM ) msg = CARDFORMAT_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_VOLUME_TOO_BIG ) msg = CARDSIZE_MSG;
		else msg = CARDERROR_MSG;
		interface::popScreen();
		Motherboard::getBoard().errorResponse(msg);
	}
}

void SDMenu::notifyButtonPressed(ButtonArray::ButtonName button) {
        updatePhase = 0;
	updatePhaseDivisor = 0;
        Menu::notifyButtonPressed(button);
}

void SDMenu::handleSelect(uint8_t index) {
	if ( index >= itemCount - 1 )
	{
		// "Exit Menu" selected
		interface::popScreen();
		return;
	}

	if ( host::getHostState() != host::HOST_STATE_READY ) {
	        Motherboard::getBoard().errorResponse(BUILDING_MSG);
		return;
	}
	else if ( !selectable )
		return;

	char fname[SD_MAXFILELENGTH + 1];
	uint8_t flen;
	bool isdir;

        drawItemLockout = true;

	if ( !getFilename(index, fname, sizeof(fname), &flen, &isdir) )
		goto badness;

	if ( isdir ) {

		// Attempt to change the directory
		if ( !sdcard::changeDirectory(fname) )
			goto badness;

		// Find our way around this folder
		//  Doing a resetState() will determine the new itemCount
		resetState();

		itemIndex = 0;

		// If we're not selectable, don't bother
		if ( selectable ) {
			// Recall last itemIndex in this folder if we popped up
			if ( fname[0] != '.' || fname[1] != '.' || fname[2] != 0 ) {
				// We've moved down into a child folder
				if ( folderStackIndex < (int8_t)(sizeof(folderStack) - 1) )
					folderStack[++folderStackIndex] = index;
			}
			else {
				// We've moved up into our parent folder
				if ( folderStackIndex >= 0 ) {
					itemIndex = folderStack[folderStackIndex--];
					if (itemIndex >= itemCount) {
						// Something is wrong; invalidate the entire stack
						itemIndex = 0;
						folderStackIndex = -1;
					}
				}
			}
		}

		// Repaint the display
		// Really ensure that the entire screen is wiped
		lastDrawIndex = index; // so that the old cursor can be cleared
		Menu::update(Motherboard::getBoard().getInterfaceBoard().lcd, true);

		return;
	}

	// Start a build
	if ( host::startBuildFromSD(fname, flen) == sdcard::SD_SUCCESS )
		return;
badness:
	Motherboard::getBoard().errorResponse(CARDOPENERR_MSG);
}

// Clear the screen, display a message, and then count down from 5 to 0 seconds
//   with a countdown timer to let folks know what's up

static void timedMessage(LiquidCrystalSerial& lcd, uint8_t which)
{
	const static PROGMEM prog_uchar eeprom_msg11[]  = "Write Failed!";
	const static PROGMEM prog_uchar eeprom_msg12[]  = "File exists!";
	const static PROGMEM prog_uchar eeprom_msg5[]   = "Read Failed!";
	const static PROGMEM prog_uchar eeprom_msg6[]   = "EEPROM may be";
	const static PROGMEM prog_uchar eeprom_msg7[]   = "corrupt";
	const static PROGMEM prog_uchar eeprom_msg8[]   = "File not found!";
	const static PROGMEM prog_uchar eeprom_message_erase[] = "Erasing...";
	const static PROGMEM prog_uchar eeprom_message_error[] = "Error";
	const static PROGMEM prog_uchar timed_message_clock[]  = "00:00";

	lcd.clearHomeCursor();
	switch(which) {
	case 1:
		lcd.writeFromPgmspace(eeprom_msg11);
		break;

	case 2:
		lcd.writeFromPgmspace(eeprom_message_error);
		lcd.setRow(1);
		lcd.writeString((char *)dumpFilename);
		lcd.setRow(2);
		lcd.writeFromPgmspace(eeprom_msg12);
		break;

	case 3:
		lcd.writeFromPgmspace(eeprom_msg5);
		lcd.setRow(1);
		lcd.writeFromPgmspace(eeprom_msg6);
		lcd.setRow(2);
		lcd.writeFromPgmspace(eeprom_msg7);
		break;

	case 4:
		lcd.writeFromPgmspace(eeprom_message_error);
		lcd.setRow(1);
		lcd.writeString((char *)dumpFilename);
		lcd.setRow(2);
		lcd.writeFromPgmspace(eeprom_msg8);
		break;

	case 5:
		lcd.writeFromPgmspace(eeprom_message_erase);
		break;
	}

	lcd.setCursor(10, 3);
	lcd.write('0' + sdcard::sdAvailable);
	lcd.setRow(3);
	lcd.writeFromPgmspace(timed_message_clock);
	for (uint8_t i = 0; i < 5; i++)
	{
		lcd.setCursor(4, 3);
		lcd.write('5' - (char)i);
		_delay_us(1000000);
	}
}

#endif
