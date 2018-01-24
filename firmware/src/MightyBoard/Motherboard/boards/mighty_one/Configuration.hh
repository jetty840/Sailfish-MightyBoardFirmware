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

#include "BoardId.hh"
#include "Model.hh"
#include "MachineId.hh"

// Electronics type (Rev E)
#define BOARD_TYPE		BOARD_TYPE_MIGHTYBOARD_E

// Bot type used in stream version command
#define BOT_TYPE		0xD314

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

// This file details the pin assignments and features of the MakerBot Mightyboard rev E

#include "AvrPort.hh"

// Enable the P-Stop (pause stop) support
#define PSTOP_SUPPORT

// --- Secure Digital Card configuration ---
// NOTE: If SD support is enabled, it is implicitly assumed that the
// following pins are connected:
//  AVR    |   SD header
//---------|--------------
//  MISO   |   DATA_OUT
//  MOSI   |   DATA_IN
//  SCK    |   CLK

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

// --- Piezo Buzzer configuration ---
// Define as 1 if the piezo buzzer is present, 0 if not.
#define HAS_BUZZER		1

// The pin that drives the buzzer
#define BUZZER_TIMER 		0
#define BUZZER_PIN		Pin(PortG,5)  //OC0B
#if defined(BUZZER_SOFT_PWM)
#undef BUZZER_SOFT_PWM
#endif

// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS 1

//Stepper Ports
#define X_STEPPER_STEP          STEPPER_PORT(F,1)	//active rising edge
#define X_STEPPER_DIR           STEPPER_PORT(F,0)	//forward on high
#define X_STEPPER_ENABLE        STEPPER_PORT(F,2)	//active low
#define X_STEPPER_MIN           STEPPER_PORT(L,0)	//active high
#define X_STEPPER_MAX           STEPPER_PORT(L,1)	//active high

#ifdef ZYYZ_3D_PRINTER
// P-Stop is Z_STEPPER_MAX = PL7
#define PSTOP_PORT  Pin(PortL,7)
#ifdef PSTOP_VECT
#undef PSTOP_VECT
#endif
#else
// P-Stop is X_STEPPER_MIN = PL0 / ICP4
#define PSTOP_PORT  Pin(PortL,0)
#ifdef PSTOP_VECT
#undef PSTOP_VECT
#endif
#endif // ZYYX_3D_PRINTER

#define Y_STEPPER_STEP          STEPPER_PORT(F,5)	//active rising edge
#define Y_STEPPER_DIR           STEPPER_PORT(F,4)	//forward on high
#define Y_STEPPER_ENABLE        STEPPER_PORT(F,6)	//active low
#define Y_STEPPER_MIN           STEPPER_PORT(L,2)	//active high
#define Y_STEPPER_MAX           STEPPER_PORT(L,3)	//active high

#define Z_STEPPER_STEP          STEPPER_PORT(K,1)	//active rising edge
#define Z_STEPPER_DIR           STEPPER_PORT(K,0)	//forward on high
#define Z_STEPPER_ENABLE        STEPPER_PORT(K,2)	//active low
#define Z_STEPPER_MIN           STEPPER_PORT(L,6)	//active high
#define Z_STEPPER_MAX           STEPPER_PORT(L,7)	//active high

#ifdef PSTOP_ZMIN_LEVEL
// The Z min endstop is used
#define Z_MIN_STOP_PORT         Pin(PortL,6)
#endif

#define A_STEPPER_STEP          STEPPER_PORT(A,3)	//active rising edge
#define A_STEPPER_DIR           STEPPER_PORT(A,2)	//forward on high
#define A_STEPPER_ENABLE        STEPPER_PORT(A,4)	//active low

#define B_STEPPER_STEP          STEPPER_PORT(A,7)	//active rising edge
#define B_STEPPER_DIR           STEPPER_PORT(A,6)	//forward on high
#define B_STEPPER_ENABLE        STEPPER_PORT(G,2)	//active low

// Digital potentiometers present and supported
#define DIGIPOT_SUPPORT		1
#define SOFTWAREI2C_SUPPORT	1

#define X_POT_PIN      		Pin(PortF,3)
#define Y_POT_PIN		Pin(PortF,7)
#define Z_POT_PIN		Pin(PortK,3)
#define A_POT_PIN		Pin(PortA,5)
#define B_POT_PIN		Pin(PortJ,6)

// i2c pots SCL pin
#define POTS_SCL        Pin(PortJ,5)

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

// Analog pin for reading an Analog Button Panel
#define ANALOG_BUTTONS_PIN 14

#ifdef HAS_ANALOG_BUTTONS
//PortK,6 is the ADC14, since we're using that for the buttons,
//Redefine another unused debug pin for DEBUG_PIN6
#undef DEBUG_PIN6
#define DEBUG_PIN6 Pin(PortH,7)
#endif

#define HAS_INTERFACE_BOARD     1

// LCD interface pins
#define LCD_STROBE		Pin(PortC,4)
#define LCD_CLK			Pin(PortC,2)
#define LCD_DATA		Pin(PortC,3)

/// This is the pin mapping for the interface board. Because of the relatively
/// high cost of using the pins in a direct manner, we will instead read the
/// buttons directly by scanning their ports. If any of these definitions are
/// modified, the #scanButtons() function _must_ be updated to reflect this.
#define INTERFACE_UP		Pin(PortJ,4)
#define INTERFACE_DOWN		Pin(PortJ,3)
#define INTERFACE_RIGHT		Pin(PortJ,1)
#define INTERFACE_LEFT		Pin(PortJ,2)
#define INTERFACE_CENTER	Pin(PortJ,0)

#define INTERFACE_DDR           DDRC
#define INTERFACE_LED_PORT      PORTC
#define INTERFACE_LED           ((1 << PC5) | (1 << PC6))

#define INTERFACE_DETECT	Pin(PortC, 7)

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH        20
#define LCD_SCREEN_HEIGHT       4

///// **** HBP and Extruder  ***************/////

// Extruder thermistor analog pin
#define THERMISTOR_PIN          15

#define THERMOCOUPLE_CS1        Pin(PortE,3)
#define THERMOCOUPLE_CS2        Pin(PortE,4)
#define THERMOCOUPLE_SCK        Pin(PortE,2)
#define THERMOCOUPLE_SO         Pin(PortE,5)

#if defined(USE_THERMOCOUPLE_DUAL)
#undef USE_THERMOCOUPLE_DUAL
#endif

#define DEFAULT_THERMOCOUPLE_VAL	1024
#define FOO_ARG(x)			x

// Platform thermistor analog pin
#define PLATFORM_PIN            15

/// POWER Pins for extruders, fans and heated build platform
#define EXA_PWR	                Pin(PortH,3) // OC4A
#define EXA_PWR_OCRn		OCR4A
#define EXA_PWR_TCCRn		TCCR4A
#define EXA_PWR_TCCRn_on	0b10000000
#define EXA_PWR_TCCRn_off	0b00111111

#define EXB_PWR	                Pin(PortB,5) // OC1A
#define EXB_PWR_OCRn		OCR1A
#define EXB_PWR_TCCRn		TCCR1A
#define EXB_PWR_TCCRn_on	0b10000000
#define EXB_PWR_TCCRn_off	0b00111111

#define HBP_HEAT                Pin(PortL,4) // OC5B

// Extruder heat sink fans
#define EXA_FAN                 Pin(PortH,4) // EX1_FAN
#define EXB_FAN                 Pin(PortB,6)

#define EX_FAN                  Pin(PortL,5)
#define EXTRA_FET               Pin(PortL,5)

#define ACTIVE_COOLING_FAN

// sample intervals for heaters
#define SAMPLE_INTERVAL_MICROS_THERMISTOR (250L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

// Safety Cutoff circuit
#ifndef CUTOFF_PRESENT
#define CUTOFF_PRESENT   0
#endif // CUTOFF_PRESENT

// safety cutoff circuit
#define CUTOFF_RESET			Pin(PortH,1)
#define CUTOFF_TEST				Pin(PortH,0)
#define CUTOFF_SR_CHECK			Pin(PortG,4)

#define EX2_PWR_CHECK			Pin(PortB,4)
#define EX1_PWR_CHECK			Pin(PortH,2)

// bot shuts down printers after a defined timeout
#define USER_INPUT_TIMEOUT		1800000000 // 30 minutes

//If defined, erase the eeprom area on every boot, useful for diagnostics
//#define ERASE_EEPROM_ON_EVERY_BOOT

//If defined, enable an additional Utilities menu that allows erasing, saving and loading
//of eeprom data
//#define EEPROM_MENU_ENABLE

//If defined, the planner is constrained to a pipeline size of 1,
//this means that acceleration still happens, but only on a per block basis,
//there's no speeding up between blocks.
//#define PLANNER_OFF

//If defined sets the debug header (DEBUG_PIN1/2/3/4/5/6/7/8) to the given value
#ifdef DEBUG_ENABLE
#define DEBUG_VALUE(x)	setDebugValue(x)
#define DEBUG_MAIN        (0x00 << 5)
#define DEBUG_MOTHERBOARD (0x01 << 5)
#define DEBUG_STEPPERS    (0x02 << 5)
#else
#define DEBUG_VALUE(x)
#endif

//If defined provides 2 debugging variables for on screen display during build
//Variables are floats:  debug_onscreen1, debug_onscreen2 and can be found in Steppers.hh
//#define DEBUG_ONSCREEN

//If defined, the stack is painted with a value and the free sram reported in
//in the Version menu.  This enables debugging to see if the SRAM was ever exhausted
//which would lead to stack corruption.
#define STACK_PAINT

//Definitions for the timer / counter  to use for the stepper interrupt
//Change this to a different 16 bit interrupt if you need to
#define STEPPER_OCRnA			OCR3A
#define STEPPER_TIMSKn			TIMSK3
#define STEPPER_OCIEnA			OCIE3A
#define STEPPER_TCCRnA			TCCR3A
#define STEPPER_TCCRnB			TCCR3B
#define STEPPER_TCCRnC			TCCR3C
#define STEPPER_TCNTn			TCNT3
#define STEPPER_TIMERn_COMPA_vect	TIMER3_COMPA_vect

#define ADVANCE_TIMER			2
#define ADVANCE_TIMSKn			TIMSK2
#define ADVANCE_OCRnA			OCR2A
#define ADVANCE_OCIEnA			OCIE2A
#define ADVANCE_TCCRnA			TCCR2A
#define ADVANCE_TCCRnB			TCCR2B
#define ADVANCE_CTC			0x02 // (WGM21)
#define ADVANCE_PRESCALE_64		0x04 // (CS22)
#define ADVANCE_TIMERn_COMPA_vect	TIMER2_COMPA_vect

//Oversample the dda to provide less jitter.
//To switch off oversampling, comment out
//2 is the number of bits, as in a bit shift.  So << 2 = multiply by 4
//= 4 times oversampling
//Obviously because of this oversampling is always a power of 2.
//Don't make it too large, as it will kill performance and can overflow int32_t
//#define OVERSAMPLED_DDA 2

#else

#define DEBUG_VALUE(x)

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
#define ACCELERATION_SLOWDOWN_LIMIT (BLOCK_BUFFER_SIZE/2)

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

//If BUILD_CLEAR_a isn't defined, then no motion along axis "a"
//is executed whilst clearing the build platform.

#ifndef BUILD_CLEAR_MARGIN
#define BUILD_CLEAR_MARGIN 5.0 // 5.0 mm
#endif

#ifndef BUILD_CLEAR_MARGIN_X
#define BUILD_CLEAR_MARGIN_X BUILD_CLEAR_MARGIN
#endif

#ifndef BUILD_CLEAR_MARGIN_Y
#define BUILD_CLEAR_MARGIN_Y BUILD_CLEAR_MARGIN
#endif

#ifndef BUILD_CLEAR_MARGIN_Z
#define BUILD_CLEAR_MARGIN_Z BUILD_CLEAR_MARGIN
#endif

#ifndef X_HOME_MIN

// ***** WARNING ***** Math for _X assumes X home offset is positive....
#define BUILD_CLEAR_X ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + X_AXIS * sizeof(uint32_t), stepperAxis[X_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN_X * stepperAxisStepsPerMM(X_AXIS)) )

#else

// ***** WARNING ***** Math for _X assumes X home offset is negative....
#define BUILD_CLEAR_X ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + X_AXIS * sizeof(uint32_t), stepperAxis[X_AXIS].max_axis_steps_limit) + (int32_t)(BUILD_CLEAR_MARGIN_X * stepperAxisStepsPerMM(X_AXIS)) )

#endif

#ifndef Y_HOME_MIN

// ***** WARNING ***** Math for __Y assumes Y home offsets is positive....
#define BUILD_CLEAR_Y ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Y_AXIS * sizeof(uint32_t), stepperAxis[Y_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN_Y * stepperAxisStepsPerMM(Y_AXIS)) )

#else

// ***** WARNING ***** Math for __Y assumes Y home offsets is negative....
#define BUILD_CLEAR_Y ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Y_AXIS * sizeof(uint32_t), stepperAxis[Y_AXIS].max_axis_steps_limit) + (int32_t)(BUILD_CLEAR_MARGIN_Y * stepperAxisStepsPerMM(Y_AXIS)) )

#endif

// Move 5mm away from extruder along the Z axis
// Code in platformAccess() will assure that we do not move too far or in the wrong direction

#define BUILD_CLEAR_Z ( currentPosition[Z_AXIS] + (int32_t)(BUILD_CLEAR_MARGIN_Z * stepperAxisStepsPerMM(Z_AXIS)) )

//When pausing, filament is retracted to stop stringing / blobbing.
//This sets the amount of filament in mm's to be retracted
#define PAUSE_RETRACT_FILAMENT_AMOUNT_MM        2.0

//When defined, the Ditto Printing setting is added to General Settings
#define DITTO_PRINT

//When defined, the Z axis is clipped to it's maximum limit
//Applicable to old, bogus gcode MBI supplied for the Replicator 1.
//#define CLIP_Z_AXIS

// Our software variant id for the advanced version command
#define SOFTWARE_VARIANT_ID 0x80

// When defined, acceleration stats are displayed on the LCD screen
//#define ACCEL_STATS

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

// When defined, VREF for the Z axis may be set above 40
//#define DIGI_POT_HIGH_Z_VREF

#if defined(BUILD_STATS) && !defined(ESTIMATE_TIME)
#define ESTIMATE_TIME 1
#endif

#endif // BOARDS_MBV40_CONFIGURATION_HH_
