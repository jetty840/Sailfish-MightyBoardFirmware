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
#include "SoftI2cManager.hh"
#include "Piezo.hh"
#include "RGB_LED.hh"
#include "Errors.hh"
#include <avr/eeprom.h>
#include <util/delay.h>
#include "Menu_locales.hh"


//Warnings to remind us that certain things should be switched off for release

#ifdef ERASE_EEPROM_ON_EVERY_BOOT
	#warning "Release: ERASE_EEPROM_ON_EVERY_BOOT enabled in Configuration.hh"
#endif

#ifdef DEBUG_VALUE
	#warning "Release: DEBUG_VALUE enabled in Configuration.hh"
#endif

#if defined(HONOR_DEBUG_PACKETS) && (HONOR_DEBUG_PACKETS == 1)
	#warning "Release: HONOR_DEBUG_PACKETS enabled in Configuration.hh"
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


/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
        lcd(LCD_STROBE, LCD_DATA, LCD_CLK),
	messageScreen((unsigned char)0),
	mainMenu((unsigned char)0),
        interfaceBoard(buttonArray,
            lcd,
            INTERFACE_GLED,
            INTERFACE_RLED,
            &mainMenu,
            &mainMenu.utils.monitorMode,
            &messageScreen),
            platform_thermistor(PLATFORM_PIN,0),
            platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,
            		eeprom_offsets::T0_DATA_BASE + toolhead_eeprom_offsets::HBP_PID_BASE, false), //TRICKY: HBP is only and anways on T0 for this machine
			using_platform(true),
			Extruder_One(0, EX1_PWR, EX1_FAN, THERMOCOUPLE_CS1,eeprom_offsets::T0_DATA_BASE),
			Extruder_Two(1, EX2_PWR, EX2_FAN, THERMOCOUPLE_CS2,eeprom_offsets::T1_DATA_BASE)
{
}

void Motherboard::setupAccelStepperTimer() {
        STEPPER_TCCRnA = 0x00;
        STEPPER_TCCRnB = 0x0A; //CTC1 + / 8 = 2Mhz.
        STEPPER_TCCRnC = 0x00;
        STEPPER_OCRnA  = 0x2000; //1KHz
        STEPPER_TIMSKn = 0x02; // turn on OCR3A match interrupt
}

#define ENABLE_TIMER_INTERRUPTS		TIMSK2		|= (1<<OCIE2A); \
                			STEPPER_TIMSKn	|= (1<<STEPPER_OCIEnA)

#define DISABLE_TIMER_INTERRUPTS	TIMSK2		&= ~(1<<OCIE2A); \
                			STEPPER_TIMSKn	&= ~(1<<STEPPER_OCIEnA)

// Initialize Timers
//	0 = Buzzer
//	1 = Extruder 2 (PWM)
//	2 = Microsecond timer, debug LED flasher timer and Advance timer
//	3 = Stepper
//	4 = Extruder 1 (PWM)
//	5 = Debug Timer (unused unless DEBUG_TIMER is defined in StepperAccel.hh)
//
//	Timer 0 = 8 bit with PWM
//	Timers 1,3,4,5 = 16 bit with PWM
//	Timer 2 = 8 bit with PWM

void Motherboard::initClocks(){
	// Reset and configure timer 0, the piezo buzzer timer
	// No interrupt, frequency controlled by Piezo
	Piezo::shutdown_timer();

	// reset and configure timer 1, the Extruder Two PWM timer
	// Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	// No interrupt, PWM controlled by ExtruderBoard
	TCCR1A = 0b00000001;
	TCCR1B = 0b00001011;
	OCR1A = 0x00;
	OCR1B = 0x00;
	TIMSK1 = 0x00;	//No interrupts

	// Reset and configure timer 2, the microsecond timer, debug LED flasher timer and Advance timer.
	// Timer 2 is 8 bit
	TCCR2A = 0x02;	// CTC
	TCCR2B = 0x04;	// prescaler at 1/64
	OCR2A = 25;	//Generate interrupts 16MHz / 64 / 25 = 10KHz
	TIMSK2 = 0x02; // turn on OCR2A match interrupt

	// Reset and configure timer 3, the stepper interrupt timer.
	// ISR(TIMER3_COMPA_vect)
        setupAccelStepperTimer();

	// reset and configure timer 4, the Extruder One PWM timer
	// Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	// No interrupt, PWM controlled by ExtruderBoard
	TCCR4A = 0b00000001;
	TCCR4B = 0b00001011;
	TCCR4C = 0x00;
	OCR4A = 0x00;
	OCR4B = 0x00;
	TIMSK4 = 0x00;	//No interrupts

	// Timer 5 (unused unless DEBUG_TIMER is defined in StepperAccel.hh)
}

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::reset(bool hard_reset) {
	indicateError(0); // turn on blinker

	// Init steppers
	uint8_t axis_invert = eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION, 0);
	SoftI2cManager::getI2cManager().init();
	// Z holding indicates that when the Z axis is not in
	// motion, the machine should continue to power the stepper
	// coil to ensure that the Z stage does not shift.
	// Bit 7 of the AXIS_INVERSION eeprom setting
	// indicates whether or not to use z holding; 
	// the bit is active low. (0 means use z holding,
	// 1 means turn it off.)
	bool hold_z = (axis_invert & (1<<7)) == 0;
	steppers::setHoldZ(hold_z);

	// Initialize the host and slave UARTs
	UART::getHostUART().enable(true);
	UART::getHostUART().in.reset();
	
	micros = 0;
	
	initClocks();
		
	// Check if the interface board is attached
	hasInterfaceBoard = interface::isConnected();
	
	DEBUG_PIN5.setValue(true);

	if (hasInterfaceBoard) {

		// Make sure our interface board is initialized
        interfaceBoard.init();

        // start with welcome script if the first boot flag is not set
        if(eeprom::getEeprom8(eeprom_offsets::FIRST_BOOT_FLAG, 0) == 0)
            interfaceBoard.pushScreen(&mainMenu.utils.welcome);
        else
            // otherwise start with the splash screen.
            interfaceBoard.pushScreen(&mainMenu.utils.splash);
            
        

        // Finally, set up the interface
        interface::init(&interfaceBoard, &lcd);
        
        DEBUG_PIN5.setValue(false);
        
        
        if(hard_reset){
			_delay_ms(3000);
		}

        interface_update_timeout.start(interfaceBoard.getUpdateRate());
    }
    
    
    
    // interface LEDs default to full ON
    interfaceBlink(0,0);
    
    // only call the piezo buzzer on full reboot start up
    // do not clear heater fail messages, though the user should not be able to soft reboot from heater fail
    if(hard_reset)
	{
		// Configure the debug pins.
		DEBUG_PIN.setDirection(true);
		DEBUG_PIN1.setDirection(true);
		DEBUG_PIN2.setDirection(true);
		DEBUG_PIN3.setDirection(true);	
		DEBUG_PIN4.setDirection(true);
		DEBUG_PIN5.setDirection(true);
		DEBUG_PIN6.setDirection(true);
		DEBUG_PIN7.setDirection(true);
		
		RGB_LED::init();
		
		Piezo::playTune(TUNE_STARTUP);
		
		heatShutdown = false;
		heatFailMode = HEATER_FAIL_NONE;
		cutoff.init();
		
		board_status = STATUS_NONE;
    } 	
    
     // initialize the extruders
    Extruder_One.reset();
    Extruder_Two.reset();
    
    HBP_HEAT.setDirection(true);
	platform_thermistor.init();
	platform_heater.reset();
    
    Extruder_One.getExtruderHeater().set_target_temperature(0);
	Extruder_Two.getExtruderHeater().set_target_temperature(0);
	platform_heater.set_target_temperature(0);	
	
	RGB_LED::setDefaultColor(); 
	buttonWait = false;	
}

/// Get the number of microseconds that have passed since
/// the board was booted.
micros_t Motherboard::getCurrentMicros() {
	micros_t micros_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_snapshot = micros;
	}
	return micros_snapshot;
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


bool connectionsErrorTriggered = false;
void Motherboard::heaterFail(HeaterFailMode mode){

    // record heat fail mode
	heatFailMode = mode;
    
	if(heatFailMode == HEATER_FAIL_NOT_PLUGGED_IN)
	{
		// if single tool, one heater is not plugged in on purpose
		// do not trigger a heatFail message unless both heaters are unplugged 
		if(!platform_heater.has_failed() && eeprom::isSingleTool() && 
			(!(Extruder_One.getExtruderHeater().has_failed() && Extruder_Two.getExtruderHeater().has_failed())))
				return;
        // only fire the heater not connected error once.  The user should be able to dismiss this one
		else if (connectionsErrorTriggered)
			return;
		else
			connectionsErrorTriggered =true;
	}
    
    // flag heat shutdown response
	heatShutdown = true;
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
void Motherboard::errorResponse(const prog_uchar msg[], bool reset){
	interfaceBoard.errorMessage(msg);
	startButtonWait();
	reset_request = reset;
}

enum stagger_timers{
	STAGGER_INTERFACE,
	STAGGER_MID, 
	STAGGER_EX2,
	STAGGER_EX1
}stagger = STAGGER_INTERFACE;

uint8_t Motherboard::GetErrorStatus(){

	return board_status;
}


bool triggered = false;
// main motherboard loop
void Motherboard::runMotherboardSlice() {
    // check for user button press
    // update interface screen as necessary
	if (hasInterfaceBoard) {
		interfaceBoard.doInterrupt();
		// stagger motherboard updates so that they do not all occur on the same loop
		if (interface_update_timeout.hasElapsed() && (stagger == STAGGER_INTERFACE)) {
			interfaceBoard.doUpdate();
			interface_update_timeout.start(interfaceBoard.getUpdateRate());
			stagger = STAGGER_MID;
		}
	}

    if(isUsingPlatform()) {
		// manage heating loops for the HBP
		platform_heater.manage_temperature();
	}
	
    // if waiting on button press
	if(buttonWait)
	{
        // if user presses enter
		if (interfaceBoard.buttonPushed()) {
			// set interface LEDs to solid
			interfaceBlink(0,0);
			// restore default LED behavior
			RGB_LED::setDefaultColor();
			//clear error messaging
			buttonWait = false;
			interfaceBoard.popScreen();
			if(reset_request)
				host::stopBuildNow();
			triggered = false;
		}
		
	}

	// if no user input for USER_INPUT_TIMEOUT, shutdown heaters and warn user
    // don't do this if a heat failure has occured ( in this case heaters are already shutdown and separate error messaging used)
	if(user_input_timeout.hasElapsed() && !heatShutdown && (host::getHostState() != host::HOST_STATE_BUILDING_FROM_SD) && (host::getHostState() != host::HOST_STATE_BUILDING))
	{
        // clear timeout
		user_input_timeout.clear();
		
		board_status |= STATUS_HEAT_INACTIVE_SHUTDOWN;
		
		// alert user if heaters are not already set to 0
		if((Extruder_One.getExtruderHeater().get_set_temperature() > 0) ||
			(Extruder_Two.getExtruderHeater().get_set_temperature() > 0) ||
			(platform_heater.get_set_temperature() > 0)){
				interfaceBoard.errorMessage(HEATER_INACTIVITY_MSG);//37
				startButtonWait();
                // turn LEDs blue
				RGB_LED::setColor(0,0,255, true);
		}
        // set tempertures to 0
		Extruder_One.getExtruderHeater().set_target_temperature(0);
		Extruder_Two.getExtruderHeater().set_target_temperature(0);
		platform_heater.set_target_temperature(0);
	}
			   
    // respond to heatshutdown.  response only needs to be called once
	if(heatShutdown && !triggered && !Piezo::isPlaying())
	{
        triggered = true;
		// rgb led response
		interfaceBlink(10,10);
        // set all heater temperatures to zero
        Extruder_One.getExtruderHeater().set_target_temperature(0);
		Extruder_Two.getExtruderHeater().set_target_temperature(0);
		platform_heater.set_target_temperature(0);
		/// error message
		switch (heatFailMode){
			case HEATER_FAIL_SOFTWARE_CUTOFF:
				interfaceBoard.errorMessage(HEATER_FAIL_SOFTWARE_CUTOFF_MSG);//,79);
				break;
			case HEATER_FAIL_NOT_HEATING:
				interfaceBoard.errorMessage(HEATER_FAIL_NOT_HEATING_MSG);//,79);
				break;
			case HEATER_FAIL_DROPPING_TEMP:
				interfaceBoard.errorMessage(HEATER_FAIL_DROPPING_TEMP_MSG);//,79);
				break;
			case HEATER_FAIL_NOT_PLUGGED_IN:
				interfaceBoard.errorMessage(HEATER_FAIL_NOT_PLUGGED_IN_MSG);//,79);
                		startButtonWait();
                		heatShutdown = false;
                		return;
			default:
				break;
		}
        // blink LEDS red
		RGB_LED::errorSequence();
		// disable command processing and steppers
		host::heatShutdown();
		command::heatShutdown();
		steppers::abort();
        for(int i = 0; i < STEPPER_COUNT; i++)
			steppers::enableAxis(i, false);
	}
		       
	// Temperature monitoring thread
	// stagger mid accounts for the case when we've just run the interface update
	if(stagger == STAGGER_MID){
		stagger = STAGGER_EX1;
	}else if(stagger == STAGGER_EX1){
		Extruder_One.runExtruderSlice();
		stagger = STAGGER_EX2;
	}else if (stagger == STAGGER_EX2){
		Extruder_Two.runExtruderSlice();
		stagger = STAGGER_INTERFACE;
	}
}

// reset user timeout to start from zero
void Motherboard::resetUserInputTimeout(){
	user_input_timeout.start(USER_INPUT_TIMEOUT);
}

//Frequency of Timer 2
//100 = (1.0 / ( 16MHz / 64 / 25 = 10KHz)) * 1000000
#define MICROS_INTERVAL 100

void Motherboard::UpdateMicros(){
	micros += MICROS_INTERVAL;	//_IN_MICROSECONDS;
}


/// Timer three comparator match interrupt
ISR(STEPPER_TIMERn_COMPA_vect) {
	Motherboard::getBoard().doStepperInterrupt();
}



/// Number of times to blink the debug LED on each cycle
volatile uint8_t blink_count = 0;

/// number of cycles to hold on and off in each interface LED blink
uint8_t interface_on_time = 0;
uint8_t interface_off_time = 0;

/// The current state of the debug LED
enum {
	BLINK_NONE,
	BLINK_ON,
	BLINK_OFF,
	BLINK_PAUSE
};

/// state trackers for blinking LEDS
int blink_state = BLINK_NONE;
int interface_blink_state = BLINK_NONE;

/// Write an error code to the debug pin.
void Motherboard::indicateError(int error_code) {
	if (error_code == 0) {
		blink_state = BLINK_NONE;
		DEBUG_PIN.setValue(false);
	}
	else if (blink_count != error_code) {
		blink_state = BLINK_OFF;
	}
	blink_count = error_code;
}

// set on / off period for blinking interface LEDs
// if both times are zero, LEDs are full on, if just on-time is zero, LEDs are full OFF
void Motherboard::interfaceBlink(int on_time, int off_time){
	
	if(off_time == 0){
		interface_blink_state = BLINK_NONE;
		interface::setLEDs(true);
	}else if(on_time == 0){
		interface_blink_state = BLINK_NONE;
		interface::setLEDs(false);
	} else{
		interface_on_time = on_time;
		interface_off_time = off_time;
		interface_blink_state = BLINK_ON;
	}

}

/// Get the current error code.
uint8_t Motherboard::getCurrentError() {
	return blink_count;
}

/// Timer2 overflow cycles that the LED remains on while blinking
#define OVFS_ON 18
/// Timer2 overflow cycles that the LED remains off while blinking
#define OVFS_OFF 18
/// Timer2 overflow cycles between flash cycles
#define OVFS_PAUSE 80

/// Number of overflows remaining on the current blink cycle
int blink_ovfs_remaining = 0;
/// Number of blinks performed in the current cycle
int blinked_so_far = 0;
/// Number of overflows remaining on the current overflow blink cycle
int interface_ovfs_remaining = 0;

uint8_t blink_overflow_counter = 0;

volatile micros_t m2;

/// Timer 2 overflow interrupt
ISR(TIMER2_COMPA_vect) {
	Motherboard::getBoard().UpdateMicros();

#ifdef JKN_ADVANCE
	steppers::doExtruderInterrupt();
#endif
	
	if(blink_overflow_counter++ <= 0xA4)
			return;
	
	blink_overflow_counter = 0;
			
	/// Debug LEDS on Motherboard
	if (blink_ovfs_remaining > 0) {
		blink_ovfs_remaining--;
	} else {
		if (blink_state == BLINK_ON) {
			blinked_so_far++;
			blink_state = BLINK_OFF;
			blink_ovfs_remaining = OVFS_OFF;
			DEBUG_PIN.setValue(false);
		} else if (blink_state == BLINK_OFF) {
			if (blinked_so_far >= blink_count) {
				blink_state = BLINK_PAUSE;
				blink_ovfs_remaining = OVFS_PAUSE;
			} else {
				blink_state = BLINK_ON;
				blink_ovfs_remaining = OVFS_ON;
				DEBUG_PIN.setValue(true);
			}
		} else if (blink_state == BLINK_PAUSE) {
			blinked_so_far = 0;
			blink_state = BLINK_ON;
			blink_ovfs_remaining = OVFS_ON;
			DEBUG_PIN.setValue(true);
		}
	}
	/// Interface Board LEDs
	if( interface_ovfs_remaining > 0){
		interface_ovfs_remaining--;
	} else {
		if (interface_blink_state == BLINK_ON) {
			interface_blink_state = BLINK_OFF;
			interface_ovfs_remaining = interface_on_time;
			interface::setLEDs(true);
		} else if (interface_blink_state == BLINK_OFF) {
			interface_blink_state = BLINK_ON;
			interface_ovfs_remaining = interface_off_time;
			interface::setLEDs(false);
		}
	} 

}

void Motherboard::setUsingPlatform(bool is_using) {
  using_platform = is_using;
}

void Motherboard::setValve(bool on) {
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		//setUsingPlatform(false);
		EXTRA_FET.setDirection(true);
		EXTRA_FET.setValue(on);
	}
}

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value) {
	// This is a bit of a hack to get the temperatures right until we fix our
	// PWM'd PID implementation.  We reduce the MV to one bit, essentially.
	// It works relatively well.
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		HBP_HEAT.setValue(value != 0);
	}
  
}


#ifdef DEBUG_VALUE

//Sets the debug leds to value
//This is in C, as we don't want C++ causing issues
//Note this is quite slow.

void setDebugValue(uint8_t value) {
	static bool initialized = false;

	if ( ! initialized ) {
		DEBUG_PIN1.setDirection(true);
		DEBUG_PIN2.setDirection(true);
		DEBUG_PIN3.setDirection(true);
		DEBUG_PIN4.setDirection(true);
		DEBUG_PIN5.setDirection(true);
		DEBUG_PIN6.setDirection(true);
		DEBUG_PIN7.setDirection(true);
		DEBUG_PIN8.setDirection(true);

		initialized = true;
	}

        DEBUG_PIN1.setValue(value & 0x80);
        DEBUG_PIN2.setValue(value & 0x40);
        DEBUG_PIN3.setValue(value & 0x20);
        DEBUG_PIN4.setValue(value & 0x10);
        DEBUG_PIN5.setValue(value & 0x08);
        DEBUG_PIN6.setValue(value & 0x04);
        DEBUG_PIN7.setValue(value & 0x02);
        DEBUG_PIN8.setValue(value & 0x01);
}

#endif
