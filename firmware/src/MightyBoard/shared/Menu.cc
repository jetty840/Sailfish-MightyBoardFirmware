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
#ifdef HAS_RGB_LED
#include "RGB_LED.hh"
#endif
#include "stdio.h"
#include "Piezo.hh"
#include "Menu_locales.hh"
#include "lib_sd/sd_raw_err.h"
#include "Heater.hh" // for MAX_VALID_TEMP

//#define HOST_PACKET_TIMEOUT_MS 20
//#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

//#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
//#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

#define SD_MAXFILELENGTH 64

static uint8_t lastFileIndex = 255;
bool ready_fail = false;
static bool singleTool = false;
static bool hasHBP = true;

const static PROGMEM prog_uchar units_mm[] = "mm";
#define DUMP_FILE "eeprom_dump.bin"
static const char dumpFilename[] = DUMP_FILE;

enum sucessState{
	SUCCESS,
	FAIL,
	SECOND_FAIL
};

uint8_t filamentSuccess;

uint32_t homePosition[PROFILES_HOME_POSITIONS_STORED];

ActiveBuildMenu               activeBuildMenu;
BotStatsScreen                botStatsScreen;
BuildStatsScreen              buildStatsScreen;
CancelBuildMenu               cancelBuildMenu;
ChangeSpeedScreen             changeSpeedScreen;
ChangeTempScreen              changeTempScreen;
FilamentMenu                  filamentMenu;
FilamentOdometerScreen        filamentOdometerScreen;
FilamentScreen                filamentScreen;
HeaterPreheatMenu             heaterPreheatMenu;
HomeOffsetsModeScreen         homeOffsetsModeScreen;
JogModeScreen                 jogModeScreen;
MonitorModeScreen             monitorModeScreen;
PauseAtZPosScreen             pauseAtZPosScreen;
PreheatSettingsMenu           preheatSettingsMenu;
ProfileChangeNameModeScreen   profileChangeNameModeScreen;
ProfileDisplaySettingsMenu    profileDisplaySettingsMenu;
ProfileSubMenu                profileSubMenu;
ProfilesMenu                  profilesMenu;
ResetSettingsMenu             resetSettingsMenu;
SDMenu                        sdMenu;
SettingsMenu                  settingsMenu;
SplashScreen                  splashScreen;
UtilitiesMenu                 utilityMenu;

#ifndef SINGLE_EXTRUDER
#ifdef NOZZLE_CALIBRATION_SCREEN
NozzleCalibrationScreen       nozzleCalibrationScreen;
#endif
SelectAlignmentMenu           selectAlignmentMenu;
#endif

#ifdef EEPROM_MENU_ENABLE
EepromMenu                    eepromMenu;
#endif

/// Static instances of our menus

//Macros to expand SVN revision macro into a str
#define STR_EXPAND(x) #x        //Surround the supplied macro by double quotes
#define STR(x) STR_EXPAND(x)

static void MenuBadness(const prog_uchar *msg)
{
	interface::popScreen();
	Motherboard::getBoard().errorResponse(msg);
}

static uint8_t lastHeatIndex;
static bool toggleBlink;

//Renamed to zabs because of conflict with stdlib.h abs
#define zabs(X) ((X) < 0 ? -(X) : (X))

static void buildInfo(LiquidCrystalSerial& lcd)
{
#ifdef HAS_RGB_LED
	RGB_LED::setDefaultColor();
#endif
	switch(host::getHostState())
	{

	case host::HOST_STATE_READY:
	case host::HOST_STATE_BUILDING_ONBOARD:
		lcd.writeString(host::getMachineName());
		break;

	case host::HOST_STATE_BUILDING:
	case host::HOST_STATE_BUILDING_FROM_SD:
	{
		const char *name = host::buildName;
		uint8_t i = 0;
		while((*name != '.') && (*name != '\0') && (++i <= LCD_SCREEN_WIDTH))
			lcd.write(*name++);
	}
	lcd.moveWriteFromPgmspace(16, 0, BUILD_PERCENT_MSG);
	break;
	
	case host::HOST_STATE_ERROR:
		lcd.writeFromPgmspace(ERROR_MSG);
		break;

	default:
		break;
	}
}

static void progressBar(LiquidCrystalSerial& lcd, int16_t delta, int16_t setTemp, bool blink)
{
	if ( setTemp <= 0 ) return;

	uint16_t currentTemp = zabs(setTemp - delta);
	uint8_t heatIndex = currentTemp * (LCD_SCREEN_WIDTH-HEATING_MSG_LEN) / setTemp;

	if ( heatIndex > (LCD_SCREEN_WIDTH-HEATING_MSG_LEN) )
		// setTemp < currentTemp
		heatIndex = (LCD_SCREEN_WIDTH-HEATING_MSG_LEN);

	if ( lastHeatIndex > heatIndex ) {
		lcd.moveWriteFromPgmspace(HEATING_MSG_LEN, 0,
					  HEATING_SPACES_MSG + HEATING_MSG_LEN);
		lastHeatIndex = 0;
	}

	lcd.setCursor(HEATING_MSG_LEN + lastHeatIndex, 0);
	for ( uint8_t i = lastHeatIndex; i < heatIndex; i++ )
		lcd.write(0xFF);
	lastHeatIndex = heatIndex;

	toggleBlink = !toggleBlink;
	lcd.write(toggleBlink ? ' ' : 0xFF);
}

#ifdef BUILD_STATS

//  Assumes room for up to 7 + NUL
//  99h59m
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
		}
		else
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
	        lcd.moveWriteFromPgmspace(0, 0, SPLASH1_MSG);

#ifdef STACK_PAINT
		if ( hold_on ) {
			lcd.moveWriteFromPgmspace(0, 1, CLEAR_MSG);
			lcd.setRow(1);
               		lcd.writeFromPgmspace(SPLASH_SRAM_MSG);
                	lcd.writeFloat((float)StackCount(), 0, LCD_SCREEN_WIDTH);
		}
		else
			lcd.moveWriteFromPgmspace(0, 1, SPLASH2_MSG);
#else
		lcd.moveWriteFromPgmspace(0, 1 SPLASH2_MSG);
#endif
		lcd.moveWriteFromPgmspace(0, 2, SPLASH3_MSG);
		lcd.moveWriteFromPgmspace(0, 3, SPLASH4_MSG);
	}
	else if ( !hold_on )
		//	 The machine has started, so we're done!
                interface::popScreen();
}

void SplashScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	// We can't really do anything, since the machine is still loading, so ignore.
	switch (button) {
	case ButtonArray::CENTER:
        case ButtonArray::LEFT:
		interface::popScreen();
		hold_on = false;
		break;
	default:
		break;
	}
}

void SplashScreen::reset() {
}

HeaterPreheatMenu::HeaterPreheatMenu() :
	Menu(0, (uint8_t)4) {
	reset();
}

void HeaterPreheatMenu::resetState(){
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

void HeaterPreheatMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;
	bool test;

	switch (index) {
	default:
		return;
	case 0:
		lcd.writeFromPgmspace(preheatActive ? STOP_MSG : GO_MSG);
		return;
	case 1:
		msg = singleTool ? TOOL_MSG : RIGHT_SPACES_MSG;
		test = _rightActive;
		break;
	case 2:
		if ( !singleTool ) {
			msg = LEFT_SPACES_MSG;
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
	lcd.moveWriteFromPgmspace(16, index, test ? ON_MSG : OFF_MSG);
}

void HeaterPreheatMenu::storeHeatByte() {
	uint8_t heatByte = (_rightActive*(1<<HEAT_MASK_RIGHT)) + (_leftActive*(1<<HEAT_MASK_LEFT)) + (_platformActive*(1<<HEAT_MASK_PLATFORM));
	eeprom_write_byte((uint8_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET), heatByte);
}

void HeaterPreheatMenu::handleSelect(uint8_t index) {
	int temp;

	switch (index) {
	case 0:
		preheatActive = !preheatActive;
		// clear paused state if any
		Motherboard::pauseHeaters(false);
		if ( preheatActive ) {
			Motherboard::getBoard().resetUserInputTimeout();
			temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP, DEFAULT_PREHEAT_TEMP) *_rightActive;
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
			if ( !singleTool ) {
				temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP, DEFAULT_PREHEAT_TEMP) *_leftActive;
				Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
			}
			if ( hasHBP ) {
				temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP, DEFAULT_PREHEAT_HBP) *_platformActive;
				Motherboard::getBoard().getPlatformHeater().set_target_temperature(temp);
			}
#if !defined(HEATERS_ON_STEROIDS)
			if ( Motherboard::getBoard().getPlatformHeater().isHeating() )
				Motherboard::pauseHeaters(true);
#endif
			if ( _platformActive || _rightActive || _leftActive ) {
				BOARD_STATUS_SET(Motherboard::STATUS_PREHEATING);
			}
		}
		else
			// Note heatersOff() clears STATUS_PREHEATING
			Motherboard::heatersOff(true);
			
		interface::popScreen();
		interface::pushScreen(&monitorModeScreen);
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

#ifndef SINGLE_EXTRUDER
#ifdef NOZZLE_CALIBRATION_SCRIPT

void NozzleCalibrationScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if ( forceRedraw || needsRedraw ) {
		const prog_uchar *msg;
		needsRedraw = false;
		lcd.setRow(0);
		switch (alignmentState) {
		default:
			return;
		case ALIGNMENT_START:
			// Make sure that the toolhead system is NEW
			if (0 == eeprom::getEeprom8(eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM, DEFAULT_TOOLHEAD_OFFSET_SYSTEM)) {
				MenuBadness(NOZZLE_ERROR_MSG);
				return;
			}
			msg = START_TEST_MSG;
			break;
		case ALIGNMENT_EXPLAIN1:
			msg = EXPLAIN1_MSG;
			break;
		case ALIGNMENT_EXPLAIN2:
			msg = EXPLAIN2_MSG;
			break;
		case ALIGNMENT_SELECT:
			Motherboard::interfaceBlinkOff();
			interface::pushScreen(&selectAlignmentMenu);
			alignmentState++;
			return;
		case ALIGNMENT_END:
			msg = END_MSG;
			break;
		}
		lcd.writeFromPgmspace(msg);
		_delay_us(500000);
		Motherboard::interfaceBlinkOn();
	}
}

void NozzleCalibrationScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
	case ButtonArray::CENTER:
		alignmentState++;
		switch (alignmentState) {
                case ALIGNMENT_PRINT:
			Motherboard::interfaceBlinkOff();
			host::startOnboardBuild(utility::TOOLHEAD_CALIBRATE);
			alignmentState++;
			break;
                case ALIGNMENT_QUIT:
			Motherboard::interfaceBlinkOff();
			interface::popScreen();
			break;
                default:
			needsRedraw = true;
			break;
		}
		break;
        case ButtonArray::LEFT:
		cancelBuildMenu.state = 0;
		interface::pushScreen(&cancelBuildMenu);
		break;
	default:
		break;
	}
}

void NozzleCalibrationScreen::reset() {
	needsRedraw = false;
	Motherboard::interfaceBlinkOn();
	alignmentState = ALIGNMENT_START;
}

#endif // NOZZLE_CALIBRATION_SCRIPT

SelectAlignmentMenu::SelectAlignmentMenu() :
	CounterMenu(0, (uint8_t)4) {
	reset();
}

void SelectAlignmentMenu::resetState() {
	itemIndex       = 1;
	firstItemIndex  = 1;
	lastSelectIndex = 2;
	counter[0] = 7;
	counter[1] = 7;
	offset[0]  = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0));
	offset[1]  = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + sizeof(int32_t), 0));
	smallOffsets = abs(offset[0]) < ((int32_t)stepperAxisStepsPerMM(0) << 2);
}

void SelectAlignmentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	switch (index) {
	case 0:
		lcd.writeFromPgmspace(SELECT_MSG);
		break;
        case 1:
	case 2:
		lcd.writeFromPgmspace((index == 1) ? XAXIS_MSG : YAXIS_MSG);
		lcd.setCursor(15, index);
		lcd.write((selectIndex == index) ? LCD_CUSTOM_CHAR_RIGHT : ' ');
		lcd.setCursor(17, index);
		lcd.writeInt(counter[index-1], 2);
		break;
	case 3:
		lcd.writeFromPgmspace(DONE_MSG);
		break;
 	}
}

void SelectAlignmentMenu::handleCounterUpdate(uint8_t index, int8_t up) {
	if ( index != 1 && index != 2 )
		return;
	--index;
	counter[index] += up;
	if ( counter[index] > 13 ) counter[index] = 13;
	else if ( counter[index] < 1 ) counter[index] = 1;
}

void SelectAlignmentMenu::handleSelect(uint8_t index) {
	if ( index != 1 && index != 2 ) {
		interface::popScreen();
		return;
	}

	--index;
	int32_t delta = stepperAxisMMToSteps((float)(counter[index] - 7) * 0.1f, index);

	// We need to know what the old X toolhead offset is so that we can judge if it
	//   is stored using the OLD system in which a negative value means wider than 33 mm
	// We cannot tell from the old Y toolhead offset since they are always close to zero
	if ( !smallOffsets ) delta = -delta;

	int32_t new_offset = offset[index] + delta;
	eeprom_write_block((uint8_t *)&new_offset,
			   (uint8_t *)eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + index * sizeof(int32_t),
			   sizeof(int32_t));
	lineUpdate = 1;
}

#endif // !SINGLE_EXTRUDER

void FilamentScreen::startMotor(){
	Piezo::playTune(TUNE_FILAMENT_START);

	//So we don't prime after a pause
	command::pauseUnRetractClear();

	int32_t interval = 300000000;  // 5 minutes
	int32_t steps = interval / 3250;
	if ( forward ) steps *= -1;
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

	steppers::setTargetNew(target, 0, interval, 0x1f);
	filamentTimer.clear();
	filamentTimer.start(300000000); //5 minutes
}

void FilamentScreen::stopMotor(){
	steppers::abort();
	steppers::deprimeEnable(true);

	//Restore the enabled axis
	for (uint8_t i = 0; i < STEPPER_COUNT; i++)
		steppers::enableAxis(i, restoreAxesEnabled & _BV(i));

	//Restore the digi pot setting from entry
	steppers::setAxisPotValue(axisID, digiPotOnEntry);
}

void FilamentScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
    	if ( filamentState == FILAMENT_WAIT ) {

		/// if extruder has reached hot temperature, start extruding
		if ( Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().has_reached_target_temperature() ) {

			int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			/// check for externally manipulated temperature (eg by RepG)
			if ( setTemp < filamentTemp[toolID] ) {
				BOARD_STATUS_CLEAR(Motherboard::STATUS_ONBOARD_PROCESS);
				Motherboard::heatersOff(false);
				MenuBadness(EXTEMP_CHANGE_MSG);
				return;
			}

			filamentState++;
			needsRedraw= true;
#ifdef HAS_RGB_LED
			RGB_LED::setDefaultColor();
#endif
			startMotor();
			filamentState = FILAMENT_STOP;
		}
		/// if heating timer has eleapsed, alert user that the heater is not getting hot as expected
		else if (filamentTimer.hasElapsed()){
			lcd.clearHomeCursor();
			lcd.writeFromPgmspace(HEATER_ERROR_MSG);
			Motherboard::interfaceBlinkOn();
			filamentState = FILAMENT_DONE;
		}
		/// if extruder is still heating, update heating bar status
		else {
			int16_t currentDelta = Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().getDelta();
			int16_t setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			// check for externally manipulated temperature (eg by RepG)
			if ( setTemp < filamentTemp[toolID] ) {
				Motherboard::heatersOff(false);
				BOARD_STATUS_CLEAR(Motherboard::STATUS_ONBOARD_PROCESS);
				MenuBadness(EXTEMP_CHANGE_MSG);
				return;
			}

			progressBar(lcd, currentDelta, setTemp, toggleBlink);
		}
	}
	/// if not in FILAMENT_WAIT state and the motor times out (5 minutes) alert the user
	else if ( filamentTimer.hasElapsed() ) {
		filamentState = FILAMENT_TIMEOUT;
		filamentTimer = Timeout();
		needsRedraw = true;
	}

	if (forceRedraw || needsRedraw) {
		//	waiting = true;
		lcd.clearHomeCursor();
		lastHeatIndex = 0;
		uint16_t offset;
		BOARD_STATUS_SET(Motherboard::STATUS_ONBOARD_PROCESS);
		switch (filamentState){
			/// starting state - set hot temperature for desired tool and start heat up timer
		case FILAMENT_HEATING:
		{
			offset = (toolID == 0) ?
				preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP : preheat_eeprom_offsets::PREHEAT_LEFT_TEMP;
			int16_t preheatTemp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + offset, DEFAULT_PREHEAT_TEMP);
			setTemp = (int16_t)(Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().get_set_temperature());
			// If the tool is already set to a temp > preheat temp, then use it
			filamentTemp[toolID] = ( preheatTemp >= setTemp ) ? preheatTemp : setTemp;
			Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().Pause(false);
			Motherboard::getBoard().getExtruderBoard(toolID).getExtruderHeater().set_target_temperature(filamentTemp[toolID]);
			lcd.writeFromPgmspace(HEATING_MSG);
			lastHeatIndex = 0;
			filamentState = FILAMENT_WAIT;
			filamentTimer.clear();
			filamentTimer.start(300000000); //5 minutes
			break;
		}
			/// show heating bar status
		case FILAMENT_WAIT:
			lcd.writeFromPgmspace(HEATING_MSG);
			break;
			/// alert user that filament is ready to extrude
			/// alert user to press M to stop extusion / reversal
		case FILAMENT_STOP:
			lcd.writeFromPgmspace(STOP_EXIT_MSG);
			Motherboard::interfaceBlinkOn();
			_delay_us(1000000);
			break;
		case FILAMENT_DONE:
			/// user indicated that filament has extruded
			stopMotor();
			Motherboard::interfaceBlinkOn();
			_delay_us(1000000);
			break;
		case FILAMENT_TIMEOUT:
			/// filament motor has been running for 5 minutes
			stopMotor();
			lcd.writeFromPgmspace(TIMEOUT_MSG);
			filamentState = FILAMENT_DONE;
			Motherboard::interfaceBlinkOn();
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
		if ( filamentState == FILAMENT_WAIT )
			break;
		filamentState++;
		Motherboard::interfaceBlinkOff();
		switch (filamentState){
			/// go to interactive 'OK' scrreen
		case FILAMENT_OK:
		case FILAMENT_EXIT:
			stopMotor();
			if ( leaveHeatOn == 0 || setTemp == 0)
			    Motherboard::heatersOff(false);
			BOARD_STATUS_CLEAR(Motherboard::STATUS_ONBOARD_PROCESS);
			interface::popScreen();
			break;
		default:
			needsRedraw = true;
			break;
		}
		break;
        case ButtonArray::LEFT:
		// Needs to be set from our caller (Utility vs. ActiveBuild)
		// cancelBuildMenu.state = 1;
		interface::pushScreen(&cancelBuildMenu);
		break;
        default:
                break;
	}
}

void FilamentScreen::reset() {
	setTemp = 0;
	needsRedraw = false;
	toggleBlink = false;
	lastHeatIndex = 0;
	filamentState = FILAMENT_HEATING;
	filamentSuccess = SUCCESS;
	filamentTimer = Timeout();
	for (uint8_t i = 0; i < EXTRUDERS; i++)
		filamentTemp[i] = DEFAULT_PREHEAT_TEMP;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"

FilamentMenu::FilamentMenu() : Menu(optionsMask, (uint8_t)4) {
	reset();
}

#pragma GCC diagnostic pop

void FilamentMenu::resetState() {
	singleTool = eeprom::isSingleTool();
	itemCount = ( singleTool ) ? 2 : 4;
	itemIndex = 0;
	firstItemIndex = 0;
}

void FilamentMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	const prog_uchar *msg;

	switch (index) {
	default:
		return;
	case 0:
		msg = singleTool ? UNLOAD_SINGLE_MSG : UNLOAD_RIGHT_MSG;
		break;
	case 1:
		msg = singleTool ? LOAD_SINGLE_MSG : LOAD_RIGHT_MSG;
		break;
	case 2:
		if ( singleTool )
			return;
		msg = UNLOAD_LEFT_MSG;
		break;
	case 3:
		if ( singleTool )
			return;
		msg = LOAD_LEFT_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
}

void FilamentMenu::handleSelect(uint8_t index) {
	FilamentScript script;

	switch (index) {
	default:
		return;
        case 0:
		script = FILAMENT_RIGHT_REV;
		break;
        case 1:
		script = FILAMENT_RIGHT_FOR;
		break;
        case 2:
		script = FILAMENT_LEFT_REV;
		break;
        case 3:
		script = FILAMENT_LEFT_FOR;
		break;
	}
	filamentScreen.setScript(script);
	interface::pushScreen(&filamentScreen);
}



bool MessageScreen::screenWaiting(void){
	return (timeout.isActive() || incomplete);
}

void MessageScreen::addMessage(CircularBuffer& buf) {
	char c = buf.pop();
	while (c != '\0' && buf.getLength() > 0) {
		if ( cursor < BUF_SIZE ) message[cursor++] = c;
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
	incomplete = false;

	host::HostState state;
	switch (button) {
	case ButtonArray::CENTER:
		break;
        case ButtonArray::LEFT:
		state = host::getHostState();
		if ( (state == host::HOST_STATE_BUILDING_ONBOARD) ||
		     (state == host::HOST_STATE_BUILDING) ||
		     (state == host::HOST_STATE_BUILDING_FROM_SD) ) {
			cancelBuildMenu.state = 0;
			interface::pushScreen(&cancelBuildMenu);
                }
        default:
                break;
	}
}

void JogModeScreen::reset() {
	jogDistance = DISTANCE_CONT;
	jogging = false;
	distanceChanged = modeChanged = false;
	JogModeScreen = JOG_MODE_X;
	for (uint8_t i = 0; i < 3; i++) {
	    digiPotOnEntry[i] = steppers::getAxisPotValue(i);
	    steppers::resetAxisPot(i);
	}
}


void JogModeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	//Stop all movement when a button is released
	if ((jogging) && (!interface::isButtonPressed(ButtonArray::DOWN)) &&
	    (!interface::isButtonPressed(ButtonArray::UP)))
	{
		jogging = false;
		steppers::abort();
	}

	if (forceRedraw || distanceChanged || modeChanged) {

		BOARD_STATUS_SET(Motherboard::STATUS_MANUAL_MODE);

		distanceChanged = false;
		modeChanged = false;

		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(JOG1_MSG);

		const prog_uchar *msg;
		switch (JogModeScreen){
		default:
			return;
		case JOG_MODE_X:
			msg = JOG3X_MSG;
			break;
		case JOG_MODE_Y:
			msg = JOG3Y_MSG;
			break;
		case JOG_MODE_Z:
			msg = JOG3Z_MSG;
			break;
		}
		lcd.moveWriteFromPgmspace(0, 1, JOG2X_MSG);
		lcd.setCursor(8, 1);
		lcd.write('X' + (JogModeScreen - JOG_MODE_X) );
		if ( JogModeScreen == JOG_MODE_Z ) lcd.write('-');

		lcd.moveWriteFromPgmspace(0, 2, msg);

		lcd.moveWriteFromPgmspace(0, 3, JOG4X_MSG);
		lcd.setCursor(8, 3);
		lcd.write('X' + (JogModeScreen - JOG_MODE_X) );
		if ( JogModeScreen == JOG_MODE_Z ) lcd.write('+');
	}
}

void JogModeScreen::jog(ButtonArray::ButtonName direction) {
	steppers::abort();
	uint8_t dummy;
	Point position = steppers::getStepperPosition(&dummy);

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

	if ( JogModeScreen == JOG_MODE_X ) {
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
	else if ( JogModeScreen == JOG_MODE_Y ) {
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
		steppers::setTargetNew(position, interval, 0, 0);
}

void JogModeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
	case ButtonArray::CENTER:
	        for (uint8_t i=0; i < 3; i++)
		    steppers::setAxisPotValue(i, digiPotOnEntry[i]);
		steppers::enableAxes(0xff, false);
		BOARD_STATUS_CLEAR(Motherboard::STATUS_MANUAL_MODE);
		interface::popScreen();
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


// Print the last build time
void printLastBuildTime(const prog_uchar *msg, uint8_t row, LiquidCrystalSerial& lcd)
{
	lcd.writeFromPgmspace(msg);

	uint8_t build_hours;
	uint8_t build_minutes;
	host::getPrintTime(build_hours, build_minutes);

	uint8_t digits = 1;
	for (uint16_t i = 10; i < 100000; i *= 10) {
		if ( i > (uint16_t)build_hours ) break;
		digits++;
	}

	lcd.setCursor(15 - digits, row);
	lcd.writeInt(build_hours, digits);

	lcd.setCursor(17, row);
	lcd.writeInt(build_minutes, 2);
}

// Print the filament used, right justified.  Written in C to save space as it's
// used 3 times.  Takes filamentUsed in millimeters

void printFilamentUsed(float filamentUsed, LiquidCrystalSerial& lcd) {
	uint8_t precision;

	filamentUsed /= 1000.0; //convert to meters
	if      ( filamentUsed < 0.1 )  {
		filamentUsed *= 1000.0;        //Back to mm's
		precision = 1;
	}
	else if ( filamentUsed < 10.0 )  precision = 4;
	else if ( filamentUsed < 100.0 ) precision = 3;
	else                             precision = 2;

	lcd.writeFloat(filamentUsed, precision, LCD_SCREEN_WIDTH - ((precision == 1) ? 2 : 1));
	lcd.writeFromPgmspace((precision == 1) ? MILLIMETERS_MSG : METERS_MSG);
}

void filamentOdometers(bool odo, uint8_t yOffset, LiquidCrystalSerial &lcd) {

	// Get lifetime filament used for A & B axis and sum them
	// into filamentUsed
	lcd.moveWriteFromPgmspace(0, yOffset, odo ? FILAMENT_LIFETIME1_MSG : FILAMENT_LIFETIME2_MSG);

	float filamentUsedA, filamentUsedB;
	filamentUsedA = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0),                  A_AXIS);
	filamentUsedB = stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t),0), B_AXIS);
	printFilamentUsed(filamentUsedA + filamentUsedB, lcd);

	// Get trip filament used for A & B axis and sum them into filamentUsed
	lcd.moveWriteFromPgmspace(0, ++yOffset, odo ? FILAMENT_TRIP1_MSG : FILAMENT_TRIP2_MSG);

	filamentUsedA -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP, 0),                  A_AXIS);
	filamentUsedB -= stepperAxisStepsToMM(eeprom::getEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t),0), B_AXIS);
	printFilamentUsed(filamentUsedA + filamentUsedB, lcd);
}

void FilamentOdometerScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw || needsRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(FILAMENT_ODOMETER_MSG);
		filamentOdometers(true, 1, lcd);
		lcd.moveWriteFromPgmspace(0, 3, FILAMENT_RESET_TRIP_MSG);
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

void MonitorModeScreen::reset() {
	updatePhase = 0;
	singleTool = eeprom::isSingleTool();
	hasHBP = eeprom::hasHBP();
	toggleBlink = false;
	heating = false;
	lastHeatIndex = 0;
#ifdef BUILD_STATS
	buildTimePhase = BUILD_TIME_PHASE_FIRST;
	lastBuildTimePhase = BUILD_TIME_PHASE_FIRST;
	lastElapsedSeconds = 0.0;
#endif
}

void MonitorModeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
#ifdef BUILD_STATS
	const static PROGMEM prog_uchar mon_elapsed_time[]       = "Elapsed:       0h00m";
	const static PROGMEM prog_uchar mon_time_left[]          = "Time Left:     0h00m";
	const static PROGMEM prog_uchar mon_time_left_secs[]     = "secs";
	const static PROGMEM prog_uchar mon_time_left_none[]     = "   none";
	const static PROGMEM prog_uchar mon_zpos[] 	             = "ZPos:               ";
	const static PROGMEM prog_uchar mon_filament[]           = "Filament:      0.00m";
#ifdef ACCEL_STATS
	const static PROGMEM prog_uchar mon_speed[] 	     = "Acc:                ";
#endif
#endif
	Motherboard& board = Motherboard::getBoard();

	if ( !heating ) {
		if  (board.getExtruderBoard(0).getExtruderHeater().isHeating() ||
		     board.getExtruderBoard(1).getExtruderHeater().isHeating() ||
		     board.getPlatformHeater().isHeating() ) {
			heating = true;
			lastHeatIndex = 0;
			lcd.setRow(0);
			lcd.writeFromPgmspace(HEATING_SPACES_MSG);
		}
	}

	if (forceRedraw) {

		lcd.clearHomeCursor();
		if ( heating ) {
			lcd.writeFromPgmspace(HEATING_MSG);
			lastHeatIndex = 0;
		}
		else {
			buildInfo(lcd);
		}

		uint8_t row;
		if ( hasHBP ) {
			lcd.moveWriteFromPgmspace(0, 3, PLATFORM_TEMP_MSG);
			row = 2;
		}
		else row = 3;

		if ( singleTool )
			lcd.moveWriteFromPgmspace(0, row--, EXTRUDER_TEMP_MSG);
		else {
			lcd.moveWriteFromPgmspace(0, row--, EXTRUDER2_TEMP_MSG);
			lcd.moveWriteFromPgmspace(0, row--, EXTRUDER1_TEMP_MSG);
		}
		while (row >= 1)
			lcd.moveWriteFromPgmspace(0, row--, CLEAR_MSG);
	}

	OutPacket responsePacket;
	uint16_t data;
	host::HostState state;
	int16_t currentDelta = 0;
	int16_t setTemp = 0;

	/// show heating progress
	if ( heating ) {
		if (board.getExtruderBoard(0).getExtruderHeater().isHeating()  && !board.getExtruderBoard(0).getExtruderHeater().isPaused()){
			currentDelta += board.getExtruderBoard(0).getExtruderHeater().getDelta();
			setTemp += (int16_t)(board.getExtruderBoard(0).getExtruderHeater().get_set_temperature());
		}
		if ( board.getExtruderBoard(1).getExtruderHeater().isHeating() && !board.getExtruderBoard(1).getExtruderHeater().isPaused() ) {
			currentDelta += board.getExtruderBoard(1).getExtruderHeater().getDelta();
			setTemp += (int16_t)(board.getExtruderBoard(1).getExtruderHeater().get_set_temperature());
		}
		if ( board.getPlatformHeater().isHeating() ) {
			currentDelta += board.getPlatformHeater().getDelta()*2;
			setTemp += (int16_t)(board.getPlatformHeater().get_set_temperature())*2;
		}

		if ( currentDelta == 0 ) {
			heating = false;
			//redraw build name
			lcd.moveWriteFromPgmspace(0, 0, CLEAR_MSG);
			lcd.setRow(0);
			buildInfo(lcd);
		}
		else {
			progressBar(lcd, currentDelta, setTemp, toggleBlink);
		}
	}

	// Redraw tool info
	switch (updatePhase) {

	// Dual extruder Tool 0 current temp
	case 0:
		if ( !singleTool ) {
			lcd.setCursor(12, hasHBP ? 1 : 2);
			data = board.getExtruderBoard(0).getExtruderHeater().get_current_temperature();
			if ( board.getExtruderBoard(0).getExtruderHeater().has_failed() || data >= BAD_TEMPERATURE )
				lcd.writeFromPgmspace(NA_MSG);
			else if ( board.getExtruderBoard(0).getExtruderHeater().isPaused() )
				lcd.writeFromPgmspace(WAITING_MSG);
			else
				lcd.writeInt(data, 3);
		}
		break;

	// Dual extruder Tool 0 set temp
	case 1:
		if ( !singleTool ) {
			if( !board.getExtruderBoard(0).getExtruderHeater().has_failed() &&
			    !board.getExtruderBoard(0).getExtruderHeater().isPaused() ) {
				uint8_t row = hasHBP ? 1 : 2;
				data = board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
				if ( data > 0 ) {
					lcd.moveWriteFromPgmspace(15, row, ON_CELCIUS_MSG);
					lcd.setCursor(16, row);
					lcd.writeInt(data, 3);
				}
				else
					lcd.moveWriteFromPgmspace(15, row, CELCIUS_MSG);
			}
		}
		break;

	// Dual extruder Tool 1 current temp
	// Sngl extruder Tool 0 current temp
	case 2:
	{
		lcd.setCursor(12, hasHBP ? 2 : 3);
		uint8_t tool = singleTool ? 0 : 1;
		data = board.getExtruderBoard(tool).getExtruderHeater().get_current_temperature();
		if ( board.getExtruderBoard(tool).getExtruderHeater().has_failed() || data >= BAD_TEMPERATURE )
			lcd.writeFromPgmspace(NA_MSG);
		else if ( board.getExtruderBoard(tool).getExtruderHeater().isPaused() )
			lcd.writeFromPgmspace(WAITING_MSG);
		else
			lcd.writeInt(data, 3);
		break;
	}

	// Dual extruder Tool 1 set temp
	// Sngl extruder Tool 0 set temp
	case 3:
	{
		uint8_t tool = singleTool ? 0 : 1;
		if ( !board.getExtruderBoard(tool).getExtruderHeater().has_failed() &&
		     !board.getExtruderBoard(tool).getExtruderHeater().isPaused() ) {
			uint8_t row = hasHBP ? 2 : 3;
			data = board.getExtruderBoard(tool).getExtruderHeater().get_set_temperature();
			if( data > 0 ) {
				lcd.moveWriteFromPgmspace(15, row, ON_CELCIUS_MSG);
				lcd.setCursor(16, row);
				lcd.writeInt(data, 3);
			}
			else
				lcd.moveWriteFromPgmspace(15, row, CELCIUS_MSG);
		}
		break;
	}

	// HBP current temp
	case 4:
		if ( hasHBP ) {
			data = board.getPlatformHeater().get_current_temperature();
			if ( board.getPlatformHeater().has_failed() || data >= BAD_TEMPERATURE )
				lcd.moveWriteFromPgmspace(12, 3, NA_MSG);
			else if ( board.getPlatformHeater().isPaused() )
				lcd.moveWriteFromPgmspace(12, 3, WAITING_MSG);
			else {
				lcd.setCursor(12, 3);
				lcd.writeInt(data, 3);
			}
		}
		break;

	// HBP set temp
	case 5:
		if ( hasHBP ) {
			if ( !board.getPlatformHeater().has_failed() &&
			     !board.getPlatformHeater().isPaused() ) {
				data = board.getPlatformHeater().get_set_temperature();
				if ( data > 0 ) {
					lcd.moveWriteFromPgmspace(15, 3, ON_CELCIUS_MSG);
					lcd.setCursor(16, 3);
					lcd.writeInt(data, 3);
				}
				else
					lcd.moveWriteFromPgmspace(15, 3, CELCIUS_MSG);
			}
		}
		break;

	// Build %
	case 6:
		state = host::getHostState();
		if ( !heating && ((state == host::HOST_STATE_BUILDING) || (state == host::HOST_STATE_BUILDING_FROM_SD)) ) {

			uint8_t buildPercentage = command::getBuildPercentage();

			if ( buildPercentage < 100 ) { 
				if ( command::getPauseAtZPos() != 0 ) {
					lcd.setCursor(16,0);
					lcd.write('*');
				}
				lcd.setCursor(17,0);
				lcd.writeInt(buildPercentage,2);
			}
			else if ( buildPercentage == 100 ) {
				if ( command::getPauseAtZPos() != 0 ) {
					lcd.setCursor(15,0);
					lcd.write('*');
				}
				lcd.moveWriteFromPgmspace(16, 0, DONE_MSG);
			}
		}
		break;

#ifdef BUILD_STATS
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
		float filamentUsed;

		switch (buildTimePhase) {

		case BUILD_TIME_PHASE_ELAPSED_TIME:
			lcd.moveWriteFromPgmspace(0, 1, mon_elapsed_time);
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
				lcd.moveWriteFromPgmspace(0, 1, mon_time_left);
				lcd.setCursor(13,1);
				if ( (tsecs > 0 ) && (tsecs < 60) && ( host::isBuildComplete() ) ) {
					digits3(buf, (uint8_t)tsecs);
					lcd.writeString(buf);
					lcd.writeFromPgmspace(mon_time_left_secs);
				}
				else if (( tsecs <= 0) || ( host::isBuildComplete()) ) {
					command::addFilamentUsed();
					lcd.writeFromPgmspace(mon_time_left_none);
				}
				else {
					formatTime(buf, (uint32_t)tsecs);
					lcd.writeString(buf);
				}
				break;
			}
			//We can't display the time left, so we drop into ZPosition instead
			else	buildTimePhase = (enum BuildTimePhase)((uint8_t)buildTimePhase + 1);

		case BUILD_TIME_PHASE_ZPOS:
			lcd.moveWriteFromPgmspace(0, 1, mon_zpos);
			{
				uint8_t dummy;
				position = steppers::getStepperPosition(&dummy);
			}
			// if the position is < -8000, we likely haven't yet defined our Z position
			if (position[Z_AXIS] > -8000) {
				lcd.setCursor(6,1);

				//Divide by the axis steps to mm's
				lcd.writeFloat(stepperAxisStepsToMM(position[2], Z_AXIS), 3, LCD_SCREEN_WIDTH-2);
				lcd.writeFromPgmspace(units_mm);
			}
			break;

		case BUILD_TIME_PHASE_FILAMENT:
			lcd.moveWriteFromPgmspace(0, 1, mon_filament);
			lcd.setCursor(9,1);
			filamentUsed = command::filamentUsed();
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
			lcd.moveWriteFromPgmspace(0, 1, mon_speed);
			lcd.setCursor(4,1);
			if ( minSpeed < 100.0 )	lcd.write(' ');	//If we have space, pad out a bit
			lcd.writeFloat(minSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write('/');
			lcd.writeFloat(avgSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write('/');
			lcd.writeFloat(maxSpeed, 0, LCD_SCREEN_WIDTH);
			lcd.write(' ');
			break;
#endif // ACCEL_STATS
		}

        	if ( ! okButtonHeld ) {
			//Advance buildTimePhase and wrap around
			lastBuildTimePhase = buildTimePhase;
			buildTimePhase = (enum BuildTimePhase)((uint8_t)buildTimePhase + 1);

			if ( buildTimePhase >= BUILD_TIME_PHASE_LAST )
				buildTimePhase = BUILD_TIME_PHASE_FIRST;
		}
		break;
#endif // BUILD_STATS
	}

#ifdef BUILD_STATS
	if (++updatePhase > 7)
#else
	if (++updatePhase > 6)
#endif
		updatePhase = 0;

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

void MonitorModeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	switch (button) {
	case ButtonArray::CENTER:
	case ButtonArray::LEFT:
		switch(host::getHostState()) {
		case host::HOST_STATE_BUILDING:
		case host::HOST_STATE_BUILDING_FROM_SD:
			interface::pushScreen(&activeBuildMenu);
			break;
		case host::HOST_STATE_BUILDING_ONBOARD:
			cancelBuildMenu.state = 0;
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

PreheatSettingsMenu::PreheatSettingsMenu() :
	CounterMenu(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)4) {
	reset();
}

void PreheatSettingsMenu::resetState() {
	counterRight = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP, DEFAULT_PREHEAT_TEMP);
	counterLeft = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP, DEFAULT_PREHEAT_TEMP);
	counterPlatform = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP, DEFAULT_PREHEAT_HBP);
	singleTool = eeprom::isSingleTool();
	hasHBP = eeprom::hasHBP();
	offset = 0;
	if ( singleTool ) offset++;
	if ( !hasHBP ) offset++;
	itemIndex = firstItemIndex = 1 + offset;
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
		if ( counterRight > MAX_VALID_TEMP )
			counterRight = MAX_VALID_TEMP;
		break;
	case 2:
		if ( !singleTool ) {
			// update left counter
			counterLeft += up;
			if ( counterLeft > MAX_VALID_TEMP )
				counterLeft = MAX_VALID_TEMP;
		}
		else if ( hasHBP ) {
			// update platform counter
			counterPlatform += up;
			if (counterPlatform > MAX_HBP_TEMP )
				counterPlatform = MAX_HBP_TEMP;
		}
		break;
	case 3:
		// update platform counter
		if ( !singleTool && hasHBP ) {
			counterPlatform += up;
			if ( counterPlatform > MAX_HBP_TEMP )
				counterPlatform = MAX_HBP_TEMP;
		}
		break;
	}
}

void PreheatSettingsMenu::handleSelect(uint8_t index) {
	if  ( index < firstItemIndex )
		return;
	index -= offset;

	switch (index) {
	case 0:
		break;
	case 1:
		// store right tool setting
		eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP), counterRight);
		break;
	case 2:
		if ( !singleTool )
			// store left tool setting
			eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP), counterLeft);
		else if ( hasHBP )
			eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP), counterPlatform);
		break;
	case 3:
		if ( !singleTool && hasHBP )
			// store platform setting
			eeprom_write_word((uint16_t*)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP), counterPlatform);
		break;
	}
}


ResetSettingsMenu::ResetSettingsMenu() :
	Menu(0, (uint8_t)4) {
	reset();
}

void ResetSettingsMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
}

void ResetSettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;
	switch (index) {
	default:
		return;
	case 0:
		msg = RESET1_MSG;
		break;
	case 1:
		msg = RESET2_MSG;
		break;
	case 2:
		msg = NO_MSG;
		break;
	case 3:
		msg = YES_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
}

void ResetSettingsMenu::handleSelect(uint8_t index) {
	if ( index == 3 ) {
		// Reset setings to defaults
		eeprom::factoryResetEEPROM();
		Motherboard::getBoard().reset(false);
	}
	else
		interface::popScreen();
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

ProfilesMenu::ProfilesMenu() :
	Menu((uint8_t)0, (uint8_t)PROFILES_QUANTITY) {
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

ProfileSubMenu::ProfileSubMenu() :
	Menu(0, (uint8_t)4) {
	reset();
}

void ProfileSubMenu::resetState() {
	itemIndex = 0;
	firstItemIndex = 0;
}

void ProfileSubMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;
	switch (index) {
	default:
		return;
	case 0:
		msg = PROFILE_RESTORE_MSG;
		break;
	case 1:
		msg = PROFILE_DISPLAY_CONFIG_MSG;
		break;
	case 2:
		msg = PROFILE_CHANGE_NAME_MSG;
		break;
	case 3:
		msg = PROFILE_SAVE_TO_PROFILE_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
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
		eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP),    rightTemp);
		eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP),     leftTemp);
		eeprom_write_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP), hbpTemp);
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
		profileChangeNameModeScreen.profileIndex = profileIndex;
		interface::pushScreen(&profileChangeNameModeScreen);
		break;
	case 3: //Save To Profile
		//Get the home axis positions
		cli();
		eeprom_read_block((void *)homePosition,(void *)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));
		rightTemp = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP));
		leftTemp  = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP));
		hbpTemp   = eeprom_read_word((uint16_t *)(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP));
		sei();

		writeProfileToEeprom(profileIndex, NULL, homePosition, hbpTemp, rightTemp, leftTemp);

		interface::popScreen();
		break;
	}
}

void ProfileChangeNameModeScreen::reset() {
	cursorLocation = 0;
	getProfileName(profileIndex, profileName);
}

void ProfileChangeNameModeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(PROFILE_PROFILE_NAME_MSG);
		lcd.moveWriteFromPgmspace(0, 3, UPDNLRM_MSG);
	}

	lcd.setRow(1);
	lcd.writeString((char *)profileName);

	//Draw the cursor
	lcd.setCursor(cursorLocation,2);
	lcd.write('^');

	//Write a blank before and after the cursor if we're not at the ends
	if ( cursorLocation >= 1 )
		lcd.moveWriteFromPgmspace(cursorLocation-1, 2, BLANK_CHAR_MSG);
	if ( cursorLocation < PROFILE_NAME_SIZE )
		lcd.moveWriteFromPgmspace(cursorLocation+1, 2, BLANK_CHAR_MSG);
}

void ProfileChangeNameModeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
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

ProfileDisplaySettingsMenu::ProfileDisplaySettingsMenu() :
	Menu(0, (uint8_t)8) {
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
	case 3:
	case 4:
		lcd.write('X' + (index - 2));
		lcd.writeFromPgmspace(XYZOFFSET_MSG);
		lcd.writeFloat(stepperAxisStepsToMM(home[index - 2], index - 2), 3, LCD_SCREEN_WIDTH - 2);
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

EepromMenu::EepromMenu() :
	Menu(0, (uint8_t)3) {
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
			const static PROGMEM prog_uchar msg1[] = "This menu can make";
			const static PROGMEM prog_uchar msg2[] = "your bot inoperable.";
			const static PROGMEM prog_uchar msg4[] = "Press UP to proceed.";

			lcd.moveWriteFromPgmspace(0, 0, msg1);
			lcd.moveWriteFromPgmspace(0, 1, msg2);
			lcd.moveWriteFromPgmspace(0, 2, CLEAR_MSG);
			lcd.moveWriteFromPgmspace(0, 3, msg4);
		}
	}
	else {
		if ( itemSelected != -1 )
			lcd.clearHomeCursor();

		const static PROGMEM prog_uchar message_dump[]	     = "Saving...";
		const static PROGMEM prog_uchar message_restore[]    = "Restoring...";
		const static PROGMEM prog_uchar message_erasing[]    = "Erasing...";
		const static PROGMEM prog_uchar eeprom_writefail[]   = "SD card write failed";
		const static PROGMEM prog_uchar eeprom_dumpexists[]  = DUMP_FILE " file" "already exists";
		const static PROGMEM prog_uchar eeprom_dumpfnf[]     = DUMP_FILE " file" "not found";
		const static PROGMEM prog_uchar eeprom_badrestore[]  = "Read failed; EEPROM " "may be corrupt";

		switch ( itemSelected ) {
		case 0:	//Dump
			if ( ! sdcard::fileExists(dumpFilename) ) {
				lcd.writeFromPgmspace(message_dump);
				if ( ! eeprom::saveToSDFile(dumpFilename) ) {
					MenuBadness(eeprom_writefail);
					return;
				}
			}
			else {
				MenuBadness(eeprom_dumpexists);
				return;
			}
			interface::popScreen();
			break;

		case 1: //Restore
			if ( sdcard::fileExists(dumpFilename) ) {
				lcd.writeFromPgmspace(message_restore);
				if ( ! eeprom::restoreFromSDFile(dumpFilename) ) {
					MenuBadness(eeprom_badrestore);
					return;
				}
			}
			else {
				MenuBadness(eeprom_dumpfnf);
				return;
			}
			interface::popScreen();
			host::stopBuildNow();
			break;

		case 2: //Erase
			lcd.writeFromPgmspace(message_erasing);
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
		}
		else
			lcd.writeFromPgmspace(CLEAR_MSG);

		itemSelected = -1;
	}
}

void EepromMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;
	switch (index) {
	default:
		return;
	case 0:
		msg = EEPROM_DUMP_MSG;
		break;
	case 1:
		msg = EEPROM_RESTORE_MSG;
		break;
	case 2:
		msg = EEPROM_ERASE_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
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

void HomeOffsetsModeScreen::reset() {
	cli();
	eeprom_read_block(homePosition, (void *)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));
	sei();

	lastHomeOffsetState = HOS_NONE;
	homeOffsetState     = HOS_OFFSET_X;
	valueChanged = false;
}

void HomeOffsetsModeScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if ( homeOffsetState != lastHomeOffsetState )
		forceRedraw = true;

	if ( forceRedraw ) {
		lcd.clearHomeCursor();
		lcd.write('X' + homeOffsetState - HOS_OFFSET_X);
		lcd.writeFromPgmspace(XYZOFFSET_MSG);
		lcd.moveWriteFromPgmspace(0, 3, UPDNLM_MSG);
	}

	float position = stepperAxisStepsToMM(homePosition[homeOffsetState - HOS_OFFSET_X], homeOffsetState - HOS_OFFSET_X);

	lcd.setRow(1);
	lcd.writeFloat(position, 3, 0);
	lcd.writeFromPgmspace(MILLIMETERS_MSG);
	lcd.writeFromPgmspace(BLANK_CHAR_MSG);

	lastHomeOffsetState = homeOffsetState;
}

void HomeOffsetsModeScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
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
	multiplier = 1.0;

	if ( currentPause == 0 ) {
		Point position = steppers::getPlannerPosition();
		pauseAtZPos = stepperAxisStepsToMM(position[2], Z_AXIS);
		if ( pauseAtZPos < 0 )	pauseAtZPos = 0;
	}
	else
		pauseAtZPos = stepperAxisStepsToMM(currentPause, Z_AXIS);
}

void PauseAtZPosScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(PAUSE_AT_ZPOS_MSG);

		lcd.moveWriteFromPgmspace(0, 3, UPDNLM_MSG);
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
		// Set the pause
		command::pauseAtZPos(stepperAxisMMToSteps(pauseAtZPos, Z_AXIS));
		// Fall through
	case ButtonArray::LEFT:
		interface::popScreen();
		break;
	case ButtonArray::RIGHT:
	        multiplier *= 10.0;
		if ( multiplier > 100.0 ) multiplier = 1.0;
		break;
	case ButtonArray::UP:
	case ButtonArray::DOWN:
	        float incr;
		repetitions = Motherboard::getBoard().getInterfaceBoard().getButtonRepetitions();
		if ( repetitions > 18 ) incr = 10.0;
		else if ( repetitions > 12 ) incr = 1.0;
		else if ( repetitions > 6 ) incr = 0.1;
		else incr = 0.01;
		if ( button == ButtonArray::UP )
		    pauseAtZPos += incr * multiplier;
		else
		    pauseAtZPos -= incr * multiplier;
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

void ChangeSpeedScreen::reset() {
	speedFactor = steppers::speedFactor;
	alterSpeed = steppers::alterSpeed;
}

void ChangeSpeedScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(CHANGE_SPEED_MSG);

		lcd.moveWriteFromPgmspace(0, 3, UPDNLM_MSG);
	}

	lcd.setRow(1);
	lcd.write('x');
	lcd.writeFloat(FPTOF(steppers::speedFactor), 2, 0);
}

void ChangeSpeedScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	FPTYPE sf = steppers::speedFactor;

	switch (button) {
	case ButtonArray::LEFT:
		// Treat as a cancel
		steppers::alterSpeed  = alterSpeed;
		steppers::speedFactor = speedFactor;
		// FALL THROUGH
	case ButtonArray::CENTER:
		interface::popScreen();
		return;
	case ButtonArray::UP:
		// increment less
		sf += KCONSTANT_0_05;
		break;
	case ButtonArray::DOWN:
		// decrement less
		sf -= KCONSTANT_0_05;
		break;
	default:
		return;
	}

	//Range clamping
	if ( sf > KCONSTANT_5 ) sf = KCONSTANT_5;
	else if ( sf < KCONSTANT_0_1 ) sf = KCONSTANT_0_1;

	steppers::alterSpeed  = (sf == KCONSTANT_1) ? 0x00 : 0x80;
	steppers::speedFactor = sf;
}

void ChangeTempScreen::reset() {
	// Make getTemp() thing that a toolhead change has occurred
	activeToolhead = 255;
	altTemp = 0;
	getTemp();
}

void ChangeTempScreen::getTemp() {
	uint8_t at;
	steppers::getStepperPosition(&at);
	if ( at != activeToolhead ) {
		activeToolhead = at;
		altTemp = command::altTemp[activeToolhead];
		if ( altTemp == 0 ) {
		    // Get the current set point
		    altTemp = (uint16_t)Motherboard::getBoard().getExtruderBoard(activeToolhead).getExtruderHeater().get_set_temperature();
		    if ( altTemp == 0 )
			altTemp = command::pausedExtruderTemp[activeToolhead];
		}
	}
}

void ChangeTempScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw) {
	if (forceRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(CHANGE_TEMP_MSG);

		lcd.moveWriteFromPgmspace(0, 3, UPDNLM_MSG);
	}

	// Since the print is still running, the active tool head may have changed
	getTemp();

	// Redraw tool info
	lcd.setRow(1);
	lcd.writeInt(altTemp, 3);
	lcd.write('C');
}

void ChangeTempScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	int16_t temp = (int16_t)(0x7fff & altTemp);
	switch (button) {
	case ButtonArray::CENTER:
	{
		// Set the temperature
		command::altTemp[activeToolhead] = altTemp;
		Motherboard &board = Motherboard::getBoard();
		if ( board.getExtruderBoard(activeToolhead).getExtruderHeater().get_set_temperature() != 0 )
			board.getExtruderBoard(activeToolhead).getExtruderHeater().set_target_temperature(altTemp);
#ifdef DITTO_PRINT
		if ( command::dittoPrinting ) {
		    uint8_t otherToolhead = activeToolhead ? 0 : 1;
		    command::altTemp[otherToolhead] = altTemp;
		    if ( board.getExtruderBoard(otherToolhead).getExtruderHeater().get_set_temperature() != 0 )
			board.getExtruderBoard(otherToolhead).getExtruderHeater().set_target_temperature(altTemp);
		}
#endif
	}
	// FALL THROUGH
	case ButtonArray::LEFT:
		interface::popScreen();
		return;
	case ButtonArray::UP:
		// increment
		temp += 1;
		break;
	case ButtonArray::DOWN:
		// decrement
		temp -= 1;
		break;
	default:
		return;
	}

	if (temp > MAX_VALID_TEMP ) altTemp = MAX_VALID_TEMP;
	else if ( temp < 0 ) altTemp = 0;
	else altTemp = (uint16_t)(0x7fff & temp);
}

ActiveBuildMenu::ActiveBuildMenu() :
	Menu(0, (uint8_t)0) {
	reset();
}

void ActiveBuildMenu::resetState() {
	fanState = EX_FAN.getValue();
	is_paused = command::isPaused();

	itemCount = is_paused ? 7 : 9;  // paused: 6 + load/unload; !paused: 6 + fan off + pause @ zpos + cold

	//If any of the heaters are on, we provide another
	//  menu options, "Heaters Off"
	//  and if we have reached temp, then jog mode is available as well
	if ( is_paused ) {
	    Motherboard& board = Motherboard::getBoard();
	    is_hot = (board.getExtruderBoard(0).getExtruderHeater().get_set_temperature() > 0) || 
		(board.getExtruderBoard(1).getExtruderHeater().get_set_temperature() > 0) ||
		(board.getPlatformHeater().get_set_temperature() > 0);
	    is_heating = board.getExtruderBoard(0).getExtruderHeater().isHeating() ||
		board.getExtruderBoard(1).getExtruderHeater().isHeating() ||
		board.getPlatformHeater().isHeating();
	    if ( !is_heating ) itemCount += 1; // jog menu  (allow for hot | cold pause for load/unload maneuvers)
	    if ( is_hot ) itemCount += 1;      // heaters off
	}
	else {
	    is_heating = false;
	    is_hot     = false;
	}
}

void ActiveBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg = 0;
	uint8_t lind = 0;

  	if ( index == lind ) msg = BACK_TO_MONITOR_MSG;
	lind++;

	if ( index == lind ) msg = CANCEL_BUILD_MSG;;
	lind++;

	if ( index == lind ) {
		resetState();	//Required to update the pause state if we were previously in
				//another menu on top of this one
		msg = is_paused ? UNPAUSE_MSG : PAUSE_MSG;
	}
	lind++;

	if ( is_paused && !is_heating ) {
		if ( index == lind ) msg = JOG_MSG;
		lind++;
	}

	if ( is_paused ) {
		if ( index == lind ) msg = FILAMENT_OPTIONS_MSG;
		lind++;
	}

	if ( is_paused && is_hot ) {
		if (index == lind ) msg = HEATERS_OFF_MSG;
		lind++;
	}

	if ( !is_paused ) {
		if ( index == lind ) msg = PAUSEATZPOS_MSG;
		lind++;
	}

	if ( index == lind ) msg = CHANGE_SPEED_MSG;
	lind++;

	if ( index == lind ) msg = CHANGE_TEMP_MSG;
	lind++;

	// Fan should be off when paused
	if ( !is_paused ) {
		if ( index == lind ) msg = fanState ? FAN_OFF_MSG : FAN_ON_MSG;
		lind++;
	}

	if ( index == lind ) msg = STATS_MSG;
	lind++;

	if ( !is_paused ) {
		if ( index == lind ) msg = COLD_PAUSE_MSG;
		lind++;
	}

	if ( msg ) lcd.writeFromPgmspace(msg);
}

void ActiveBuildMenu::handleSelect(uint8_t index) {
	uint8_t lind = 0;

  	if ( index == lind ) {
		interface::popScreen();
		return;
	}
	lind++;

	if ( index == lind ) {
		// Cancel build
		cancelBuildMenu.state = 0;
		interface::pushScreen(&cancelBuildMenu);
		return;
	}
	lind++;

	if ( index == lind )  {
		// pause command execution
		is_paused = !is_paused;
		host::pauseBuild(is_paused, false);
		if ( is_paused ) {
			resetState(); // options have changed
			needsRedraw = true;
		}
		else
			interface::popScreen();
		return;
	}
	lind++;

	if ( is_paused && !is_heating ) {
		if ( index == lind ) {
			interface::pushScreen(&jogModeScreen);
			return;
		}
		lind++;
	}

	if ( is_paused ) {
		if ( index == lind ) {
			//Handle filament
			cancelBuildMenu.state = 2;
			filamentScreen.leaveHeatOn = eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE);
			interface::pushScreen(&filamentMenu);
			return;
		}
		lind++;
	}

	if ( is_paused && is_hot ) {
		if ( index == lind ) {
			//Switch all the heaters off
			Motherboard::heatersOff(true);
			resetState();
			needsRedraw = true;
			return;
		}
		lind++;
	}

	if ( !is_paused ) {
		if ( index == lind ) {
			interface::pushScreen(&pauseAtZPosScreen);
			return;
		}
		lind++;
	}

	if ( index == lind ) {
		interface::pushScreen(&changeSpeedScreen);
		return;
	}
	lind++;

	if ( index == lind ) {
		interface::pushScreen(&changeTempScreen);
		return;
	}
	lind++;

	// Fan should be off when paused
	if ( !is_paused ) {
		if ( index == lind ) {
			fanState = !fanState;
			EX_FAN.setValue(fanState);
			lineUpdate = true;
			return;
		}
		lind++;
	}

	if ( index == lind ) {
		interface::pushScreen(&buildStatsScreen);
		return;
	}
	lind++;

	if ( !is_paused ) {
		if ( index == lind ) {
			is_paused = true;
			host::pauseBuild(true, true);
			resetState();
			needsRedraw = true;
		}
	}
}

void BuildStatsScreen::update(LiquidCrystalSerial& lcd, bool forceRedraw){

	if (forceRedraw) {
		lcd.clearHomeCursor();
		lcd.writeFromPgmspace(BUILD_TIME_MSG);
		lcd.moveWriteFromPgmspace(0, 1, Z_POSITION_MSG);
		lcd.moveWriteFromPgmspace(0, 2, FILAMENT_MSG);
		lcd.moveWriteFromPgmspace(0, 3, LINE_NUMBER_MSG);
	}

	Point position;

	switch (update_count){

	case 0:
		uint8_t build_hours;
		uint8_t build_minutes;
		host::getPrintTime(build_hours, build_minutes);

		lcd.setCursor(12,0);
		lcd.writeInt(build_hours,4);

		lcd.setCursor(17,0);
		lcd.writeInt(build_minutes,2);

		break;
	case 1:
		uint32_t line_number;
		line_number = command::getLineNumber();
		/// if line number greater than counted, print an indicator that we are over count
		if ( line_number > command::MAX_LINE_COUNT ) {
			//Replaced with this message, because writeInt / writeInt32 can't display it anyway.
			//and 1,000,000,000 lines would represent 115 days of printing at 100 moves per second
			lcd.moveWriteFromPgmspace(0, 3, PRINTED_TOO_LONG_MSG);
		}
		else {
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
		lcd.moveWriteFromPgmspace(10, 1, BLANK_CHAR_4_MSG);
		lcd.writeFloat(stepperAxisStepsToMM(position[Z_AXIS], Z_AXIS), 3, LCD_SCREEN_WIDTH - 2);
		lcd.writeFromPgmspace(MILLIMETERS_MSG);
		break;

	case 3:
		printFilamentUsed(command::filamentUsed(), lcd);
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

void BuildStatsScreen::reset() {
	update_count = 0;
}

void BuildStatsScreen::notifyButtonPressed(ButtonArray::ButtonName button){

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

CancelBuildMenu::CancelBuildMenu() :
	Menu(IS_CANCEL_SCREEN_MASK, (uint8_t)4) {
	reset();
}

// cancel types (state)
// 0 -- building
// 1 -- filament load/unload from utility menu
// 2 -- filament load/unload during build

void CancelBuildMenu::resetState() {
	itemIndex = 2;
	firstItemIndex = 2;
	// state is set by whomever pushed us onto the screen/menu stack
}

void CancelBuildMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;

	switch (index) {
	case 0 :
		msg = (state != 0) ? CANCEL_FIL_MSG : CANCEL_MSG;
		break;
	case 2:
		msg = NO_MSG;
		break;
	case 3:
		msg = YES_MSG;
		break;
	default:
		return;
	}
	lcd.writeFromPgmspace(msg);
}

void CancelBuildMenu::handleSelect(uint8_t index) {
	switch (index) {
	case 2:
		interface::popScreen();
		break;
	case 3:
		// Cancel build
		if ( state != 0 ) {
			// We're merely paused while printing
			interface::popScreen();
			interface::popScreen();
			if ( (state != 2) || (0 == eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE)) )
				// Turn heat off if cancelling a utility filament load/unload or if canceling
				//   a filament load during pause and HEAT_DURING_PAUSE is disabled
				Motherboard::heatersOff(true);
		}
		else {
			command::addFilamentUsed();
			host::stopBuild();
		}
		break;
	}
}


MainMenu::MainMenu() :
	Menu(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)4) {
	reset();
}

void MainMenu::resetState() {
	itemIndex = 1;
	firstItemIndex = 1;
}

void MainMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	const prog_uchar *msg;

	switch (index) {
	default:
		return;
	case 0:
	{
		char *name = host::getMachineName();
		lcd.setCursor((LCD_SCREEN_WIDTH - strlen(name)) >> 1, 0);
		lcd.writeString(name);
		return;
	}
	case 1:
		msg = BUILD_MSG;
		break;
	case 2:
		msg = PREHEAT_MSG;
		break;
	case 3:
		msg = UTILITIES_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
}

void MainMenu::handleSelect(uint8_t index) {
	switch (index) {
	case 1:
		// Show build from SD screen
		interface::pushScreen(&sdMenu);
		break;
	case 2:
		// Show preheat screen
		interface::pushScreen(&heaterPreheatMenu);
		break;
	case 3:
		// home axes script
		interface::pushScreen(&utilityMenu);
		break;
	}
}


UtilitiesMenu::UtilitiesMenu() :
	Menu(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN),(uint8_t)18) {
	singleTool = eeprom::isSingleTool();
	if (singleTool) itemCount--; // No nozzleCalibration
	reset();
}

void UtilitiesMenu::resetState(){
	singleTool = eeprom::isSingleTool();
	itemCount = 17;
	if ( singleTool ) --itemCount;
	stepperEnable = ( axesEnabled ) ? false : true;
}

void UtilitiesMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	const prog_uchar *msg;
	switch (index) {
	default:
		return;
	case 0:
		msg = MONITOR_MSG;
		break;
	case 1:
		msg = FILAMENT_OPTIONS_MSG;
		break;
	case 2:
		msg = PREHEAT_SET_MSG;
		break;
	case 3:
		msg = PLATE_LEVEL_MSG;
		break;
		// ------ next screen ------
	case 4:
		msg = HOME_AXES_MSG;
		break;
	case 5:
		msg = BOT_STATS_MSG;
		break;
	case 6:
		msg = FILAMENT_ODOMETER_MSG;
		break;
	case 7:
		msg = SETTINGS_MSG;
		break;
		// ------ next screen ------
	case 8:
		msg = PROFILES_MSG;
		break;
	case 9:
		msg = HOME_OFFSETS_MSG;
		break;
	case 10:
		msg = JOG_MSG;
		break;
	case 11:
		msg = stepperEnable ? ESTEPS_MSG : DSTEPS_MSG;
		break;
		// ------ next screen ------
	case 12:
		msg = singleTool ? RESET_MSG : NOZZLES_MSG;
		break;
	case 13:
		msg = singleTool ? EEPROM_MSG : RESET_MSG;
		break;
	case 14:
		msg = singleTool ? VERSION_MSG : EEPROM_MSG;
		break;
	case 15:
		msg = singleTool ? EXIT_MSG : VERSION_MSG;
		break;
		// ------ next screen ------
	case 16:
		msg = EXIT_MSG;
		break;
	}
	lcd.writeFromPgmspace(msg);
}

void UtilitiesMenu::handleSelect(uint8_t index) {

	switch (index) {
	case 0:
		// Show monitor build screen
		interface::pushScreen(&monitorModeScreen);
		break;
	case 1:
		// load filament script
		cancelBuildMenu.state = 1;
	        filamentScreen.leaveHeatOn = 0;
		interface::pushScreen(&filamentMenu);
		break;
	case 2:
		interface::pushScreen(&preheatSettingsMenu);
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
		interface::pushScreen(&botStatsScreen);
		break;
	case 6:
		// Filament Odometer
		interface::pushScreen(&filamentOdometerScreen);
		break;
	case 7:
		// settings menu
		interface::pushScreen(&settingsMenu);
		break;
	case 8:
		// Profiles
		interface::pushScreen(&profilesMenu);
		break;
	case 9:
		// Home Offsets
		interface::pushScreen(&homeOffsetsModeScreen);
		break;
	case 10:
		// Jog axes
		interface::pushScreen(&jogModeScreen);
		break;
	case 11:
		steppers::enableAxes(0xff, stepperEnable);
		lineUpdate = true;
		stepperEnable = !stepperEnable;
		break;
	case 12:
		if ( singleTool ) interface::pushScreen(&resetSettingsMenu);
#ifndef SINGLE_EXTRUDER
#ifdef NOZZLE_CALIBRATION_SCREEN
		else interface::pushScreen(&nozzleCalibrationScreen);
#else
		else interface::pushScreen(&selectAlignmentMenu);
#endif
#endif
		break;
	case 13:
		if ( singleTool ) interface::pushScreen(&eepromMenu);
		else interface::pushScreen(&resetSettingsMenu);
		break;
	case 14:
		//Eeprom Menu
		if ( !singleTool )
			interface::pushScreen(&eepromMenu);
		else
		{
			splashScreen.hold_on = true;
			interface::pushScreen(&splashScreen);
		}
		break;
	case 15:
		if ( !singleTool ) {
			splashScreen.hold_on = true;
			interface::pushScreen(&splashScreen);
		}
		else
			interface::popScreen();
		break;
	case 16:
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

	uint16_t total_hours;
	uint8_t total_minutes;
	eeprom::getBuildTime(&total_hours, &total_minutes);

	uint8_t digits = 1;
	for (uint32_t i = 10; i < 100000; i *= 10) {
		if ( i > (uint32_t)total_hours ) break;
		digits++;
	}
	lcd.setCursor(15 - digits, 0);
	lcd.writeInt(total_hours, digits);
	lcd.setCursor(17, 0);
	lcd.writeInt(total_minutes, 2);

	/// LAST PRINT TIME
	lcd.setRow(1);
	printLastBuildTime(LAST_TIME_MSG, 1, lcd);

	/// TOTAL FILAMENT USED
	filamentOdometers(false, 2, lcd);
}

void BotStatsScreen::reset() {
}

void BotStatsScreen::notifyButtonPressed(ButtonArray::ButtonName button) {
	if ( button == ButtonArray::LEFT )
		interface::popScreen();
}

SettingsMenu::SettingsMenu() :
	CounterMenu(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)10
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
#ifdef HAS_RGB_LED
	LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::BASIC_COLOR_OFFSET, LED_DEFAULT_WHITE);
	heatingLEDOn = 0 != eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1);
#endif
	accelerationOn = 0 != eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACCELERATION_ACTIVE, 0x01);
	overrideGcodeTempOn = 0 != eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0);
	pauseHeatOn = 0 != eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE);
	extruderHoldOn = 0 != eeprom::getEeprom8(eeprom_offsets::EXTRUDER_HOLD,
						 DEFAULT_EXTRUDER_HOLD);
	toolOffsetSystemOld = 0 == eeprom::getEeprom8(eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
						      DEFAULT_TOOLHEAD_OFFSET_SYSTEM);
	useCRC = 1 == eeprom::getEeprom8(eeprom_offsets::SD_USE_CRC, DEFAULT_SD_USE_CRC);
	pstopEnabled = 1 == eeprom::getEeprom8(eeprom_offsets::PSTOP_ENABLE, 0);
#ifdef DITTO_PRINT
	dittoPrintOn = 0 != eeprom::getEeprom8(eeprom_offsets::DITTO_PRINT_ENABLED, 0);
	if ( singleExtruder ) dittoPrintOn = false;
#endif
}

void SettingsMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {
	bool test;
	const prog_uchar *msg;
	uint8_t selIndex = selectIndex;

	uint8_t row = index % 4;
#ifndef DITTO_PRINT
	index++;
	selIndex++;
#endif

	lcd.setCursor(16, row);
	lcd.write((selIndex == index) ? LCD_CUSTOM_CHAR_RIGHT : ' ');

	switch (index) {
	default:
		return;
#ifdef DITTO_PRINT
	case 0:
		lcd.moveWriteFromPgmspace(1, row, DITTO_PRINT_MSG);
		lcd.setCursor(17, row);
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

		msg = ACCELERATE_MSG;
		test = accelerationOn;
		break;
	case 5:
		lcd.moveWriteFromPgmspace(1, row, TOOL_COUNT_MSG);
		lcd.setCursor(17, row);
		lcd.write(singleExtruder ? '1' : '2');
		return;
	case 6:
		msg = EXTRUDER_HOLD_MSG;
		test = extruderHoldOn;
		break;
	case 7:
		lcd.moveWriteFromPgmspace(1, row, HBP_MSG);
		lcd.moveWriteFromPgmspace(17, row, hasHBP ? YES_MSG : NO_MSG);
		return;
	case 8:
		lcd.moveWriteFromPgmspace(1, row, TOOL_OFFSET_SYSTEM_MSG);
		lcd.moveWriteFromPgmspace(17, row, toolOffsetSystemOld ? OLD_MSG : NEW_MSG);
		return;
	case 9:
		lcd.moveWriteFromPgmspace(1, row, SD_USE_CRC_MSG);
		lcd.moveWriteFromPgmspace(17, row, useCRC ? YES_MSG : NO_MSG);
		return;
	case 10:
		msg = PSTOP_ENABLE_MSG;
		test = pstopEnabled;
		break;
	}
	lcd.moveWriteFromPgmspace(1, row, msg);
	lcd.moveWriteFromPgmspace(17, row, test ? ON_MSG : OFF_MSG);
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
		return;
#endif
	case 1:
		// update right counter
		overrideGcodeTempOn = !overrideGcodeTempOn;
		return;
	case 2:
		// update right counter
		pauseHeatOn = !pauseHeatOn;
		return;
	case 3:
		// update right counter
		soundOn = !soundOn;
		return;
	case 4:
		// update right counter
		accelerationOn = !accelerationOn;
		return;
	case 5:
		// update platform counter
		// update right counter
		singleExtruder = !singleExtruder;
		return;
	case 6:
		// update right counter
		extruderHoldOn = !extruderHoldOn;
		return;
	case 7:
		// update right counter
		hasHBP = !hasHBP;
		return;
	case 8:
		toolOffsetSystemOld = !toolOffsetSystemOld;
		return;
	case 9:
		useCRC = !useCRC;
		return;
	case 10:
		pstopEnabled = !pstopEnabled;
		return;
	}
}


void SettingsMenu::handleSelect(uint8_t index) {
#ifndef DITTO_PRINT
	index++;
#endif
	lineUpdate = 1;

	switch (index) {
	default:
		lineUpdate = 0;
		return; // prevents line_update from changing;
#ifdef DITTO_PRINT
	case 0:
		if ( singleExtruder )
			return;
		eeprom_write_byte((uint8_t*)eeprom_offsets::DITTO_PRINT_ENABLED,
				  dittoPrintOn ? 1 : 0);
		command::reset();
		return;
#endif
	case 1:
		eeprom_write_byte((uint8_t *)eeprom_offsets::OVERRIDE_GCODE_TEMP,
				  overrideGcodeTempOn ? 1 : 0);
		return;
	case 2:
		eeprom_write_byte((uint8_t*)eeprom_offsets::HEAT_DURING_PAUSE,
				  pauseHeatOn ? 1 : 0);
		return;
	case 3:
		// update sound preferences
		eeprom_write_byte((uint8_t*)eeprom_offsets::BUZZ_SETTINGS,
				  soundOn ? 1 : 0);
		Piezo::reset();
		return;
	case 4:
		eeprom_write_byte((uint8_t*)eeprom_offsets::ACCELERATION_SETTINGS +
				  acceleration_eeprom_offsets::ACCELERATION_ACTIVE,
				  accelerationOn ? 1 : 0);
		steppers::reset();
		return;
	case 5:
		eeprom::setToolHeadCount(singleExtruder ? 1 : 2);
		if ( singleExtruder )
			Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
		command::reset();
		return;
	case 6:
		eeprom_write_byte((uint8_t*)eeprom_offsets::EXTRUDER_HOLD,
				  extruderHoldOn ? 1 : 0);
		command::reset();
		return;
	case 7:
		eeprom_write_byte((uint8_t*)eeprom_offsets::HBP_PRESENT, hasHBP ? 1 : 0);
		if ( !hasHBP )
		    Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
		command::reset();
		return;
	case 8:
		eeprom_write_byte((uint8_t*)eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
				  toolOffsetSystemOld ? 0 : 1);
		command::reset();
		return;
	case 9:
		eeprom_write_byte((uint8_t*)eeprom_offsets::SD_USE_CRC,
				  useCRC ? 1 : 0);
#ifndef BROKEN_SD
		sdcard::mustReinit = true;
#endif
		return;
	case 10:
#ifdef PSTOP_SUPPORT
		Motherboard::getBoard().pstop_enabled = pstopEnabled ? 1 : 0;
		eeprom_write_byte((uint8_t*)eeprom_offsets::PSTOP_ENABLE,
				  Motherboard::getBoard().pstop_enabled);
#endif
		steppers::init();
		return;
	}
}

//Returns true if the file is an s3g/j4g file
//Keeping this in C instead of C++ saves 20 bytes

bool isSXGFile(char *filename, uint8_t len) {
	if ((len >= 4) &&
	    (filename[len-4] == '.') &&
	    ((filename[len-3] == 's') || (filename[len-3] == 'x') ||
	     (filename[len-3] == 'S') || (filename[len-3] == 'X')) &&
	    (filename[len-2] == '3') &&
	    ((filename[len-1] == 'g') || (filename[len-1] == 'G'))) return true;
	return false;
}

// Count the number of files on the SD card
static uint8_t fileCount;

uint8_t countFiles() {
	fileCount = 0;

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
			return fileCount;
		// Count .. and anyfile which doesn't begin with .
		if ( isdir ) {
			if ( fnbuf[0] != '.' || ( fnbuf[1] == '.' && fnbuf[2] == 0 ) ) fileCount++;
		}
		else if ( isSXGFile(fnbuf, flen) ) fileCount++;
	} while (true);

	// Never reached
	return fileCount;
}

bool getFilename(uint8_t index, char buffer[], uint8_t buffer_size, uint8_t *buflen, bool *isdir) {

	*buflen = 0;
	*isdir = false;

	// First, reset the directory list
	if ( sdcard::directoryReset() != sdcard::SD_SUCCESS )
		return false;

	uint8_t my_buflen = 0; // set to zero in case the for loop never runs
	bool my_isdir;

#ifdef REVERSE_SD_FILES
	// present files in reverse order in hopes this will show newer files first
	// HOWEVER, with wrap around on the LCD menu, this isn't too useful
	index = (fileCount - 1) - index;
#endif
	for (uint8_t i = 0; i < index+1; i++) {
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

FinishedPrintMenu::FinishedPrintMenu() :
	Menu(0, (uint8_t)4)
{
	reset();
}

void FinishedPrintMenu::resetState() {
	if ( sdcard::sdAvailable != sdcard::SD_SUCCESS )
		lastFileIndex = 255;
	itemIndex = 3;
	firstItemIndex = (lastFileIndex != 255) ? 2 : 3;
}

void FinishedPrintMenu::drawItem(uint8_t index, LiquidCrystalSerial& lcd) {

	switch (index) {
	case 0:
		printLastBuildTime(BUILD_TIME2_MSG, 0, lcd);
		break;
	case 1:
		// Filament used
		lcd.writeFromPgmspace(FILAMENT_MSG);
		printFilamentUsed(command::filamentUsed(), lcd);
		break;
	case 2:
		// Print Another Copy
		lcd.writeFromPgmspace((lastFileIndex != 255) ? PRINT_ANOTHER_MSG : CANNOT_PRINT_ANOTHER_MSG);
		break;
	case 3:
		lcd.writeFromPgmspace(RETURN_TO_MAIN_MSG);
		break;
	}
}

void FinishedPrintMenu::handleSelect(uint8_t index) {
	if ( index == 3 || lastFileIndex == 255 ) {
		interface::popScreen();
		return;
	}
	char fname[SD_MAXFILELENGTH+1];
	uint8_t flen;
	bool isdir;
	if ( !getFilename(lastFileIndex, fname, sizeof(fname), &flen, &isdir) || isdir )
		goto badness;
	if ( host::startBuildFromSD(fname, flen) == sdcard::SD_SUCCESS )
		return;
badness:
	// XXXX  can we pop the top screen?
	MenuBadness((sdcard::sdAvailable == sdcard::SD_ERR_CRC) ? CARDCRC_MSG : CARDOPENERR_MSG);
}

SDMenu::SDMenu() :
	Menu(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN), (uint8_t)0),
	drawItemLockout(false), selectable(false),
	updatePhase(0), updatePhaseDivisor(0), folderStackIndex(-1) {
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
	uint8_t jj = offset + longFilenameOffset;
	for (idx = 0; (idx < displayWidth) && ((longFilenameOffset + idx) < sizeof(fnbuf)) &&
		     (fnbuf[jj + idx] != 0); idx++)
		lcd.write(fnbuf[jj + idx]);

	//Clear out the rest of the line
	while ( idx++ < displayWidth )
		lcd.write(' ');
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

		//Continuous Buttons Fires every 0.05 seconds, but the default
		//menu update is every 0.5 seconds, which is slow for a continuous press.
		//Menu update rate is changed to 0.05 seconds in Menu.hh, we divide
		//by 10 here to give a filename scrolling update rate of 0.5 seconds.
		if ( ++updatePhaseDivisor >= 10 ) {
			updatePhase ++;
			updatePhaseDivisor = 0;
		}
	}
	else {
		// This was actually triggered in drawItem() but popping a screen
		// from there is not a good idea
		const prog_uchar *msg;
		if ( (sdcard::sdAvailable == sdcard::SD_ERR_DEGRADED) ||
		     (sdcard::sdErrno & SDR_ERR_COMMS) ) msg = CARDCOMMS_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_SUCCESS ) msg = CARDNOFILES_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_NO_CARD_PRESENT ) msg = NOCARD_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_OPEN_FILESYSTEM ) msg = CARDFORMAT_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_VOLUME_TOO_BIG ) msg = CARDSIZE_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_CRC ) msg = CARDCRC_MSG;
		else msg = CARDERROR_MSG;
		MenuBadness(msg);
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
		MenuBadness(BUILDING_MSG);
		return;
	}
	else if ( !selectable )
		return;

	char fname[SD_MAXFILELENGTH + 1];
	uint8_t flen;
	bool isdir;

	drawItemLockout = true;
	lastFileIndex = 255;

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
	lastFileIndex = index;
	if ( host::startBuildFromSD(fname, flen) == sdcard::SD_SUCCESS )
		return;
badness:
	MenuBadness((sdcard::sdAvailable == sdcard::SD_ERR_CRC) ? CARDCRC_MSG : CARDOPENERR_MSG);
}

#endif
