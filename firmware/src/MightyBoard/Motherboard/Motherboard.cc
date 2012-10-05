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
#include "Planner.hh"
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
#include "TemperatureTable.hh"
#ifdef MODEL_REPLICATOR
#include "Cutoff.hh"
Cutoff cutoff;
#endif

/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
      lcd(LCD_STROBE, LCD_DATA, LCD_CLK),
      interfaceBoard(buttonArray, lcd, INTERFACE_LED_ONE, INTERFACE_LED_TWO),
      platform_thermistor(PLATFORM_PIN, TemperatureTable::table_thermistor),
			platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,
     	eeprom_offsets::T0_DATA_BASE + toolhead_eeprom_offsets::HBP_PID_BASE, false), //TRICKY: HBP is only and anways on T0 for this machine
			using_platform(eeprom::getEeprom8(eeprom_offsets::HBP_PRESENT, 1)),
#ifdef MODEL_REPLICATOR2
			Extruder_One(0, EXA_PWR, EXA_FAN, ThermocoupleReader::CHANNEL_ONE, eeprom_offsets::T0_DATA_BASE),
			Extruder_Two(1, EXB_PWR, EXB_FAN, ThermocoupleReader::CHANNEL_TWO, eeprom_offsets::T1_DATA_BASE),
			therm_sensor(THERMOCOUPLE_DO,THERMOCOUPLE_SCK,THERMOCOUPLE_DI, THERMOCOUPLE_CS)		
#else
      Extruder_One(0, EX1_PWR, EX1_FAN, THERMOCOUPLE_CS1,eeprom_offsets::T0_DATA_BASE),
      Extruder_Two(1, EX2_PWR, EX2_FAN, THERMOCOUPLE_CS2,eeprom_offsets::T1_DATA_BASE)
#endif
{
}

void Motherboard::initClocks(){

  // set up piezo timer
  Piezo::shutdown_timer();
		
	// Reset and configure timer 5,  stepper
	// interrupt timer.
	TCCR5A = 0x00;
	TCCR5B = 0x09; // no prescaling
	TCCR5C = 0x00;
	OCR5A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK5 = 0x02; // turn on OCR5A match interrupt
	
	// Reset and configure timer 2, the microsecond timer and debug LED flasher timer.
	TCCR2A = 0x00;  
	TCCR2B = 0x0A; /// prescaler at 1/8
	OCR2A = INTERVAL_IN_MICROSECONDS;  // TODO: update PWM settings to make overflowtime adjustable if desired : currently interupting on overflow
	OCR2B = 0;
	TIMSK2 = 0x02; // turn on OCR5A match interrupt

#ifdef MODEL_REPLICATOR2
	// reset and configure timer 3, the Extruders timer
	// Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	TCCR3A = 0b00000001;  
	TCCR3B = 0b00001011; /// set to PWM mode
	OCR3A = 0;
	OCR3C = 0;
	TIMSK3 = 0b00000000; // no interrupts needed
#else
  // reset and configure timer 1, the Extruder Two PWM timer
  // Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
  // Prescaler: 1/64 (250 KHz)
  TCCR1A = 0b00000001;  
  TCCR1B = 0b00001011; /// set to PWM mode
  OCR1A = 0;
  OCR1B = 0;
  TIMSK1 = 0b00000000; // no interrupts needed
  
  // reset and configure timer 4, the Extruder One PWM timer
  // Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
  // Prescaler: 1/64 (250 KHz)
  TCCR4A = 0b00000001;  
  TCCR4B = 0b00001011; /// set to PWM mode
  OCR4A = 0;
  OCR4B = 0;
  TIMSK4 = 0b00000000; // no interrupts needed
#endif
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

	if (hasInterfaceBoard) {
		// Make sure our interface board is initialized
        interfaceBoard.init();

        // start with welcome script if the first boot flag is not set
        if(eeprom::getEeprom8(eeprom_offsets::FIRST_BOOT_FLAG, 0) == 0)
            interfaceBoard.pushScreen(&welcomeScreen);
        else
            // otherwise start with the splash screen.
            interfaceBoard.pushScreen(&splashScreen);     
        
        if(hard_reset)
          _delay_us(3000000);


        // Finally, set up the interface
        interface::init(&interfaceBoard, &lcd);

        interface_update_timeout.start(interfaceBoard.getUpdateRate());
  }
  
  // interface LEDs default to full ON
  interfaceBlink(0,0);
  
  // only call the piezo buzzer on full reboot start up
  // do not clear heater fail messages, the user should not be able to soft reboot from heater fail
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
		
		RGB_LED::init();
		
		Piezo::playTune(TUNE_STARTUP);
		
		heatShutdown = false;
		heatFailMode = HEATER_FAIL_NONE;
#ifdef MODEL_REPLICATOR
    cutoff.init();
#endif
  } 	
  
  board_status = STATUS_NONE;
 
#ifdef MODEL_REPLICATOR2 
  therm_sensor.init();
	therm_sensor_timeout.start(THERMOCOUPLE_UPDATE_RATE);
	// turn off the active cooling fan
	setExtra(false);  
#else
  extruder_manage_timeout.start(SAMPLE_INTERVAL_MICROS_THERMOCOUPLE);
#endif
  
  // initialize the extruders
  Extruder_One.reset();
  Extruder_Two.reset();
    
	HBP_HEAT.setDirection(true);
	platform_thermistor.init();
	platform_heater.reset();
    
  Extruder_One.getExtruderHeater().set_target_temperature(0);
	Extruder_Two.getExtruderHeater().set_target_temperature(0);
	platform_heater.set_target_temperature(0);	
	platform_timeout.start(SAMPLE_INTERVAL_MICROS_THERMISTOR);
	
	// disable extruder two if sigle tool machine
	Extruder_Two.getExtruderHeater().disable(eeprom::isSingleTool());
	
	// disable platform heater if no HBP
	platform_heater.disable(!eeprom::hasHBP());
	
	RGB_LED::setDefaultColor(); 
	buttonWait = false;	
	currentTemp = 0;
  setTemp = 0; 
  heating_lights_active = false;
  progress_active = false;
  progress_line = 0;
  progress_start_char = 0;
  progress_end_char = 0;
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
void Motherboard::doInterrupt() {

	//micros += INTERVAL_IN_MICROSECONDS;
	// Do not move steppers if the board is in a paused state
	if (command::isPaused()) return;
	steppers::doInterrupt();
	
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
void Motherboard::errorResponse(char msg[], bool reset){
	interfaceBoard.errorMessage(msg);
	startButtonWait();
  Piezo::playTune(TUNE_ERROR);
	reset_request = reset;
}


void Motherboard::setBoardStatus(status_states state, bool on){

	if (on){
		board_status |= state;
	}else{
		board_status &= ~state;
	}
}


bool Motherboard::isHeating(){

	return getExtruderBoard(0).getExtruderHeater().isHeating() || getExtruderBoard(1).getExtruderHeater().isHeating() ||
                getPlatformHeater().isHeating();

}

void Motherboard::HeatingAlerts(){
    
    setTemp = 0;
    currentTemp = 0;
    
    /// show heating progress
    if(isHeating()){
        if(getPlatformHeater().isHeating()){
            currentTemp += getPlatformHeater().getDelta()*2;
            setTemp += (int16_t)(getPlatformHeater().get_set_temperature())*2;
        }else{
          /// clear extruder paused states if needed
          if(getExtruderBoard(0).getExtruderHeater().isPaused()){getExtruderBoard(0).getExtruderHeater().Pause(false);}
          if(getExtruderBoard(1).getExtruderHeater().isPaused()){getExtruderBoard(1).getExtruderHeater().Pause(false);}
        } 
        if(getExtruderBoard(0).getExtruderHeater().isHeating()  && !getExtruderBoard(0).getExtruderHeater().isPaused()){
            currentTemp += getExtruderBoard(0).getExtruderHeater().getDelta();
            setTemp += (int16_t)(getExtruderBoard(0).getExtruderHeater().get_set_temperature());
        }
        if(getExtruderBoard(1).getExtruderHeater().isHeating() && !getExtruderBoard(1).getExtruderHeater().isPaused()){
            currentTemp += getExtruderBoard(1).getExtruderHeater().getDelta();
            setTemp += (int16_t)(getExtruderBoard(1).getExtruderHeater().get_set_temperature());
        }
             
		if((setTemp != 0) && eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::LED_HEAT_OFFSET, 1)
          && (eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, LED_DEFAULT_OFF) != LED_DEFAULT_OFF)){
			int32_t mult = 255;
			if(!heating_lights_active){
#ifdef MODEL_REPLICATOR
        RGB_LED::clear();
#endif
				heating_lights_active = true;
			}
			RGB_LED::setColor((mult*(setTemp - currentTemp))/setTemp, 0, (mult*currentTemp)/setTemp, false);
		}
	}else{
		if(heating_lights_active){
			RGB_LED::setDefaultColor();
			heating_lights_active = false;
		}
	}
	if(progress_active){
		progress_last_index = HeatProgressBar(progress_line, progress_start_char, progress_end_char, progress_last_index);
	}
	
}
void Motherboard::StartProgressBar(uint8_t line, uint8_t start_char, uint8_t end_char){
	progress_active = true;
	progress_line = line;
	progress_start_char = start_char;
	progress_end_char = end_char;
	progress_last_index = 0;
}
void Motherboard::StopProgressBar(){

	progress_active = false;
	// clear the progress Bar
	lcd.setCursor(progress_start_char,progress_line);
	for(uint8_t i = progress_start_char; i < progress_end_char; i++){ 
		lcd.writeString(" ");
	}
}


uint8_t Motherboard::HeatProgressBar(uint8_t line, uint8_t start_char, uint8_t end_char, uint8_t lastHeatIndex){
	uint8_t heatIndex = 0;
	
	if((start_char > end_char) || (lastHeatIndex > (end_char - start_char))){
		return 0;
	}
		
	if(setTemp > 0){
		heatIndex = (abs((setTemp - currentTemp)) * (end_char - start_char)) / setTemp;		
	}
	if (lastHeatIndex > heatIndex){
		lcd.setCursor(start_char,line);
		for(uint8_t i = start_char; i < end_char; i++){ 
			lcd.writeString(" ");
		}
		lastHeatIndex = 0;
	}
		
	lcd.setCursor(start_char + lastHeatIndex,line);
	for (int i = lastHeatIndex; i < heatIndex; i++)
		lcd.write(0xFF);
	lastHeatIndex = heatIndex;
	
	toggleBlink = !toggleBlink;
	if(toggleBlink)
		lcd.writeFromPgmspace(BLANK_CHAR_MSG);
	else
		lcd.write(0xFF);
		
	return heatIndex;
}

bool extruder_update = false;
bool triggered = false;
// main motherboard loop
void Motherboard::runMotherboardSlice() {
	
  bool interface_updated = false;
    
    // check for user button press
    // update interface screen as necessary
	if (hasInterfaceBoard) {
		interfaceBoard.doInterrupt();
		// stagger motherboard updates so that they do not all occur on the same loop
		if (interface_update_timeout.hasElapsed()){
			interfaceBoard.doUpdate();
			interface_update_timeout.start(interfaceBoard.getUpdateRate());
      interface_updated = true;
		}
	}
			   
   if(isUsingPlatform() && platform_timeout.hasElapsed()) {
		// manage heating loops for the HBP
		platform_heater.manage_temperature();
		platform_timeout.start(SAMPLE_INTERVAL_MICROS_THERMISTOR);
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
			interfaceBoard.DoneWithMessage();
			if(reset_request)
				host::stopBuild();
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
		board_status &= ~STATUS_PREHEATING;
		
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
       
		/// error message
		switch (heatFailMode){
			case HEATER_FAIL_SOFTWARE_CUTOFF:
				interfaceBoard.errorMessage(HEATER_FAIL_SOFTWARE_CUTOFF_MSG);
				break;
			case HEATER_FAIL_NOT_HEATING:
				interfaceBoard.errorMessage(HEATER_FAIL_NOT_HEATING_MSG);
				break;
			case HEATER_FAIL_DROPPING_TEMP:
				interfaceBoard.errorMessage(HEATER_FAIL_DROPPING_TEMP_MSG);
				break;
			case HEATER_FAIL_BAD_READS:
				errorResponse(HEATER_FAIL_READ_MSG);
        heatShutdown = false;
        return;
			case HEATER_FAIL_NOT_PLUGGED_IN:
				errorResponse(HEATER_FAIL_NOT_PLUGGED_IN_MSG);
				/// turn off whichever heater has failed
				if(Extruder_One.getExtruderHeater().has_failed()){
					Extruder_One.getExtruderHeater().set_target_temperature(0);
				} if (Extruder_Two.getExtruderHeater().has_failed()){
					Extruder_Two.getExtruderHeater().set_target_temperature(0);
				} if (platform_heater.has_failed()){
					platform_heater.set_target_temperature(0);
				}
        heatShutdown = false;
        return;
		}
		
		// set all heater temperatures to zero
    Extruder_One.getExtruderHeater().set_target_temperature(0);
		Extruder_Two.getExtruderHeater().set_target_temperature(0);
		platform_heater.set_target_temperature(0);
		
    //error sound
    Piezo::playTune(TUNE_ERROR);
    // blink LEDS red
		RGB_LED::errorSequence();
		// disable command processing and steppers
		host::heatShutdown();
		command::heatShutdown();
		planner::abort();
    for(int i = 0; i < STEPPER_COUNT; i++)
			steppers::enableAxis(i, false);
	}
	
#ifdef MODEL_REPLICATOR2
	if(therm_sensor_timeout.hasElapsed() && !interface_updated){
		bool success = therm_sensor.update();
		if (success){
			therm_sensor_timeout.start(THERMOCOUPLE_UPDATE_RATE);
			switch (therm_sensor.getLastUpdated()){
				case ThermocoupleReader::CHANNEL_ONE:
					Extruder_One.runExtruderSlice();
					HeatingAlerts();
					break;
				case ThermocoupleReader::CHANNEL_TWO:
					Extruder_Two.runExtruderSlice();
					break;
				default:
					break;
			}
		}
	}
#else 
  if(extruder_manage_timeout.hasElapsed() && !interface_updated){
    Extruder_One.runExtruderSlice();
    HeatingAlerts();
    extruder_manage_timeout.start(SAMPLE_INTERVAL_MICROS_THERMOCOUPLE);
    // we are using extruer_update and interface_updated to stagger update loops
    // this is desireable for limiting time spent in the motherboard loop
    extruder_update = true;
  }else if (extruder_update){
    Extruder_Two.runExtruderSlice();
    extruder_update = false;
  }
#endif

}

// reset user timeout to start from zero
void Motherboard::resetUserInputTimeout(){
	user_input_timeout.start(USER_INPUT_TIMEOUT);
}

#define MICROS_INTERVAL 128

void Motherboard::UpdateMicros(){
	micros += MICROS_INTERVAL;//_IN_MICROSECONDS;

}


/// Timer three comparator match interrupt
ISR(TIMER5_COMPA_vect) {
	Motherboard::getBoard().doInterrupt();
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

uint16_t blink_overflow_counter = 0;

/// Timer 2 overflow interrupt
ISR(TIMER2_COMPA_vect) {
	
	Motherboard::getBoard().UpdateMicros();
	
	if(blink_overflow_counter++ <= 0x080)
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

void Motherboard::setExtra(bool on) {
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      //setUsingPlatform(false);
      //pwmHBP_On(false);
      EX_FAN.setDirection(true);
      EX_FAN.setValue(on);
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


