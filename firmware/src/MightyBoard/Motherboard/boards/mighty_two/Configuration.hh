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

// Bot type used in stream version command
#define BOT_TYPE 0xB015

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.
#define STEPPER_COUNT   5
#define MAX_STEPPERS    5
#define EXTRUDERS       2

// microstepping is 1 / (1 << MICROSTEPPING)
//  0 for 1/1
//  1 for 1/2
//  2 for 1/4
//  3 for 1/8
//  4 for 1/16
//  5 for 1/32
//  etc.
#define MICROSTEPPING   4

#ifndef SIMULATOR

// This file details the pin assignments and features of the Makerbot Extended Motherboard 4.0

#include "AvrPort.hh"

// Enable the P-Stop (pause stop) support
#define PSTOP_SUPPORT

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


// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS 1

//Stepper Ports
#define X_STEPPER_STEP          STEPPER_PORT(D,6)	//active rising edge
#define X_STEPPER_DIR           STEPPER_PORT(D,7)	//forward on high
#define X_STEPPER_ENABLE        STEPPER_PORT(D,4)	//active low
#define X_STEPPER_MIN           STEPPER_PORT(J,2)	//active high
#define X_STEPPER_MAX           STEPPER_PORT(C,7)	//active high

#define Y_STEPPER_STEP          STEPPER_PORT(L,5)	//active rising edge
#define Y_STEPPER_DIR           STEPPER_PORT(L,7)	//forward on high
#define Y_STEPPER_ENABLE        STEPPER_PORT(L,4)	//active low
#define Y_STEPPER_MIN           STEPPER_PORT(J,1)	//active high
#define Y_STEPPER_MAX           STEPPER_PORT(C,6)	//active high

// P-Stop is X_STEPPER_MIN = PJ1 = PCINT10
#define PSTOP_PORT  Pin(PortL,0)
#define PSTOP_MSK   PCMSK1
#define PSTOP_PCINT PCINT10
#define PSTOP_PCIE  PCIE1
#define PSTOP_VECT  PCINT1_vect

#define Z_STEPPER_STEP          STEPPER_PORT(L,1)	//active rising edge
#define Z_STEPPER_DIR           STEPPER_PORT(L,2)	//forward on high
#define Z_STEPPER_ENABLE        STEPPER_PORT(L,0)	//active low
#define Z_STEPPER_MIN           STEPPER_PORT(C,5)	//active high
#define Z_STEPPER_MAX           STEPPER_PORT(J,0)	//active high

#define A_STEPPER_STEP          STEPPER_PORT(A,3)	//active rising edge
#define A_STEPPER_DIR           STEPPER_PORT(A,2)	//forward on high
#define A_STEPPER_ENABLE        STEPPER_PORT(A,5)	//active low

#define B_STEPPER_STEP          STEPPER_PORT(A,0)	//active rising edge
#define B_STEPPER_DIR           STEPPER_PORT(K,7)	//forward on high
#define B_STEPPER_ENABLE        STEPPER_PORT(A,1)	//active low

// X stepper potentiometer pin
#define X_POT_PIN	Pin(PortD,5)
// Y stepper potentiometer pin
#define Y_POT_PIN	Pin(PortL,6)
// Z stepper potentiometer pin
#define Z_POT_PIN	Pin(PortL,3)
// A stepper potentiometer pin
#define A_POT_PIN	Pin(PortA,4)
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
#define DEBUG_PIN4  Pin(PortF,0)  // ADC0
#define DEBUG_PIN5  Pin(PortF,1)  // ADC1
#define DEBUG_PIN6  Pin(PortF,2)  // ADC2

/// Analog pins for reading digipot output (VREF Pins)
#define XVREF_Pin	NULL
#define YVREF_Pin	NULL
#define ZVREF_Pin	NULL
#define AVREF_Pin	NULL
#define BVREF_Pin	NULL

// Random Generator Pin
#define RANDOM_PIN  8


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

#ifdef REVG
#define INTERFACE_POWER		Pin(PortA, 7)
#endif

#define INTERFACE_DDR           DDRC
#define INTERFACE_LED_PORT      PORTC
#define INTERFACE_LED           (1 << PC2)

#define INTERFACE_DETECT	Pin(PortC, 4)

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH        20
#define LCD_SCREEN_HEIGHT       4

///// **** HBP and Extruder  ***************/////

/// True if there are any thermistors on the board
#define HAS_THERMISTOR_TABLES

// Platform thermistor analog pin
#define PLATFORM_PIN          3

#define HAS_THERMOCOUPLE        1

#define THERMOCOUPLE_DI        Pin(PortE,7)
#define THERMOCOUPLE_CS        Pin(PortE,6)
#define THERMOCOUPLE_SCK       Pin(PortE,2)
#define THERMOCOUPLE_DO        Pin(PortH,2)

#define DEFAULT_THERMOCOUPLE_VAL	1024

/// POWER Pins for extruders, fans and heated build platform
#define EXA_PWR	                Pin(PortE,5) // OC3C
#define EXB_PWR	                Pin(PortE,3) // OC3A
#define EXA_FAN                 Pin(PortH,4) // OC4B
#define EXB_FAN                 Pin(PortE,4) // OC3B
#define HBP_HEAT                Pin(PortH,5) // OC5B

#define ACTIVE_COOLING_FAN
#define EX_FAN                  Pin(PortG,5)
#define EXTRA_FET               EX_FAN

// sample intervals for heaters
#define SAMPLE_INTERVAL_MICROS_THERMISTOR    (50L * 1000L)

// Rep 1: wait 0.5 seconds, then handle Extruder 1
//        wait tiny amount, then handle Extruder 2
//
// Sampling rate is 2 Hz

// Rep 2: wait 0.25 seconds, sample ambient (cold junction)
//        wait 0.25 seconds, sample extruder 1
//        wait 0.25 seconds, sample extruder 2
//
// Sampling is thus 1.333 Hz

#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (250L * 1000L)

// bot shuts down printers after a defined timeout 
#define USER_INPUT_TIMEOUT		1800000000 // 30 minutes

//If defined, erase the eeprom area on every boot, useful for diagnostics
//#define ERASE_EEPROM_ON_EVERY_BOOT

//If defined, enable an additional Utilities menu that allows erasing, saving and loading
//of eeprom data
#define EEPROM_MENU_ENABLE

//If defined, the planner is constrained to a pipeline size of 1,
//this means that acceleration still happens, but only on a per block basis,
//there's no speeding up between blocks.
//#define PLANNER_OFF

//If defined sets the debug header (DEBUG_PIN1/2/3/4/5/6/7/8) to the given value
//#define DEBUG_VALUE(x)	setDebugValue(x)

//If defined provides 2 debugging variables for on screen display during build
//Variables are floats:  debug_onscreen1, debug_onscreen2 and can be found in Steppers.hh
//#define DEBUG_ONSCREEN

//If defined, the stack is painted with a value and the free sram reported in
//in the Version menu.  This enables debugging to see if the SRAM was ever exhausted
//which would lead to stack corruption.
#define STACK_PAINT

//Definitions for the timer / counter  to use for the stepper interrupt
//Change this to a different 16 bit interrupt if you need to
#define STEPPER_OCRnA			OCR1A
#define STEPPER_TIMSKn			TIMSK1
#define STEPPER_OCIEnA			OCIE1A
#define STEPPER_TCCRnA			TCCR1A
#define STEPPER_TCCRnB			TCCR1B
#define STEPPER_TCCRnC			TCCR1C
#define STEPPER_TCNTn			TCNT1
#define STEPPER_TIMERn_COMPA_vect	TIMER1_COMPA_vect

//Oversample the dda to provide less jitter.
//To switch off oversampling, comment out
//2 is the number of bits, as in a bit shift.  So << 2 = multiply by 4
//= 4 times oversampling
//Obviously because of this oversampling is always a power of 2.
//Don't make it too large, as it will kill performance and can overflow int32_t
//#define OVERSAMPLED_DDA 2

#endif // !SIMULATOR

#define JKN_ADVANCE

//Minimum time in seconds that a movement needs to take if the planning pipeline command buffer is
//emptied. Increase this number if you see blobs while printing high speed & high detail. It will
//slowdown on the detailed stuff.
#define ACCELERATION_MIN_SEGMENT_TIME 0.0200

//Minimum planner junction speed (mm/sec). Sets the default minimum speed the planner plans for at 
//the end of the buffer and all stops. This should not be much greater than zero and should only be 
//changed if unwanted behavior is observed on a user's machine when running at very slow speeds. 
//2mm/sec is the recommended value.
#define ACCELERATION_MIN_PLANNER_SPEED 2

//Slowdown limit specifies what to do when the pipeline command buffer starts to empty.
//The pipeline command buffer is 16 commands in length, and Slowdown Limit can be set 
//between 0 - 8 (half the buffer size).
//
//When Commands Left <= Slowdown Limit, the feed rate is progressively slowed down as the buffer 
//becomes more empty.
//
//By slowing down the feed rate, you reduce the possibility of running out of commands, and creating 
//a blob due to the stopped movement.
//
//Possible values are:
//
//0 - Disabled - Never Slowdown
//1 - DON'T USE
//2 - DON'T USE
//3,4,5,6,7,8 - The higher the number, the earlier the start of the slowdown
#define ACCELERATION_SLOWDOWN_LIMIT 4

//ACCELERATION_EXTRUDER_WHEN_NEGATIVE specifies the direction of extruder.
//If negative steps cause an extruder to extrude material, then set this to true.
//If positive steps cause an extruder to extrude material, then set this to false.
//Note: Although a Replicator can have 2 extruders rotating in opposite directions,
//both extruders require negative steps to extrude material.
//This setting effects "Advance" and "Extruder Deprime".
#define ACCELERATION_EXTRUDE_WHEN_NEGATIVE_A true
#define ACCELERATION_EXTRUDE_WHEN_NEGATIVE_B true

// If defined, overlapping stepper interrupts don't cause clunking
// The ideal solution it to adjust calc_timer, but this is just a safeguard
#define ANTI_CLUNK_PROTECTION

//If defined, speed is drastically reducing to crawling
//Very useful for watching acceleration and locating any bugs visually
//Only slows down when acceleration is also set on.
//#define DEBUG_SLOW_MOTION

//If defined, the toolhead and hbp are not heated, and there's
//no waiting.
//This is useful to test movement without extruding any plastic.
//HIGHLY ADVISABLE TO HAVE NO FILAMENT LOADED WHEN YOU DO THIS
//#define DEBUG_NO_HEAT_NO_WAIT

//If defined (and STACK_PAINT is defined), SRAM is monitored occasionally for
//corruption, signalling and 6 repeat error tone on the buzzer if it occurs.
//#define DEBUG_SRAM_MONITOR

//When a build is cancelled or paused, we clear the nozzle
//from the build volume.  This denotes the X/Y/Z position we should
//move to.  max/min_axis_steps_limit can be used for the limits of an axis.
//If you're moving to a position that's an end stop, it's advisable to
//clear the end stop by a few steps as you don't want the endstop to
//be hit due to positioning accuracy and the possibility of an endstop triggering
//a few steps around where it should be.
//If the value isn't defined, the axis is moved

#define BUILD_CLEAR_MARGIN 5.0 // 5.0 mm
// ***** WARNING ***** Math for _X and _Y assumes X and Y home offsets are positive....
#define BUILD_CLEAR_X ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + X_AXIS * sizeof(uint32_t), stepperAxis[X_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(X_AXIS)) )
#define BUILD_CLEAR_Y ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Y_AXIS * sizeof(uint32_t), stepperAxis[Y_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(Y_AXIS)) )
#define BUILD_CLEAR_Z (stepperAxis[Z_AXIS].max_axis_steps_limit)

//When pausing, filament is retracted to stop stringing / blobbing.
//This sets the amount of filament in mm's to be retracted
#define PAUSE_RETRACT_FILAMENT_AMOUNT_MM        2.0

//When defined, the Ditto Printing setting is added to General Settings
// Note ditto printing is okay for Rep 2X
#define DITTO_PRINT

//When defined, the Z axis is clipped to it's maximum limit
//Applicable to Replicator.  Probably not applicable to ToM/Cupcake due to incorrect length
//in the various .xml's out there
#define CLIP_Z_AXIS

// Our software variant id for the advanced version command
#define SOFTWARE_VARIANT_ID 0x80

#ifdef MODEL_REPLICATOR2
//When defined, acceleration stats are displayed on the LCD screen
//#define ACCEL_STATS
#endif

// Disabled SD card folder support owing to a broken SD card detect switch
//#define BROKEN_SD

// Build with nozzle calibration S3G script and help screen to run it
//#define NOZZLE_CALIBRATION_SCRIPT

// Single extruder builds have space for SDHC & FAT-32 support
#if defined(SINGLE_EXTRUDER) || !defined(NOZZLE_CALIBRATION_SCRIPT) || defined(__AVR_ATmega2560__)
#define SD_RAW_SDHC 1
#endif

// When defined, the ability to write an SD card file over S3G is supported
//#define S3G_CAPTURE_2_SD

#endif // BOARDS_MBV40_CONFIGURATION_HH_
