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

#include "ExtruderBoard.hh"
#include "HeatingElement.hh"
//#include "ExtruderMotor.hh"
//#include "MotorController.hh"
#include "Configuration.hh"
#include "CoolingFan.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>

//ExtruderBoard ExtruderBoard::extruder_board;

ExtruderBoard::ExtruderBoard(uint8_t slave_id_in, Pin HeaterPin_In, Pin FanPin_In, Pin ThermocouplePin_In) :
     		extruder_thermocouple(ThermocouplePin_In,THERMOCOUPLE_SCK,THERMOCOUPLE_SO),
     		extruder_element(slave_id_in),
            extruder_heater(extruder_thermocouple,extruder_element,SAMPLE_INTERVAL_MICROS_THERMOCOUPLE,eeprom::EXTRUDER_PID_BASE),
      		coolingFan(extruder_heater, eeprom::COOLING_FAN_BASE),
      		slave_id(slave_id_in),
      		Heater_Pin(HeaterPin_In),
      		Fan_Pin(FanPin_In)
{
}

void ExtruderBoard::reset() {


	// Set the output mode for the mosfets.  All three should default
	// off.
	Heater_Pin.setValue(false);
	Heater_Pin.setDirection(true);
	Fan_Pin.setValue(false);
	Fan_Pin.setDirection(true);

	// Timer 0:
	//  Mode: Phase-correct PWM (WGM2:0 = 001), cycle freq= 976 Hz
	//  Prescaler: 1/64 (250 KHz)
	//  Mosfet C (labeled heater, used for extruder heater)
	//   - uses OCR0A to generate PWM
	//  H-bridge enable (used for DC motor, or fan on stepstruder:
	//   - uses OCR0B to generate PWM
	/*	TCCR0A = 0b00000001;
	TCCR0B = 0b00000011;
	OCR0A = 0;
	OCR0B = 0;
	TIMSK0 = 0b00000000; // no interrupts needed

	// Timer 1:
	//  Mode: Normal (WGM13:0 = 0000), cycle freq= 30Hz
	//  Prescaler: 1/8 (2 MHz)
	//  Mosfet A (labeled fan, used for ABP motor)
	//  - Uses  OCR1B to generate PWM
	//  Mosfet B (labeled extra, used for HBP heater)
	//  - Uses  OCR1A to generate PWM
	TCCR1A = 0b00000000;
	TCCR1B = 0b00000010;
	OCR1A = 0;
	OCR1B = 0;
	TIMSK0 = 0b00000000; // no interrupts needed


	// Timer 2:
	//  Mode: CTC (WGM2:0 = 010), cycle freq=
	//  Prescaler: 1/32 (500 KHz)
	//  used as a provider for microsecond-level counting
	//  - Generates interrupt every 32uS
	//  used also to run servos in software
	TCCR2A = 0x02; // CTC is mode 2 on timer 2
	TCCR2B = 0x03; // prescaler: 1/32
	OCR2A = INTERVAL_IN_MICROSECONDS / 2; // 2uS/tick at 1/32 prescaler
	TIMSK2 = 0x02; // turn on OCR2A match interrupt
	*/
	
	extruder_thermocouple.init();
	coolingFan.reset();

}

void ExtruderBoard::runExtruderSlice() {

        extruder_heater.manage_temperature();
        coolingFan.manageCoolingFan();

}



void ExtruderBoard::setFan(bool on) {
	//CHANNEL_A.setValue(on);
//	MOTOR_DIR_PIN.setDirection(true);
//	MOTOR_DIR_PIN.setValue(true);
//	MOTOR_ENABLE_PIN.setDirection(true);
//	MOTOR_ENABLE_PIN.setValue(on);
//	EX1_FAN.setValue(on);
	
}



