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
#define INTERVAL_IN_MICROSECONDS 128

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
#define SD_WRITE_PIN    Pin(PortH,5)
// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN   Pin(PortH,6)
// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN   Pin(PortB,0)

// --- Slave UART configuration ---
// The slave UART is presumed to be an RS485 connection through a sn75176 chip.
// Define as 1 if the slave UART is present; 0 if not.
#define HAS_SLAVE_UART  1
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
#define BUZZER_PIN Pin(PortG,5)

// --- Emergency Stop configuration ---
// Define as 1 if the estop is present, 0 if not.
#define HAS_ESTOP 0
// The pin connected to the emergency stop
//
//Note: this is a non-connected pin -temporary until ESTOP pin is located
#define ESTOP_PIN Pin(PortG,3)

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.
#define STEPPER_COUNT 5

// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS 1

// The X stepper step pin (active on rising edge)
#define X_STEP_PIN      Pin(PortF,1)
// The X direction pin (forward on logic high)
#define X_DIR_PIN       Pin(PortF,0)
// The X stepper enable pin (active low)
#define X_ENABLE_PIN    Pin(PortF,2)
// X stepper potentiometer pin
#define X_POT_PIN	Pin(PortF,3)
// The X minimum endstop pin (active high)
#define X_MIN_PIN       Pin(PortL,0)
// The X maximum endstop pin (active high)
#define X_MAX_PIN       Pin(PortL,1)

// The Y stepper step pin (active on rising edge)
#define Y_STEP_PIN      Pin(PortF,5)
// The Y direction pin (forward on logic high)
#define Y_DIR_PIN       Pin(PortF,4)
// The Y stepper enable pin (active low)
#define Y_ENABLE_PIN    Pin(PortF,6)
// Y stepper potentiometer pin
#define Y_POT_PIN	Pin(PortF,7)
// The Y minimum endstop pin (active high)
#define Y_MIN_PIN       Pin(PortL,2)
// The Y maximum endstop pin (active high)
#define Y_MAX_PIN       Pin(PortL,3)

// The Z stepper step pin (active on rising edge)
#define Z_STEP_PIN      Pin(PortK,1)
// The Z direction pin (forward on logic high)
#define Z_DIR_PIN       Pin(PortK,0)
// The Z stepper enable pin (active low)
#define Z_ENABLE_PIN    Pin(PortK,2)
// Z stepper potentiometer pin
#define Z_POT_PIN	Pin(PortK,3)
// The Z minimum endstop pin (active high)
#define Z_MIN_PIN       Pin(PortL,6)
// The Z maximum endstop pin (active high)
#define Z_MAX_PIN       Pin(PortL,7)

// The A stepper step pin (active on rising edge)
#define A_STEP_PIN      Pin(PortA,3)
// The A direction pin (forward on logic high)
#define A_DIR_PIN       Pin(PortA,2)
// The A stepper enable pin (active low)
#define A_ENABLE_PIN    Pin(PortA,4)
// A stepper potentiometer pin
#define A_POT_PIN	Pin(PortA,5)

// The B stepper step pin (active on rising edge)
#define B_STEP_PIN      Pin(PortA,7)
// The B direction pin (forward on logic high)
#define B_DIR_PIN       Pin(PortA,6)
// The B stepper enable pin (active low)
#define B_ENABLE_PIN    Pin(PortG,2)
// B stepper potentiometer pin
#define B_POT_PIN	Pin(PortJ,6)

// --- Debugging configuration ---
// The pin which controls the debug LED (active high)
#define DEBUG_PIN       Pin(PortB,7)
// By default, debugging packets should be honored; this is made
// configurable if we're short on cycles or EEPROM.
// Define as 1 if debugging packets are honored; 0 if not.
#define HONOR_DEBUG_PACKETS 1

#define HAS_INTERFACE_BOARD     1

// Additional Debug Pins
#define DEBUG_PIN1	Pin(PortC,1)
#define DEBUG_PIN2	Pin(PortC,0)
#define DEBUG_PIN3	Pin(PortG,1)

//these are not real pins - they are unconnected pins as place holders until code is changed
#define LCD_STROBE		Pin(PortC,4)
#define LCD_CLK			Pin(PortC,2)
#define LCD_DATA		Pin(PortC,3)

/// This is the pin mapping for the interface board. Because of the relatively
/// high cost of using the pins in a direct manner, we will instead read the
/// buttons directly by scanning their ports. If any of these definitions are
/// modified, the #scanButtons() function _must_ be updated to reflect this.
///

// pin connections on revD board
#define INTERFACE_UP		Pin(PORTJ,4)
#define INTERFACE_DOWN		Pin(PortJ,3) 
#define INTERFACE_RIGHT		Pin(PortJ,1) 
#define INTERFACE_LEFT		Pin(PortJ,2) 
#define INTERFACE_CENTER	Pin(PortJ,0) 

#define INTERFACE_GLED		Pin(PortC, 6)
#define INTERFACE_RLED		Pin(PortC, 5)

#define INTERFACE_DETECT	Pin(PortC, 7)

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH        20
#define LCD_SCREEN_HEIGHT       4

///// **** Extruder Controller ***************///////////

// TODO: Ditch this in favor of a unified board.hh, or something.
//#define IS_EXTRUDER_BOARD

// Interval for timer update in microseconds
// Servos are locked to this, so this must be 2500.
// (It turns out that nothing needed microsecond timing, anyway.)
#define INTERVAL_IN_MICROSECONDS 2500

// Fix for buggy RS485 chips: no known Gen3 hardware has this issue.
#define ASSERT_LINE_FIX         0


// Extrusion head heater configuration
#define HAS_HEATER              1

#define HAS_SERVOS              1

/// True if there are any thermistors on the board
#define HAS_THERMISTOR_TABLES

#define HAS_THERMISTOR          0

// Extruder thermistor analog pin
#define THERMISTOR_PIN          15

#define HAS_THERMOCOUPLE        1

#define THERMOCOUPLE_CS1        Pin(PortE,3)
#define THERMOCOUPLE_CS2        Pin(PortE,4)
#define THERMOCOUPLE_SCK        Pin(PortE,2)
#define THERMOCOUPLE_SO         Pin(PortE,5)

// Platform thermistor analog pin
#define PLATFORM_PIN            15

#define CHANNEL_A               Pin(PortH,3) //EX1_PWR
#define EX2_PWR	                Pin(PortB,5) //EX2_PWR
#define EX1_FAN                 Pin(PortH,4) //EX1_FAN
#define HBP_HEAT                Pin(PortL,4) // OC5B
#define EX2_FAN                 Pin(PORTB,6)
#define EXTRA_FET               Pin(PortL,5)

// Fan configuration
//#define HAS_FAN                 1
//#define FAN_ENABLE_PIN          CHANNEL_C

/// these are replaced by A,B stepper motors
//#define HB1_ENABLE_PIN          Pin(PortD,5)
//#define HB1_DIR_PIN             Pin(PortD,7)

//#define HB2_ENABLE_PIN          Pin(PortD,6)
//#define HB2_DIR_PIN             Pin(PortB,0)

// define the tick length, or how often the interrupt is called,
// for the external stepper, in half microseconds:
// 200 means 100 us -> 10 KHz
#define ES_TICK_LENGTH          200

// Enable = "D10"
#define ES_ENABLE_PIN           Pin(PortB,2)
// Dir and step are in the quadrature
#define ES_DIR_PIN              Pin(PortD,2) // Quadrature pin 8
#define ES_STEP_PIN             Pin(PortD,3) // Quadrature pin 7

//#define DEBUG_LED               Pin(PortB,5)

//#define SERVO0                  Pin(PortC,2)
//#define SERVO1                  Pin(PortC,3)

//#define HAS_DC_MOTOR            1
#define MOTOR_ENABLE_PIN        Pin(PortD,5)
#define MOTOR_DIR_PIN           Pin(PortB,0)

#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

#endif // BOARDS_MBV40_CONFIGURATION_HH_
