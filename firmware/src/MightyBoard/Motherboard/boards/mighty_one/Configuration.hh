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
#define SD_WRITE_PIN    Pin(PortH,5)
// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN   Pin(PortH,6)
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

// Random Generator Pin
#define RANDOM_PIN  0


// --- Piezo Buzzer configuration ---
// Define as 1 if the piezo buzzer is present, 0 if not.
#define HAS_BUZZER 1
// The pin that drives the buzzer
#define BUZZER_PIN Pin(PortG,5)  //OC0B

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

// The X stepper step pin (active on rising edge)
#define X_STEP_PORT		PORTF	
#define X_STEP_PIN		PINF1
#define X_STEP_DDR		DDRF
// The X direction pin (forward on logic high)
#define X_DIR_PORT		PORTF	
#define X_DIR_PIN		PINF0
#define X_DIR_DDR		DDRF
// The X stepper enable pin (active low)
#define X_ENABLE_PORT	PORTF	
#define X_ENABLE_IPORT	PINF	
#define X_ENABLE_PIN	PINF2
#define X_ENABLE_DDR	DDRF
// X stepper potentiometer pin
#define X_POT_PIN	Pin(PortF,3)
// The X minimum endstop pin (active high)
#define X_MIN_IPORT		PINL	
#define X_MIN_PORT		PORTL
#define X_MIN_PIN		PINL0
#define X_MIN_DDR		DDRL
// The X maximum endstop pin (active high)
#define X_MAX_IPORT		PINL	
#define X_MAX_PORT		PORTL
#define X_MAX_PIN		PINL1
#define X_MAX_DDR		DDRL

// The Y stepper step pin (active on rising edge)
#define Y_STEP_PORT		PORTF	
#define Y_STEP_PIN		PINF5
#define Y_STEP_DDR		DDRF
// The Y direction pin (forward on logic high)
#define Y_DIR_PORT		PORTF	
#define Y_ENABLE_IPORT	PINF
#define Y_DIR_PIN		PINF4
#define Y_DIR_DDR		DDRF
// The Y stepper enable pin (active low)
#define Y_ENABLE_PORT	PORTF	
#define Y_ENABLE_PIN	PINF6
#define Y_ENABLE_DDR	DDRF
// Y stepper potentiometer pin
#define Y_POT_PIN	Pin(PortF,7)
// The Y minimum endstop pin (active high)
#define Y_MIN_IPORT		PINL	
#define Y_MIN_PORT		PORTL
#define Y_MIN_PIN		PINL2
#define Y_MIN_DDR		DDRL
// The Y maximum endstop pin (active high)
#define Y_MAX_IPORT		PINL	
#define Y_MAX_PORT		PORTL
#define Y_MAX_PIN		PINL3
#define Y_MAX_DDR		DDRL

// The Z stepper step pin (active on rising edge)
#define Z_STEP_PORT		PORTK	
#define Z_STEP_PIN		PINK1
#define Z_STEP_DDR		DDRK
// The Z direction pin (forward on logic high)
#define Z_DIR_PORT		PORTK	
#define Z_ENABLE_IPORT	PINF
#define Z_DIR_PIN		PINK0
#define Z_DIR_DDR		DDRK
// The Z stepper enable pin (active low)
#define Z_ENABLE_PORT	PORTK	
#define Z_ENABLE_PIN	PINK2
#define Z_ENABLE_DDR	DDRK
// Z stepper potentiometer pin
#define Z_POT_PIN	Pin(PortK,3)
// The Z minimum endstop pin (active high)
#define Z_MIN_IPORT		PINL	
#define Z_MIN_PORT		PORTL
#define Z_MIN_PIN		PINL6
#define Z_MIN_DDR		DDRL
// The Z maximum endstop pin (active high)
#define Z_MAX_IPORT		PINL	
#define Z_MAX_PORT		PORTL
#define Z_MAX_PIN		PINL7
#define Z_MAX_DDR		DDRL

// The A stepper step pin (active on rising edge)
#define A_STEP_PORT		PORTA	
#define A_STEP_PIN		PINA3
#define A_STEP_DDR		DDRA
// The A direction pin (forward on logic high)
#define A_DIR_PORT		PORTA	
#define A_ENABLE_IPORT	PINA
#define A_DIR_PIN		PINA2
#define A_DIR_DDR		DDRA
// The A stepper enable pin (active low)
#define A_ENABLE_PORT	PORTA	
#define A_ENABLE_PIN	PINA4
#define A_ENABLE_DDR	DDRA
// A stepper potentiometer pin
#define A_POT_PIN	Pin(PortA,5)

// The B stepper step pin (active on rising edge)
#define B_STEP_PORT		PORTA	
#define B_STEP_PIN		PINA7
#define B_STEP_DDR		DDRA
// The B direction pin (forward on logic high)
#define B_DIR_PORT		PORTA	
#define B_DIR_PIN		PINA6
#define B_DIR_DDR		DDRA
// The B stepper enable pin (active low)
#define B_ENABLE_PORT	PORTG	
#define B_ENABLE_IPORT	PING
#define B_ENABLE_PIN	PING2
#define B_ENABLE_DDR	DDRG
// B stepper potentiometer pin
#define B_POT_PIN       Pin(PortJ,6)

// i2c pots SCL pin
#define POTS_SCL        Pin(PortJ,5)
// default value for pots (0-127 valid)
#define POTS_DEFAULT_VAL 50

// --- Debugging configuration ---
// The pin which controls the debug LED (active high)
#define DEBUG_PIN       Pin(PortB,7)
// Additional Debug Pins
#define DEBUG_PIN1	Pin(PortC,1)
#define DEBUG_PIN2	Pin(PortC,0)
#define DEBUG_PIN3	Pin(PortG,1)
#define DEBUG_PIN4  Pin(PortB,7)
#define DEBUG_PIN5  Pin(PortG,0)
#define DEBUG_PIN6  Pin(PortK,6)
#define DEBUG_PIN7  Pin(PortK,5)
#define DEBUG_PIN8  Pin(PortK,4)

// By default, debugging packets should be honored; this is made
// configurable if we're short on cycles or EEPROM.
// Define as 1 if debugging packets are honored; 0 if not.
#define HONOR_DEBUG_PACKETS 0

#define HAS_INTERFACE_BOARD     1

// LCD interface pins
#define LCD_STROBE		Pin(PortC,4)
#define LCD_CLK			Pin(PortC,2)
#define LCD_DATA		Pin(PortC,3)

/// This is the pin mapping for the interface board. Because of the relatively
/// high cost of using the pins in a direct manner, we will instead read the
/// buttons directly by scanning their ports. If any of these definitions are
/// modified, the #scanButtons() function _must_ be updated to reflect this.
#define INTERFACE_UP		Pin(PORTJ,4)
#define INTERFACE_DOWN		Pin(PortJ,3) 
#define INTERFACE_RIGHT		Pin(PortJ,1) 
#define INTERFACE_LEFT		Pin(PortJ,2) 
#define INTERFACE_CENTER	Pin(PortJ,0) 

#define INTERFACE_LED_ONE		Pin(PortC, 5)
#define INTERFACE_LED_TWO		Pin(PortC, 6)

#define INTERFACE_DETECT	Pin(PortC, 7)

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH        20
#define LCD_SCREEN_HEIGHT       4

///// **** HBP and Extruder  ***************/////

/// True if there are any thermistors on the board
#define HAS_THERMISTOR_TABLES

// Extruder thermistor analog pin
#define THERMISTOR_PIN          15

#define HAS_THERMOCOUPLE        1

#define THERMOCOUPLE_CS1        Pin(PortE,3)
#define THERMOCOUPLE_CS2        Pin(PortE,4)
#define THERMOCOUPLE_SCK        Pin(PortE,2)
#define THERMOCOUPLE_SO         Pin(PortE,5)

#define DEFAULT_THERMOCOUPLE_VAL	1024

// Platform thermistor analog pin
#define PLATFORM_PIN            15

/// Analog pins for reading digipot output (VREF Pins)
#define XVREF_Pin	NULL
#define YVREF_Pin	NULL
#define ZVREF_Pin	NULL
#define AVREF_Pin	NULL
#define BVREF_Pin	NULL

/// POWER Pins for extruders, fans and heated build platform
#define EX1_PWR	                Pin(PortH,3) // OC4A
#define EX2_PWR	                Pin(PortB,5) // OC1A
#define EX1_FAN                 Pin(PortH,4) // EX1_FAN
#define HBP_HEAT                Pin(PortL,4) // OC5B
#define EX2_FAN                 Pin(PortB,6)
#define EX_FAN      	        Pin(PortL,5)


#define ACTIVE_COOLING_FAN

// sample intervals for heaters
#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

// Safety Cutoff circuit
#ifndef CUTOFF_PRESENT
  #define CUTOFF_PRESENT			0
#endif // CUTOFF_PRESENT

// safety cutoff circuit 
#define CUTOFF_RESET			Pin(PortH,1)
#define CUTOFF_TEST				Pin(PortH,0)
#define CUTOFF_SR_CHECK			Pin(PortG,4)

#define EX2_PWR_CHECK			Pin(PortB,4)
#define EX1_PWR_CHECK			Pin(PortH,2)

// bot shuts down printers after a defined timeout 
#define USER_INPUT_TIMEOUT		1800000000 // 30 minutes

#define XSTEPS_PER_MM          94.139704f
#define YSTEPS_PER_MM          94.139704f
#define ZSTEPS_PER_MM          400
#define ASTEPS_PER_MM          96.2752018f
#define BSTEPS_PER_MM          96.2752018f


#endif // BOARDS_MBV40_CONFIGURATION_HH_
