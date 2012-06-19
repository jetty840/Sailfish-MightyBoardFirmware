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

#ifndef BOARDS_MBV40_CONFIGURATION_HH_
#define BOARDS_MBV40_CONFIGURATION_HH_

// This file details the pin assignments and features of the Makerbot Extended Motherboard 4.0

#include "AvrPort.hh"

// Interval for the stepper update in microseconds.  This interval is the minimum
// possible time between steps; in practical terms, your time between steps should
// be at least eight times this large.  Reducing the interval can cause resource
// starvation; leave this at 64uS or greater unless you know what you're doing.
#define INTERVAL_IN_MICROSECONDS 256
#define HOMING_INTERVAL_IN_MICROSECONDS 128

// --- Power Supply Unit configuration ---
// Define as 1 if a PSU is present; 0 if not.
#define HAS_PSU         0

// --- Secure Digital Card configuration ---
// NOTE: If SD support is enabled, it is implicitly assumed that the
// following pins are connected:
//  AVR    |   SD header
//---------|--------------
//  MISO   |   DATA_OUT
//  MOSI   |   DATA_IN
//  SCK    |   CLK

// Define as 1 if and SD card slot is present; 0 if not.
#define HAS_SD          0
// The pin that connects to the write protect line on the SD header.
#define SD_WRITE_PIN    Pin(PortG,0)
// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN   Pin(PortG,1)
// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN   Pin(PortB,0)

// --- Slave UART configuration ---
// The slave UART is presumed to be an RS485 connection through a sn75176 chip.
// Define as 1 if the slave UART is present; 0 if not.
#define HAS_SLAVE_UART 0
// The pin that connects to the driver enable line on the RS485 chip.
#define TX_ENABLE_PIN           Pin(PortD,3)
// The pin that connects to the active-low recieve enable line on the RS485 chip.
#define RX_ENABLE_PIN           Pin(PortD,2)

// --- Host UART configuration ---
// The host UART is presumed to always be present on the RX/TX lines.

// --- Piezo Buzzer configuration ---
// Define as 1 if the piezo buzzer is present, 0 if not.
#define HAS_BUZZER 1
// The pin that drives the buzzer
#define BUZZER_PIN Pin(PortH,3)  //OC4A

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.
#define STEPPER_COUNT   5
#define MAX_STEPPERS    5

// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS 1

// compare register used by stepper timer
#define STEPPER_COMP_REGISTER OCR1A

// The X stepper step pin (active on rising edge)
#define X_STEP_PIN      Pin(PortD,6)
// The X direction pin (forward on logic high)
#define X_DIR_PIN       Pin(PortD, 7)
// The X stepper enable pin (active low)
#define X_ENABLE_PIN    Pin(PortD,4)
// X stepper potentiometer pin
#define X_POT_PIN	Pin(PortD,5)
// The X minimum endstop pin (active high)
#define X_MIN_PIN       Pin(PortJ,2)
// The X maximum endstop pin (active high)
#define X_MAX_PIN       Pin(PortC,7)

// The Y stepper step pin (active on rising edge)
#define Y_STEP_PIN      Pin(PortL,5)
// The Y direction pin (forward on logic high)
#define Y_DIR_PIN       Pin(PortL,7)
// The Y stepper enable pin (active low)
#define Y_ENABLE_PIN    Pin(PortL,4)
// Y stepper potentiometer pin
#define Y_POT_PIN		Pin(PortL,6)
// The Y minimum endstop pin (active high)
#define Y_MIN_PIN       Pin(PortJ,1)
// The Y maximum endstop pin (active high)
#define Y_MAX_PIN       Pin(PortC,6)

// The Z stepper step pin (active on rising edge)
#define Z_STEP_PIN      Pin(PortL,1)
// The Z direction pin (forward on logic high)
#define Z_DIR_PIN       Pin(PortL,2)
// The Z stepper enable pin (active low)
#define Z_ENABLE_PIN    Pin(PortL,0)
// Z stepper potentiometer pin
#define Z_POT_PIN		Pin(PortL,3)
// The Z minimum endstop pin (active high)
#define Z_MIN_PIN       Pin(PortC,5)
// The Z maximum endstop pin (active high)
#define Z_MAX_PIN       Pin(PortJ,0)

// The A stepper step pin (active on rising edge)
#define A_STEP_PIN      Pin(PortA,3)
// The A direction pin (forward on logic high)
#define A_DIR_PIN       Pin(PortA,2)
// The A stepper enable pin (active low)
#define A_ENABLE_PIN    Pin(PortA,5)
// A stepper potentiometer pin
#define A_POT_PIN		Pin(PortA,4)

// The B stepper step pin (active on rising edge)
#define B_STEP_PIN      Pin(PortA,0)
// The B direction pin (forward on logic high)
#define B_DIR_PIN       Pin(PortK,7)
// The B stepper enable pin (active low)
#define B_ENABLE_PIN    Pin(PortA,1)
// B stepper potentiometer pin
#define B_POT_PIN       Pin(PortJ,7)

// i2c pots SCL pin
#define POTS_SCL        Pin(PortA,6)
// default value for pots (0-127 valid)
#define POTS_DEFAULT_VAL 50

// --- Debugging configuration ---
// The pin which controls the debug LED (active high)
#define DEBUG_PIN       Pin(PortB,7)
// Additional Debug Pins
#define DEBUG_PIN1	Pin(PortG,4)
#define DEBUG_PIN2	Pin(PortG,3)
#define DEBUG_PIN3	Pin(PortH,7)
#define DEBUG_PIN4  Pin(PortF,0) // ADC0
#define DEBUG_PIN5  Pin(PortF,1) // ADC1
#define DEBUG_PIN6  Pin(PortF,2) // ADC2

/// Analog pins for reading digipot output (VREF Pins)
#define XVREF_Pin	NULL
#define YVREF_Pin	NULL
#define ZVREF_Pin	NULL
#define AVREF_Pin	NULL
#define BVREF_Pin	NULL


// By default, debugging packets should be honored; this is made
// configurable if we're short on cycles or EEPROM.
// Define as 1 if debugging packets are honored; 0 if not.
#define HONOR_DEBUG_PACKETS 0

#define HAS_INTERFACE_BOARD     1

// LCD interface pins
#define LCD_STROBE		Pin(PortC,3)
#define LCD_CLK			Pin(PortC,1)
#define LCD_DATA		Pin(PortC,0)

/// This is the pin mapping for the interface board. Because of the relatively
/// high cost of using the pins in a direct manner, we will instead read the
/// buttons directly by scanning their ports. If any of these definitions are
/// modified, the #scanButtons() function _must_ be updated to reflect this.
#define INTERFACE_UP		Pin(PORTJ,5)
#define INTERFACE_DOWN		Pin(PortJ,4) 
#define INTERFACE_RIGHT		Pin(PortJ,3) 
#define INTERFACE_LEFT		Pin(PortJ,6) 
#define INTERFACE_CENTER	Pin(PortG,2) 

#define INTERFACE_LED_ONE	Pin(PortA, 7)
#define INTERFACE_LED_TWO	Pin(PortC, 2)

#define INTERFACE_DETECT	Pin(PortC, 4)

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH        20
#define LCD_SCREEN_HEIGHT       4

///// **** HBP and Extruder  ***************/////

/// True if there are any thermistors on the board
#define HAS_THERMISTOR_TABLES

// Platform thermistor analog pin
/// TEMPORARY PIN - We're going to have to cut the trace and connect the thermistor
/// currently the thermistor is connected to PH6 
/// analog pin zero is extra_io_4
#define PLATFORM_PIN          0

#define HAS_THERMOCOUPLE        1

#define THERMOCOUPLE_DI        Pin(PortH,2)
#define THERMOCOUPLE_CS        Pin(PortE,6)
#define THERMOCOUPLE_SCK       Pin(PortE,2)
#define THERMOCOUPLE_DO        Pin(PortE,7)

#define DEFAULT_THERMOCOUPLE_VAL	1024

/// POWER Pins for extruders, fans and heated build platform
#define EXA_PWR	                Pin(PortE,5) // OC3C
#define EXB_PWR	                Pin(PortE,3) // OC3A
#define EXA_FAN                 Pin(PortH,4) // OC4B
#define EXB_FAN                 Pin(PortE,4) // OC3B
#define HBP_HEAT                Pin(PortH,5) // OC5B

#define EXTRA_FET               Pin(PortG,5)

// sample intervals for heaters
#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

// bot shuts down printers after a defined timeout 
#define USER_INPUT_TIMEOUT		1800000000 // 30 minutes

#define XSTEPS_PER_MM          94.139704f
#define YSTEPS_PER_MM          94.139704f
#define ZSTEPS_PER_MM          400
#define ASTEPS_PER_MM          96.2752018f
#define BSTEPS_PER_MM          96.2752018f


#endif // BOARDS_MBV40_CONFIGURATION_HH_
