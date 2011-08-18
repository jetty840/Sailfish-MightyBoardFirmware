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
#include "Tool.hh"
#include "Commands.hh"
#include "EepromMap.hh"


/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
        lcd(LCD_RS_PIN,
            LCD_ENABLE_PIN,
            LCD_D0_PIN,
            LCD_D1_PIN,
            LCD_D2_PIN,
            LCD_D3_PIN),
        interfaceBoard(buttonArray,
            lcd,
            INTERFACE_FOO_PIN,
            INTERFACE_BAR_PIN,
            &mainMenu,
            &monitorMode)

{
	/// Set up the stepper pins on board creation
#if STEPPER_COUNT > 0
        stepper[0] = StepperInterface(X_DIR_PIN,
                                      X_STEP_PIN,
                                      X_ENABLE_PIN,
                                      X_MAX_PIN,
                                      X_MIN_PIN,
                                      eeprom::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 1
        stepper[1] = StepperInterface(Y_DIR_PIN,
                                      Y_STEP_PIN,
                                      Y_ENABLE_PIN,
                                      Y_MAX_PIN,
                                      Y_MIN_PIN,
                                      eeprom::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 2
        stepper[2] = StepperInterface(Z_DIR_PIN,
                                      Z_STEP_PIN,
                                      Z_ENABLE_PIN,
                                      Z_MAX_PIN,
                                      Z_MIN_PIN,
                                      eeprom::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 3
        stepper[3] = StepperInterface(A_DIR_PIN,
                                      A_STEP_PIN,
                                      A_ENABLE_PIN,
                                      Pin(),
                                      Pin(),
                                      eeprom::AXIS_INVERSION);
#endif
#if STEPPER_COUNT > 4
        stepper[4] = StepperInterface(B_DIR_PIN,
                                      B_STEP_PIN,
                                      B_ENABLE_PIN,
                                      Pin(),
                                      Pin(),
                                      eeprom::AXIS_INVERSION);
#endif
}

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::reset() {
	indicateError(0); // turn off blinker

	// Init steppers
	// NB: for now, we are turning on Z hold for these boards!
	steppers::setHoldZ(true);
	for (int i = 0; i < STEPPER_COUNT; i++) {
		stepper[i].init(i);
	}
	// Initialize the host and slave UARTs
        UART::getHostUART().enable(true);
        UART::getHostUART().in.reset();
        UART::getSlaveUART().enable(true);
        UART::getSlaveUART().in.reset();
	// Reset and configure timer 1, the microsecond and stepper
	// interrupt timer.
	TCCR1A = 0x00;
	TCCR1B = 0x09;
	TCCR1C = 0x00;
	OCR1A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
	// Reset and configure timer 2, the debug LED flasher timer.
	TCCR2A = 0x00;
	TCCR2B = 0x07; // prescaler at 1/1024
	TIMSK2 = 0x01; // OVF flag on
	// Configure the debug pin.
	DEBUG_PIN.setDirection(true);

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

        // Blindly try to reset the toolhead with index 0.
//        resetToolhead();
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
}

void Motherboard::runMotherboardSlice() {
	if (hasInterfaceBoard) {
		if (interface_update_timeout.hasElapsed()) {
                        interfaceBoard.doUpdate();
                        interface_update_timeout.start(interfaceBoard.getUpdateRate());
		}
	}
}


/// Timer one comparator match interrupt
ISR(TIMER1_COMPA_vect) {
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
