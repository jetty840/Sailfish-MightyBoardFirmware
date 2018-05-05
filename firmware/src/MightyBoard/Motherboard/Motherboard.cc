/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "Compat.hh"
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include "Motherboard.hh"
#include "Configuration.hh"
#include "Steppers.hh"
#include "Command.hh"
#include "Interface.hh"
#include "Commands.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Piezo.hh"
#ifdef HAS_RGB_LED
#include "RGB_LED.hh"
#endif
#include "Errors.hh"
#include <avr/eeprom.h>
#include <util/delay.h>
#include "Menu_locales.hh"
#include "TemperatureTable.hh"
#include "SDCard.hh"
#include "TWI.hh"

#ifdef DIGIPOT_SUPPORT
#include "DigiPots.hh"
#endif

//Warnings to remind us that certain things should be switched off for release

#ifdef ERASE_EEPROM_ON_EVERY_BOOT
	#warning "Release: ERASE_EEPROM_ON_EVERY_BOOT enabled in Configuration.hh"
#endif

#ifdef DEBUG_ENABLE
	#warning "Release: DEBUG_ENABLE defined"
#endif

#ifdef DEBUG_ONSCREEN
	#warning "Release: DEBUG_ONSCREEN enabled in Configuration.hh"
#endif

#ifndef JKN_ADVANCE
	#warning "Release: JKN_ADVANCE disabled in Configuration.hh"
#endif

#ifdef DEBUG_SLOW_MOTION
	#warning "Release: DEBUG_SLOW_MOTION enabled in Configuration.hh"
#endif

#ifdef DEBUG_NO_HEAT_NO_WAIT
	#warning "Release: DEBUG_NO_HEAT_NO_WAIT enabled in Configuration.hh"
#endif

#ifdef DEBUG_SRAM_MONITOR
	#warning "Release: DEBUG_SRAM_MONITOR enabled in Configuration.hh"
#endif

//Frequency of Timer 5
//100 = (1.0 / ( 16MHz / 64 / 25 = 10KHz)) * 1000000
//#define MICROS_INTERVAL 100

/// ticks of 100 Microsecond units since board initialization
static volatile micros_t centa_micros = 0;
static volatile uint8_t clock_wrap    = 0;

// Seconds since board initialization
static volatile micros_t seconds = 0;
static int16_t mcount            = 0;

// Save an EEPROM lookup and code
// Note correct value will be pulled out of EEPROM; these are just defaults
static bool singleTool;

uint8_t board_status;
static bool heating_lights_active;

#if defined(COOLING_FAN_PWM)
#define FAN_PWM_BITS 6
static uint8_t fan_pwm_bottom_count;
bool           fan_pwm_enable = false;
#endif

#if defined(PSTOP_ZMIN_LEVEL)
static volatile int32_t zprobe_trigger_clear_zpos = 0;
static volatile uint32_t total_zprobe_triggered = 0;
#endif

/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
#if defined(USE_THERMOCOUPLE_DUAL)
#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G
     therm_sensor(THERMOCOUPLE_DO, THERMOCOUPLE_SCK, THERMOCOUPLE_DI, THERMOCOUPLE_CS),
#elif BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     therm_sensor(),
#endif
#endif
#if defined(HAS_I2C_LCD) || defined(HAS_VIKI_INTERFACE)
     lcd(),
#else
     lcd(LCD_STROBE, LCD_DATA, LCD_CLK),
#endif
     messageScreen(),
     mainMenu(),
     finishedPrintMenu(),
#if defined(HAS_VIKI_INTERFACE)
     //The ViKi is both an LCD and a buttonArray, so pass it twice.
     interfaceBoard(lcd, lcd, &mainMenu, &monitorModeScreen,
		    &messageScreen, &finishedPrintMenu),
#else
     interfaceBoard(buttonArray, lcd, &mainMenu, &monitorModeScreen,
		    &messageScreen, &finishedPrintMenu),
#endif
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     platform_thermistor(THERM_CHANNEL_HBP),
#else
     platform_thermistor(PLATFORM_PIN, TABLE_HBP_THERMISTOR),
#endif
     platform_heater(platform_thermistor, platform_element,
		     // NOTE: MBI had the calibration_offset as 0 which then causes
		     //       the calibration_offset for Tool 0 to be used instead
		     eeprom_offsets::T0_DATA_BASE + toolhead_eeprom_offsets::HBP_PID_BASE, false, 2),
     using_platform(eeprom::getEeprom8(eeprom_offsets::HBP_PRESENT, 1)),
#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G || BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     Extruder_One(0, EXA_PWR, EXA_FAN, THERM_CHANNEL_ONE, eeprom_offsets::T0_DATA_BASE),
     Extruder_Two(1, EXB_PWR, EXB_FAN, THERM_CHANNEL_TWO, eeprom_offsets::T1_DATA_BASE)
#elif BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_E
     Extruder_One(0, EXA_PWR, EXA_FAN, THERMOCOUPLE_CS1, eeprom_offsets::T0_DATA_BASE),
     Extruder_Two(1, EXB_PWR, EXB_FAN, THERMOCOUPLE_CS2, eeprom_offsets::T1_DATA_BASE)
#else
#error Unknown BOARD_TYPE
#endif
{
}

#define ENABLE_TIMER_INTERRUPTS		ADVANCE_TIMSKn 	|= (1<<ADVANCE_OCIEnA); \
                			STEPPER_TIMSKn	|= (1<<STEPPER_OCIEnA)

#define DISABLE_TIMER_INTERRUPTS	ADVANCE_TIMSKn 	&= ~(1<<ADVANCE_OCIEnA); \
                			STEPPER_TIMSKn	&= ~(1<<STEPPER_OCIEnA)

// Initialize Timers
//
//
// Timer  0 = 8 bit with PWM
// Timers 1, 3, 4, 5 = 16 bit with PWM
// Timer  2 = 8 bit with PWM
//
// Priority: 2, 1, 0, 3, 4, 5
//
// Advance Timer should be higher priority than stepper interrupt
// Stepper interrupt should be high priority
// Everything else can be low priority
//
// Replicator 1
//	2 = ( 8) Extruder/Advance timer
//	1 = (16) Extruder 2 (PWM)
//	0 = ( 8) Buzzer
//	3 = (16) Stepper
//	4 = (16) Extruder 1 (PWM)
//	5 = (16) Microsecond timer, "M" flasher, check SD card switch,
//               check P-Stop switch
// Replicator 2
//	2 = ( 8) Extruder/Advance timer
//	1 = (16) Stepper
//	0 = ( 8)
//	3 = (16) Extruders (PWM)
//	4 = (16) Buzzer
//	5 = (16) Microsecond timer, "M" flasher, check SD card switch
//
// Azteeg X3
//   Unfortunately, the extruder heaters are on the Timer 2 outputs
//	2 = ( 8) Extruders (PWM)
//	1 = (16) RGB timer
//	0 = ( 8) Extruder/Advance timer
//	3 = (16) Stepper
//	4 = (16) Buzzer
//	5 = (16) Microsecond timer, "M" flasher, check SD card switch,
//               check P-Stop switch

void Motherboard::initClocks() {

	// Reset and configure timer 0, the piezo buzzer timer
	// No interrupt, frequency controlled by Piezo

	// this call is handled in Piezo::reset() -- no need to make it here as well
	// Piezo::shutdown_timer();

#ifdef JKN_ADVANCE
	// Extruder/Advance timer

	ADVANCE_TCCRnA = ADVANCE_CTC;	       	// CTC
	ADVANCE_TCCRnB = ADVANCE_PRESCALE_64;  	// prescaler at 1/64
	ADVANCE_OCRnA  = 25;			// Generate interrupts 16 MHz / 64 / 25 = 10 KHz
	ADVANCE_TIMSKn = ADVANCE_OCIEnA;	// Enable OCRnA match interrupt
#endif

	// Choice of timer is done in Configuration.hh via STEPPER_ macros
	//
	// Rep 1, Azteeg X3
	//   Reset and configure timer 3, the stepper interrupt timer.
	//   ISR(TIMER3_COMPA_vect)
	//
	// Rep 2:
	//   Reset and configure timer 1, the stepper interrupt timer.
	//   ISR(TIMER1_COMPA_vect)

        STEPPER_TCCRnA = 0x00;
        STEPPER_TCCRnB = 0x0A; //CTC1 + / 8 = 2Mhz.
        STEPPER_TCCRnC = 0x00;
        STEPPER_OCRnA  = 0x2000; //1KHz
        STEPPER_TIMSKn = 0x02; // turn on OCR3A match interrupt  [OCR5A for Rep 2]

	// Heater PWM control for the two extruder heaters

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
	// Reset and configure Timer 2, the extruders' timer
	// Fast PWM mode with TOP=0xFF (8bit; WGM22:0 = 3), cycle frequency = 976 Hz
	// Prescaler is 1/64 (250 KHz)

	TCCR2A = 0b01010011; // Toggle OC2A & OC2B on compare match (COM2A0 | COM2B0 )
	                     // Fast PWM, TOP=0xFF (WGM22:0 = 3 = 0b011)
	TCCR2B = 0b00000100; // 1/64th scaling
	OCR2A  = 0;
	OCR3A  = 0;
	TIMSK2 = 0b00000000; // No interrupts needed
#elif BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G
	// Reset and configure Timer 3, the extruders' timer
	// Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler is 1/64 (250 KHz)

	TCCR3A = 0b00000001; // Toggle OC3A, OC3B, OC3C on compare match (WGM30)
	TCCR3B = 0b00001011; // set to PWM mode, 1/64 prescaling (WGM32 | CS31 | CS30)
	                     // WGMn3:0 = 5 = 0b0101: Fast PWM, 8bit, TOP=0xFF 
	OCR3A  = 0;
	OCR3C  = 0;
	TIMSK3 = 0b00000000; // no interrupts needed
#elif BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_E
	// Reset and configure Timer 1, the extruder two PWM timer
	// Fast PWM mode with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	// No interrupt, PWM controlled by ExtruderBoard

	TCCR1A = 0b00000001;
	TCCR1B = 0b00001011;
	OCR1A  = 0x00;
	OCR1B  = 0x00;
	TIMSK1 = 0x00;	//No interrupts

	// Reset and configure Timer 4, the extruder one PWM timer
	// Fast PWM mode with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	// No interrupt, PWM controlled by ExtruderBoard

	TCCR4A = 0b00000001;
	TCCR4B = 0b00001011;
	TCCR4C = 0x00;
	OCR4A  = 0x00;
	OCR4B  = 0x00;
	TIMSK4 = 0x00;	//No interrupts
#else
#error unknown BOARD_TYPE
#endif

#if defined(PSTOP_SUPPORT)
	pstop_enabled = eeprom::getEeprom8(eeprom_offsets::PSTOP_ENABLE, DEFAULT_PSTOP_ENABLE);
	uint8_t pstop_inverted = eeprom::getEeprom8(eeprom_offsets::PSTOP_INVERTED, DEFAULT_PSTOP_INVERTED);
	if ( pstop_enabled != 1 ) pstop_enabled = 0;
	pstop_value = ( pstop_inverted == 1 ) ? 1 : 0;

#if defined(PSTOP_VECT)
	// We set a LOW pin change interrupt on the X min endstop
	if ( pstop_enabled ) {
		PSTOP_MSK |= ( 1 << PSTOP_PCINT );
		PCICR     |= ( 1 << PSTOP_PCIE );
	}
#endif

#if defined(PSTOP2_VECT)
	// We set a LOW pin change interrupt on the X min endstop
	if ( pstop_enabled ) {
		PSTOP2_MSK |= ( 1 << PSTOP2_PCINT );
		PCICR      |= ( 1 << PSTOP2_PCIE );
	}
#endif
#endif

	// Reset and configure timer 5:
	// Timer 5 is 16 bit
	//
	//   - Microsecond timer, SD card check timer, P-Stop check timer, LED flashing timer

	TCCR5A = 0x00; // WGM51:WGM50 00
	TCCR5B = 0x0B; // WGM53:WGM52 10 (CTC) CS52:CS50 011 (/64) => CTC, 250 KHz
	TCCR5C = 0x00;
	OCR5A  =   25; // 250KHz / 25 => 10 KHz
	TIMSK5 = 0x02; // | ( 1 << OCIE5A  -> turn on OCR5A match interrupt
}

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::init() {
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x01);

#ifdef DIGIPOT_SUPPORT
	DigiPots::init();
#endif

	// Check if the interface board is attached
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x02);

	hasInterfaceBoard = interface::isConnected();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x03);

	initClocks();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x04);
}

void Motherboard::reset(bool hard_reset) {
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x06);

	// Initialize the host and slave UARTs
	UART::getHostUART().enable(true);
	UART::getHostUART().in.reset();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x07);

	if (hasInterfaceBoard) {

		// Make sure our interface board is initialized
		interfaceBoard.init();
		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x09);

		splashScreen.hold_on = false;
		interfaceBoard.pushScreen(&splashScreen);
		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0A);

		if ( hard_reset )
			_delay_ms(3000);

		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0B);

		// Finally, set up the interface
		interface::init(&interfaceBoard, &lcd);
		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0C);

		interface_update_timeout.start(interfaceBoard.getUpdateRate());
	}

	// interface LEDs default to full ON
	interfaceBlink(0,0);

	// only call the piezo buzzer on full reboot start up
	// do not clear heater fail messages, though the user should not be able to soft reboot from heater fail
	if ( hard_reset ) {

#ifdef HAS_RGB_LED
		RGB_LED::init();
		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0D);
#endif
		Piezo::playTune(TUNE_SAILFISH_STARTUP);
		DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0E);

		heatShutdown = 0;
		heatFailMode = HEATER_FAIL_NONE;
	}

	board_status = STATUS_NONE | STATUS_PREHEATING;
	heating_lights_active = false;

#if defined(USE_THERMOCOUPLE_DUAL)
	therm_sensor.init();
#endif

#if CUTOFF_PRESENT
	cutoff.init();
#endif
	extruder_manage_timeout.start(SAMPLE_INTERVAL_MICROS_THERMOCOUPLE);
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x0F);

	// initialize the extruders
	Extruder_One.reset();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x10);

	Extruder_Two.reset();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x11);

	HBP_HEAT.setDirection(true);
	platform_thermistor.init();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x12);

	platform_heater.reset();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x13);

#if defined(SAMPLE_INTERVAL_MICROS_THERMISTOR)
	platform_timeout.start(SAMPLE_INTERVAL_MICROS_THERMISTOR);
#endif

	// Note it's less code to turn them all off at once
	//  then to conditionally turn of or disable
	heatersOff(true);

	// disable extruder two if sigle tool machine
	singleTool = eeprom::isSingleTool();
	if ( singleTool )
	     Extruder_Two.disable(true);

	// disable platform heater if no HBP
	if ( !eeprom::hasHBP() )
	    platform_heater.disable(true);

	// user_input_timeout.start(USER_INPUT_TIMEOUT);
#ifdef HAS_RGB_LED
	RGB_LED::setDefaultColor();
	DEBUG_VALUE(DEBUG_MOTHERBOARD | 0x14);
#endif
	buttonWait = false;

	// turn off the active cooling fan
	EXTRA_FET.setDirection(true);
	setExtra(false);
}

/// Get the number of microseconds that have passed since
/// the board was booted.
micros_t Motherboard::getCurrentCentaMicros(uint8_t *wrap) {
	micros_t micros_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	     micros_snapshot = centa_micros;
	     *wrap = clock_wrap;
	}
	return micros_snapshot;
}

micros_t Motherboard::getCurrentSeconds() {
	micros_t seconds_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		seconds_snapshot = seconds;
	}
	return seconds_snapshot;
}

/// Run the motherboard interrupt
void Motherboard::doStepperInterrupt() {
	//We never ignore interrupts on pause, because when paused, we might
	//extrude filament to change it or fix jams

	DISABLE_TIMER_INTERRUPTS;
	sei();

	steppers::doStepperInterrupt();

	cli();
	ENABLE_TIMER_INTERRUPTS;

#ifdef ANTI_CLUNK_PROTECTION
	//Because it's possible another stepper interrupt became due whilst
	//we were processing the last interrupt, and had stepper interrupts
	//disabled, we compare the counter to the requested interrupt time
	//to see if it overflowed.  If it did, then we reset the counter, and
	//schedule another interrupt for very shortly into the future.
	if ( STEPPER_TCNTn >= STEPPER_OCRnA ) {
		STEPPER_OCRnA = 0x01;	//We set the next interrupt to 1 interval, because this will cause the
					//interrupt to  fire again on the next chance it has after exiting this
					//interrupt, i.e. it gets queued.

		STEPPER_TCNTn = 0;	//Reset the timer counter

		//debug_onscreen1 ++;
	}
#endif
}

void Motherboard::HeatingAlerts() {
	int16_t setTemp = 0;
	int16_t deltaTemp = 0;
	int16_t top_temp = 0;
	int16_t div_temp = 0;

	/// show heating progress
	// TODO: top temp should use preheat temps stored in eeprom instead of a hard coded value
	Heater& heater0 = getExtruderBoard(0).getExtruderHeater();
	Heater& heater1 = getExtruderBoard(1).getExtruderHeater();

	if ( heater0.isHeating() || heater1.isHeating() || getPlatformHeater().isHeating() ) {

		if ( getPlatformHeater().isHeating() ) {
			deltaTemp = getPlatformHeater().getDelta()*2;
			setTemp = (int16_t)(getPlatformHeater().get_set_temperature())*2;
			top_temp = 260;
		}
		else {
			/// clear extruder paused states if needed
			if ( heater0.isPaused() ) heater0.Pause(false);
			if ( heater1.isPaused() ) heater1.Pause(false);
		}
		if ( heater0.isHeating() && !heater0.isPaused() )
		{
			deltaTemp += heater0.getDelta();
			setTemp += (int16_t)(heater0.get_set_temperature());
			top_temp += 260;
		}
		if ( heater1.isHeating() && !heater1.isPaused() ) {

			deltaTemp += heater1.getDelta();
			setTemp += (int16_t)(heater1.get_set_temperature());
			top_temp += 120;
		}

		if ( setTemp < deltaTemp )
			div_temp = (top_temp - setTemp);
		else
			div_temp = setTemp;

		if ( div_temp != 0 ) {
			if( !heating_lights_active ) {
#if (BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_E) && defined(HAS_RGB_LED)
				if ( eeprom::heatLights() )
					RGB_LED::clear();
#endif
				heating_lights_active = true;
			}
#ifdef HAS_RGB_LED
			if (RGB_LED::LEDEnabled)
				SET_COLOR((255*abs((setTemp - deltaTemp)))/div_temp, 0,
						  (255*deltaTemp)/div_temp, false);
#endif
		}
	}
	else {
		if ( heating_lights_active ) {
			heating_lights_active = false;
			// Always play tune while not printing, only play during print if HEAT_BUZZ is on or while paused.
			if ( eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS + buzz_eeprom_offsets::HEAT_BUZZ_OFFSET,
				DEFAULT_BUZZ_HEAT) ||
			     host::getHostState() == host::HOST_STATE_READY ||
			     command::pauseState() == PAUSE_STATE_PAUSED )
				Piezo::playTune(TUNE_FILAMENT_START);
#ifdef HAS_RGB_LED
			RGB_LED::setDefaultColor();
#endif
		}
	}
}

bool connectionsErrorTriggered = false;
void Motherboard::heaterFail(HeaterFailMode mode, uint8_t slave_id) {

	// record heat fail mode
	heatFailMode = mode;

	if ( heatFailMode == HEATER_FAIL_NOT_PLUGGED_IN ) {

		// MBI's code has a design flaw whereby it manages all heaters, whether they exist or
		// not and even if they are disabled!  Thus, the Heater::manage_temperature() routine
		// will attempt termperature reads on disabled heaters.  That, in turn, leads to failures
		// on non-existent or otherwise disabled heaters.  And when they fail, this routine is called.
		// Thus this routine ends up having to decide whether a failure is a false alarm or not.
		// The logic for doing that is non-trivial.  But, more importantly, such logic should not
		// have to exist: the manage routine shouldn't be trying to manage non-existent or disabled
		// heaters!

		// It would seem that some of the code in this routine is the non-trivial logic trying to
		// figure out whether or not a heater error should be ignored.  Thus, some of the code
		// here is a work around to the deeper problem.  Worse yet, there's an "if" test below
		// which is simply incorrect: it allows a Rep 2 (single heater) to keep on running when
		// its single heater fails with a "not plugged in" error:
		//
		//    !platform_heater.has_failed() == !false == true  // no heated platform on Rep 2
		//    eeprom::isSingleTool() == true                   // Rep 2 is single tool
		//    !(Extruder_One...has_failed() && Extruder_Two...has_failed()) == !(true && false) == !(false) == true
		//      ^^^ By the above, BOTH heaters have to fail on a Rep 2, but a Rep 2 has only one heater
		//
		// Net result of the above logic is to always ignore a "not plugged in" error on a Rep 2 *unless*
		// the management routines just happen to also trigger an error on the non-existent HBP or
		// non-existent 2nd extruder.
#if 0
		// BEGIN MBI's original comment
		// if single tool, one heater is not plugged in on purpose
		// do not trigger a heatFail message unless both heaters are unplugged
		if ( !platform_heater.has_failed() && singleTool &&
			(!(Extruder_One.getExtruderHeater().has_failed() && Extruder_Two.getExtruderHeater().has_failed())) )
			return;
		// only fire the heater not connected error once.  The user should be able to dismiss this one
		else
#endif
		if ( connectionsErrorTriggered )
			return;
		else
			connectionsErrorTriggered = true;
	}

	// flag heat shutdown response
	heatShutdown = slave_id + 1;  // slave_ids are 0 = tool 0, 1 = tool 1, 2 = platform
}

// Motherboard class waits for a button press from the user
// used for firmware initiated error reporting
void Motherboard::startButtonWait(){
	// blink the interface LEDs
	interfaceBlink(25,15);

	interfaceBoard.waitForButton(0xFF);
	buttonWait = true;
}

// set an error message on the interface and wait for user button press
void Motherboard::errorResponse(const prog_uchar *msg, bool reset, bool incomplete) {
	errorResponse(msg, 0, reset, incomplete);
}

void Motherboard::errorResponse(const prog_uchar *msg1, const prog_uchar *msg2,
				bool reset, bool incomplete) {
	interfaceBoard.errorMessage(msg1, msg2, incomplete);
	startButtonWait();
	reset_request = reset;
}

bool triggered = false;
bool extruder_update = false;

// main motherboard loop
void Motherboard::runMotherboardSlice() {
	bool interface_updated = false;

	// check for user button press
	// update interface screen as necessary
	if ( hasInterfaceBoard ) {
		interfaceBoard.doInterrupt();
		// stagger motherboard updates so that they do not all occur on the same loop
		if ( interface_update_timeout.hasElapsed() ) {
			interfaceBoard.doUpdate();
			interface_update_timeout.start(interfaceBoard.getUpdateRate());
			interface_updated = true;
		}
	}

#if defined(SAMPLE_INTERVAL_MICROS_THERMISTOR)
	if ( isUsingPlatform() && platform_timeout.hasElapsed() ) {
		// manage heating loops for the HBP
		platform_heater.manage_temperature();
		platform_timeout.start(SAMPLE_INTERVAL_MICROS_THERMISTOR);
	}
#endif

	// if waiting on button press
	if ( buttonWait ) {
		// if user presses enter
		if ( interfaceBoard.buttonPushed() ) {

			// set interface LEDs to solid
			interfaceBlink(0,0);

#ifdef HAS_RGB_LED
			// restore default LED behavior
			RGB_LED::setDefaultColor();
#endif

			//clear error messaging
			buttonWait = false;
			interfaceBoard.popScreen();

			if ( reset_request )
				host::stopBuildNow();
			reset_request = false;
			triggered = false;
		}
	}

	// if no user input for USER_INPUT_TIMEOUT, shutdown heaters and warn user
	// don't do this if a heat failure has occured
	// ( in this case heaters are already shutdown and separate error messaging used)
	if ( user_input_timeout.hasElapsed() &&
	     !heatShutdown &&
	     (host::getHostState() != host::HOST_STATE_BUILDING_FROM_SD) &&
	     (host::getHostState() != host::HOST_STATE_BUILDING) ) {

		BOARD_STATUS_SET(STATUS_HEAT_INACTIVE_SHUTDOWN);
		BOARD_STATUS_CLEAR(STATUS_PREHEATING);

		// alert user if heaters are not already set to 0
		if ( (Extruder_One.getExtruderHeater().get_set_temperature() > 0) ||
		     (Extruder_Two.getExtruderHeater().get_set_temperature() > 0) ||
		     (platform_heater.get_set_temperature() > 0) ) {
			interfaceBoard.errorMessage(HEATER_INACTIVITY_MSG, false);
			startButtonWait();
#ifdef HAS_RGB_LED
			// turn LEDs blue
			RGB_LED::setColor(0, 0, 255);
#endif
		}
		// set tempertures to 0
		heatersOff(true);

		// clear timeout
		user_input_timeout.clear();
	}

	// respond to heatshutdown.  response only needs to be called once
	if ( heatShutdown && !triggered && !Piezo::isPlaying() ) {
		triggered = true;

		if ( heatShutdown == 1 )
		     Extruder_One.getExtruderHeater().disable(true);
		else if ( heatShutdown == 2)
		     Extruder_Two.getExtruderHeater().disable(true);
		else
		     platform_heater.disable(true);

		// rgb led response
		interfaceBlink(10,10);

		const prog_uchar *msg, *msg2 = 0;
		if ( heatShutdown < 3 ) {
			if ( singleTool ) msg = HEATER_TOOL_MSG;
			else if ( heatShutdown == 1 ) msg = HEATER_TOOL0_MSG;
			else msg = HEATER_TOOL1_MSG;
		}
		else msg = HEATER_PLATFORM_MSG;

		/// error message
		switch (heatFailMode) {
		case HEATER_FAIL_SOFTWARE_CUTOFF:
		        msg2 = HEATER_FAIL_SOFTWARE_CUTOFF_MSG;
			break;
		case HEATER_FAIL_NOT_HEATING:
		        msg2 = HEATER_FAIL_NOT_HEATING_MSG;
			break;
		case HEATER_FAIL_DROPPING_TEMP:
		        msg2 = HEATER_FAIL_DROPPING_TEMP_MSG;
			break;
		case HEATER_FAIL_NOT_PLUGGED_IN:
			errorResponse(msg, HEATER_FAIL_NOT_PLUGGED_IN_MSG);
			heatShutdown = 0;
			return;
		case HEATER_FAIL_BAD_READS:
			errorResponse(msg, HEATER_FAIL_READ_MSG);
			heatShutdown = 0;
			return;
		default:
			break;
		}
		interfaceBoard.errorMessage(msg, msg2);
		// All heaters off
		heatersOff(true);

		//error sound
		Piezo::playTune(TUNE_ERROR);

#ifdef HAS_RGB_LED
		// blink LEDS red
		RGB_LED::errorSequence();
#endif

		// disable command processing and steppers
		host::heatShutdown();
		command::heatShutdown();
		steppers::abort();
		steppers::enableAxes(0xff, false);
	}

	// Temperature monitoring thread
#if defined(USE_THERMOCOUPLE_DUAL)
	if ( extruder_manage_timeout.hasElapsed() && !interface_updated ) {
	     uint8_t retval = therm_sensor.update();
	     if ( retval != THERM_ADC_BUSY ) {
		  extruder_manage_timeout.start(SAMPLE_INTERVAL_MICROS_THERMOCOUPLE);
		  if ( retval == THERM_READY ) {
		       switch (therm_sensor.getLastUpdated()) {

			    // Right extruder (Tool 0)
		       case THERM_CHANNEL_ONE:
			    Extruder_One.runExtruderSlice();
			    HeatingAlerts();
			    break;

			    // Left extruder (Tool 1)
		       case THERM_CHANNEL_TWO:
			    Extruder_Two.runExtruderSlice();
			    break;

			    // Next case doesn't occur on a Rep 2
		       case THERM_CHANNEL_HBP:
			    if ( isUsingPlatform() )
				 platform_heater.manage_temperature();
			    break;

			    // Cold junction read on a Rep 2
		       default:
			    break;
		       }
		  }
	     }
	}
#else
	// stagger mid accounts for the case when we've just run the interface update
	if ( extruder_manage_timeout.hasElapsed() && !interface_updated ) {
		Extruder_One.runExtruderSlice();
		HeatingAlerts();
		extruder_manage_timeout.start(SAMPLE_INTERVAL_MICROS_THERMOCOUPLE);
		extruder_update = true;
	}
	else if (extruder_update) {
	        Extruder_Two.runExtruderSlice();
		extruder_update = false;
	}
#endif
}

// reset user timeout to start from zero
void Motherboard::resetUserInputTimeout(){
	user_input_timeout.start(USER_INPUT_TIMEOUT);
}

/// Timer three comparator match interrupt
ISR(STEPPER_TIMERn_COMPA_vect) {
	Motherboard::getBoard().doStepperInterrupt();
}

#if defined(PSTOP_SUPPORT) && defined(PSTOP_PORT) && defined(PSTOP_VECT)

ISR(PSTOP_VECT) {
     if ( (pstop_enabled) && (PSTOP_PORT.getValue() == pstop_value) ) command::pstop_triggered = true;
}

#endif

#if defined(PSTOP_SUPPORT) && defined(PSTOP2_PORT) && defined(PSTOP2_VECT)

ISR(PSTOP2_VECT) {
     if ( PSTOP2_PORT.getValue() == 0 ) command::pstop_triggered = true;
}

#endif

/// Number of times to blink the debug LED on each cycle
volatile uint8_t blink_count = 0;

/// number of cycles to hold on and off in each interface LED blink
uint8_t interface_on_time = 0;
uint8_t interface_off_time = 0;

/// The current state of the debug LED
enum {
	BLINK_NONE = 0,
	BLINK_ON,
	BLINK_OFF
};

/// state trackers for blinking LEDS
static uint8_t interface_blink_state = BLINK_NONE;

// set on / off period for blinking interface LEDs
// if both times are zero, LEDs are full on, if just on-time is zero, LEDs are full OFF
void Motherboard::interfaceBlink(uint8_t on_time, uint8_t off_time) {
	if ( off_time == 0 ) {
		interface_blink_state = BLINK_NONE;
		interfaceBoard.setLED(true);
	}
	else if ( on_time == 0 ) {
		interface_blink_state = BLINK_NONE;
		interfaceBoard.setLED(false);
	}
	else {
		interface_on_time = on_time;
		interface_off_time = off_time;
		interface_blink_state = BLINK_ON;
	}
}

#ifdef JKN_ADVANCE
/// Timer 2 extruder advance
ISR(ADVANCE_TIMERn_COMPA_vect) {
	steppers::doExtruderInterrupt();
}
#endif

/// Number of overflows remaining on the current overflow blink cycle
uint8_t interface_ovfs_remaining = 0;
uint8_t blink_overflow_counter = 0;

volatile micros_t m2;

#if defined(HBP_SOFTPWM) && defined(__AVR_ATmega2560__)
volatile uint8_t pwmcnt = 0;
#define PWM_H   210
#define PWM_COUST   255
#endif

/// Timer 5 overflow interrupt
ISR(TIMER5_COMPA_vect) {
#if defined(COOLING_FAN_PWM)
     static uint8_t fan_pwm_counter = 255;
#endif

     // Motherboard::getBoard().UpdateMicros();
     if ( ++centa_micros == 0 ) ++clock_wrap;
     if ( ++mcount >= 10000 ) {
	  seconds += 1;
	  mcount = 0;
     }

#if defined(HBP_SOFTPWM) && defined(__AVR_ATmega2560__)
     //softpwm
    if (pwmcnt < PWM_H) {
        HBP_HEAT.setValue(true);
    } else {
        HBP_HEAT.setValue(false);
    }
    if (pwmcnt >= PWM_COUST)
        pwmcnt = 0;
    else
        pwmcnt++;
#endif

#if defined(COOLING_FAN_PWM)
    if ( fan_pwm_enable ) {
	 // fan_pwm_counter is uint8_t
	 //   we expect it to wrap such that 255 + 1 ==> 0
	 if ( ++fan_pwm_counter == 0 )
	      fan_pwm_counter = 256 - (1 << FAN_PWM_BITS);
	 EX_FAN.setValue(fan_pwm_counter <= fan_pwm_bottom_count);
    }
#endif

	if (blink_overflow_counter++ <= 0xA4)
	     return;

	blink_overflow_counter = 0;

#ifndef BROKEN_SD
	/// Check SD Card Detect
	if ( SD_DETECT_PIN.getValue() != 0x00 ) sdcard::mustReinit = true;
#endif

#if defined(PSTOP_SUPPORT)
#if defined(PSTOP_PORT) && !defined(PSTOP_VECT)
	if ( (pstop_enabled) && (PSTOP_PORT.getValue() == pstop_value) ) command::pstop_triggered = true;
#endif
#if defined(PSTOP2_PORT) && !defined(PSTOP2_VECT)
	if ( PSTOP2_PORT.getValue() == 0 ) command::pstop_triggered = true;
#endif

#if defined(PSTOP_ZMIN_LEVEL) && defined(Z_MIN_STOP_PORT) && defined(AUTO_LEVEL)
        if ( (Z_MIN_STOP_PORT.getValue() == 0) ) {
	     // 40 ticks of zprobe low is 1 second
	     if ( ++total_zprobe_triggered >= 40 ) {
		  command::possibleZLevelPStop();
		  total_zprobe_triggered = 0;
		  // set clear trigger threshold to current zpos plus 4mm
		  zprobe_trigger_clear_zpos = steppers::getPlannerPosition()[Z_AXIS] + 
		       stepperAxisMMToSteps(ALEVEL_ZPROBE_HITS_RESET_MM, Z_AXIS);
	     }
        }
        else {
	     if (steppers::getPlannerPosition()[Z_AXIS] > zprobe_trigger_clear_zpos) command::zprobe_hits = 0;
        }
#endif

#if defined(PSTOP_2_SUPPORT)
	Y_MIN_STOP_PORT.setValue(extrusion_seen[0]);
	extrusion_seen[0] = false;
#if EXTRUDERS > 1
	Z_MAX_STOP_PORT.setValue(extrusion_seen[1]);
	extrusion_seen[1] = false;
#endif
#endif
#endif

	/// Interface Board LEDs
	if ( interface_blink_state != BLINK_NONE ) {
		if ( interface_ovfs_remaining != 0 )
			interface_ovfs_remaining--;
		else if ( interface_blink_state == BLINK_ON ) {
			interface_blink_state = BLINK_OFF;
			interface_ovfs_remaining = interface_on_time;
			Motherboard::getBoard().getInterfaceBoard().setLED(true);
		}
		else if ( interface_blink_state == BLINK_OFF ) {
			interface_blink_state = BLINK_ON;
			interface_ovfs_remaining = interface_off_time;
			Motherboard::getBoard().getInterfaceBoard().setLED(false);
		}
	}
}

void Motherboard::setUsingPlatform(bool is_using) {
  using_platform = is_using;
}

#if defined(COOLING_FAN_PWM)

void Motherboard::setExtra(bool on) {
     uint16_t fan_pwm; // Will be multiplying 8 bits by 100(decimal)

     // Disable any fan PWM handling in Timer 5
     fan_pwm_enable = false;

     if ( !on ) {
	  EXTRA_FET.setValue(false);
	  return;
     }

     // See what the PWM setting is -- may have been changed
     ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	  fan_pwm = (uint16_t)eeprom::getEeprom8(eeprom_offsets::COOLING_FAN_DUTY_CYCLE,
						 COOLING_FAN_DUTY_CYCLE_DEFAULT);
     }

     // Don't bother with PWM handling if the PWM is >= 100
     // Just turn the fan on full tilt
     if ( fan_pwm >= 100 ) {
	  EXTRA_FET.setValue(true);
	  return;
     }

     // Fan is to be turned on AND we are doing PWM
     // We start the bottom count at 255 - 64 and then wrap
     fan_pwm_enable = true;
     fan_pwm_bottom_count = (255 - (1 << FAN_PWM_BITS)) +
	  (int)(0.5 +  ((uint16_t)(1 << FAN_PWM_BITS) * fan_pwm) / 100.0);
}

#else

void Motherboard::setExtra(bool on) {
     EXTRA_FET.setValue(on);
}

#endif

#if defined(HBP_SOFTPWM) && defined(__AVR_ATmega2560__)
void softpwmHBP(bool on){
    if (on)
    {
       HBP_HEAT.setDirection(true);
    }else{
        HBP_HEAT.setDirection(false);
    }
}
#endif

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value) {
     // This is a bit of a hack to get the temperatures right until we fix our
     // PWM'd PID implementation.  We reduce the MV to one bit, essentially.
     // It works relatively well.
     ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#if defined(HBP_SOFTPWM) && defined(__AVR_ATmega2560__)
	  softpwmHBP(value != 0);
#else
	  HBP_HEAT.setValue(value != 0);
#endif
     }
}

void Motherboard::heatersOff(bool platform)
{
	motherboard.getExtruderBoard(0).getExtruderHeater().Pause(false);
	motherboard.getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
	motherboard.getExtruderBoard(1).getExtruderHeater().Pause(false);
	motherboard.getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
	if ( platform ) motherboard.getPlatformHeater().set_target_temperature(0);
	BOARD_STATUS_CLEAR(Motherboard::STATUS_PREHEATING);
}


void Motherboard::interfaceBlinkOn()
{
	motherboard.interfaceBlink(25, 15);
}

void Motherboard::interfaceBlinkOff()
{
	motherboard.interfaceBlink(0, 0);
}

void Motherboard::pauseHeaters(bool pause)
{
	motherboard.getExtruderBoard(0).getExtruderHeater().Pause(pause);
	motherboard.getExtruderBoard(1).getExtruderHeater().Pause(pause);
}

#ifdef DEBUG_VALUE

#if 0
/// Get the current error code.
uint8_t Motherboard::getCurrentError() {
	return blink_count;
}
#endif

//Sets the debug leds to value
//This is in C, as we don't want C++ causing issues
//Note this is quite slow.

void setDebugValue(uint8_t value) {
        DEBUG_PIN1.setValue(value & 0x01);
        DEBUG_PIN2.setValue(value & 0x02);
        DEBUG_PIN3.setValue(value & 0x04);
        DEBUG_PIN4.setValue(value & 0x08);
        DEBUG_PIN5.setValue(value & 0x10);
        DEBUG_PIN6.setValue(value & 0x20);
#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_E || BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
        DEBUG_PIN7.setValue(value & 0x40);
        DEBUG_PIN8.setValue(value & 0x80);
#endif
}

#endif
