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


/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
        lcd(LCD_STROBE, LCD_DATA, LCD_CLK),
        interfaceBoard(buttonArray,
            lcd,
            INTERFACE_GLED,
            INTERFACE_RLED,
            &mainMenu,
            &monitorMode),
            platform_thermistor(PLATFORM_PIN,0),
            platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,
            		eeprom_offsets::T0_DATA_BASE + toolhead_eeprom_offsets::HBP_PID_BASE), //TRICKY: HBP is only and anways on T0 for this machine
            //platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,eeprom::HBP_PID_BASE),
			using_platform(true),
			Extruder_One(0, EX1_PWR, EX1_FAN, THERMOCOUPLE_CS1,eeprom_offsets::T0_DATA_BASE),
			Extruder_Two(1, EX2_PWR, EX2_FAN, THERMOCOUPLE_CS2,eeprom_offsets::T1_DATA_BASE)
{
	/// Set up the stepper pins on board creation
#if STEPPER_COUNT > 0
        stepper[0] = StepperInterface(X_DIR_PIN,
                                      X_STEP_PIN,
                                      X_ENABLE_PIN,
                                      X_MAX_PIN,
                                      X_MIN_PIN,
                                      X_POT_PIN,
                                      eeprom_offsets::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 1
        stepper[1] = StepperInterface(Y_DIR_PIN,
                                      Y_STEP_PIN,
                                      Y_ENABLE_PIN,
                                      Y_MAX_PIN,
                                      Y_MIN_PIN,
                                      Y_POT_PIN,
                                      eeprom_offsets::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 2
        stepper[2] = StepperInterface(Z_DIR_PIN,
                                      Z_STEP_PIN,
                                      Z_ENABLE_PIN,
                                      Z_MAX_PIN,
                                      Z_MIN_PIN,
                                      Z_POT_PIN,
                                      eeprom_offsets::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 3
        stepper[3] = StepperInterface(A_DIR_PIN,
                                      A_STEP_PIN,
                                      A_ENABLE_PIN,
                                      Pin(),
                                      Pin(),
                                      A_POT_PIN,
                                      eeprom_offsets::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 4
        stepper[4] = StepperInterface(B_DIR_PIN,
                                      B_STEP_PIN,
                                      B_ENABLE_PIN,
                                      Pin(),
                                      Pin(),
                                      B_POT_PIN,
                                      eeprom_offsets::AXIS_INVERSION);
#endif
}

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::reset() {
	indicateError(2); // turn on blinker

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

	for (int i = 0; i < STEPPER_COUNT; i++) {
		stepper[i].init(i);
	}
	// Initialize the host and slave UARTs
        UART::getHostUART().enable(true);
        UART::getHostUART().in.reset();
    
    Extruder_One.reset();
    Extruder_Two.reset();
    
    // Reset and configure timer 0, the piezo buzzer timer
    // Mode: Phase-correct PWM with OCRnA (WGM2:0 = 101)
	// Prescaler: set on call by piezo function
    TCCR0A = 0;//0b00000011; // default mode off / phase correct piezo   
	TCCR0B = 0b01;//0b00001001; // default pre-scaler 1/1
	OCR0A = 0;
	OCR0B = 0;
	TIMSK0 = 0b00000000; //interrupts default to off   
    
	// Reset and configure timer 3, the microsecond and stepper
	// interrupt timer.
	TCCR3A = 0x00;
	TCCR3B = 0x09;
	TCCR3C = 0x00;
	OCR3A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK3 = 0x02; // turn on OCR3A match interrupt
	
	// Reset and configure timer 2, the debug LED flasher timer.
	TCCR2A = 0x00;
	TCCR2B = 0x07; // prescaler at 1/1024
	TIMSK2 = 0x01; // OVF flag on
	
	// reset and configure timer 5, the HBP PWM timer
	// not currently being used
	TCCR5A = 0b00000000;  
	TCCR5B = 0b00000010; /// set to PWM mode
	OCR5A = 0;
	OCR5B = 0;
	TIMSK5 = 0b00000000; // no interrupts needed
	
	// reset and configure timer 1, the Extruder Two PWM timer
	// Mode: Phase-correct PWM with OCRnA(WGM3:0 = 1011), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	TCCR1A = 0b00000011;  
	TCCR1B = 0b00010011; /// set to PWM mode
	OCR1A = 0;
	OCR1B = 0;
	TIMSK1 = 0b00000000; // no interrupts needed
	
	// reset and configure timer 4, the Extruder One PWM timer
	// Mode: Phase-correct PWM with OCRnA (WGM3:0 = 1011), cycle freq= 976 Hz
	// Prescaler: 1/64 (250 KHz)
	TCCR4A = 0b00000011;  
	TCCR4B = 0b00010011; /// set to PWM mode
	OCR4A = 0;
	OCR4B = 0;
	TIMSK4 = 0b00000000; // no interrupts needed
	
	// Configure the debug pins.
	DEBUG_PIN.setDirection(true);
	DEBUG_PIN1.setDirection(true);
	DEBUG_PIN2.setDirection(true);
	DEBUG_PIN3.setDirection(true);	

	// Check if the interface board is attached
        hasInterfaceBoard = interface::isConnected();

	if (hasInterfaceBoard) {
		// Make sure our interface board is initialized
                interfaceBoard.init();

                // Then add the splash screen to it.
                interfaceBoard.pushScreen(&splashScreen);

                // Finally, set up the *** interface
                interface::init(&interfaceBoard, &lcd);

                interface_update_timeout.start(interfaceBoard.getUpdateRate());
	}
	
	HBP_HEAT.setDirection(true);
	platform_thermistor.init();
	platform_heater.reset();
	cutoff.init();
 //   Piezo::startUpTone();

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
	if (hasInterfaceBoard) {
                interfaceBoard.doInterrupt();
	}
	micros += INTERVAL_IN_MICROSECONDS;
	// Do not move steppers if the board is in a paused state
	if (command::isPaused()) return;
	steppers::doInterrupt();
	
	if(cutoff.isCutoffActive())
	{
		interfaceBoard.setLED(0, true);
		interfaceBoard.setLED(1, true);
		cutoff.noiseResponse();
	}	
}

void Motherboard::runMotherboardSlice() {
	if (hasInterfaceBoard) {
		if (interface_update_timeout.hasElapsed()) {
                        interfaceBoard.doUpdate();
                        interface_update_timeout.start(interfaceBoard.getUpdateRate());
		}
	}
	
	 if(isUsingPlatform()) {
			   platform_heater.manage_temperature();
		}
        
	// Temperature monitoring thread
	Extruder_One.runExtruderSlice();
	Extruder_Two.runExtruderSlice();
}

/// Timer one comparator match interrupt
ISR(TIMER3_COMPA_vect) {
	Motherboard::getBoard().doInterrupt();
}

/// Number of times to blink the debug LED on each cycle
volatile uint8_t blink_count = 0;


/// The current state of the debug LED
enum {
	BLINK_NONE,
	BLINK_ON,
	BLINK_OFF,
	BLINK_PAUSE
} blink_state = BLINK_NONE;

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

/// Timer 2 overflow interrupt
ISR(TIMER2_OVF_vect) {
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
}

void pwmHBP_On(bool on) {
	if (on) {
		TCCR5A |= 0b00100000; /// turn on OC5B PWM output
	} else {
		TCCR5A &= 0b11001111; /// turn off OC5B PWM output
	}
}

void Motherboard::setUsingPlatform(bool is_using) {
  using_platform = is_using;
}

void Motherboard::setValve(bool on) {
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		setUsingPlatform(false);
		pwmHBP_On(false);
		HBP_HEAT.setValue(on);
	}
}

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value) {
	// This is a bit of a hack to get the temperatures right until we fix our
	// PWM'd PID implementation.  We reduce the MV to one bit, essentially.
	// It works relatively well.
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		pwmHBP_On(false);
		HBP_HEAT.setValue(value != 0);
	}
  
}


