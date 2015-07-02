/*
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

// Electronics type
#define BOARD_TYPE		BOARD_TYPE_AZTEEG_X3
#define HAS_VIKI_INTERFACE	1
#define ENABLE_I2C_PULLUPS	1

// Swap the Azteeg X3's X & Y min and max endstops
// This is done since most makerbot-style printers use X max and Y max
// homing BUT on the X3, the convenient X & Y endstop terminals are
// for X & Y min.  By swapping them, we make them the X & Y max endstops.
#define SWAP_ENDSTOPS_X		1
#define SWAP_ENDSTOPS_Y		1

// Bot type used in stream version command
#define BOT_TYPE		0xB015

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.
#define STEPPER_COUNT		5
#define MAX_STEPPERS		5
#define EXTRUDERS		2

// microstepping is 1 / (1 << MICROSTEPPING)
//  0 for 1/1
//  1 for 1/2
//  2 for 1/4
//  3 for 1/8
//  4 for 1/16
//  5 for 1/32
//  etc.
#define MICROSTEPPING		4

#ifndef SIMULATOR

// This file details the pin assignments and features of the Panucatt Azteeg X3

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
// #define SD_WRITE_PIN	Pin(PortX,?)
// Azteeg and ViKi use microSD cards which do not have a read-only switch
#define SD_NO_WRITE_LOCK

// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN	Pin(PortL,0)

// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN	Pin(PortB,0)

// --- Slave UART configuration ---
// The slave UART is presumed to be an RS485 connection through a sn75176 chip.
// Define as 1 if the slave UART is present; 0 if not.
#define HAS_SLAVE_UART		0

// --- Host UART configuration ---
// The host UART is presumed to always be present on the RX/TX lines.

// --- Piezo Buzzer configuration ---
// Define as 1 if the piezo buzzer is present, 0 if not.
#define HAS_BUZZER		1

// The pin that drives the buzzer
#define BUZZER_TIMER		4
#define BUZZER_PIN		Pin(PortC,4)  // OC4A
#define BUZZER_SOFT_PWM		TIMER4_COMPA_vect

// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS	1

//Stepper Ports
#define X_STEPPER_STEP		STEPPER_PORT(F,0)	//active rising edge
#define X_STEPPER_DIR		STEPPER_PORT(F,1)	//forward on high
#define X_STEPPER_ENABLE	STEPPER_PORT(D,7)	//active low

#if defined(SWAP_ENDSTOPS_X)
#define X_STEPPER_MAX		STEPPER_PORT(E,5)	//active high
#define X_STEPPER_MIN		STEPPER_PORT(E,4)	//active high
#else
#define X_STEPPER_MIN		STEPPER_PORT(E,5)	//active high
#define X_STEPPER_MAX		STEPPER_PORT(E,4)	//active high
#endif

#define Y_STEPPER_STEP		STEPPER_PORT(F,6)	//active rising edge
#define Y_STEPPER_DIR		STEPPER_PORT(F,7)	//forward on high
#define Y_STEPPER_ENABLE	STEPPER_PORT(F,2)	//active low

#if defined(SWAP_ENDSTOPS_Y)
#define Y_STEPPER_MAX		STEPPER_PORT(J,1)	//active high
#define Y_STEPPER_MIN		STEPPER_PORT(J,0)	//active high
#else
#define Y_STEPPER_MIN		STEPPER_PORT(J,1)	//active high
#define Y_STEPPER_MAX		STEPPER_PORT(J,0)	//active high
#endif

// P-Stop is X_STEPPER_MIN = PE5 = OC3C/INT5 || PE4; neither are PCINT pins
#if defined(SWAP_ENDSTOPS_X)
#define PSTOP_PORT	Pin(PORTE,4)
#else
#define PSTOP_PORT	Pin(PORTE,5)
#endif

// Skip using an interrupt vector for now.  When we use one, we then
// have to do an initial probe to see if the state is LOW from the
// beginning.  And it's less code space usage to not have the interrupt
// routine -- saves 150 bytes!

#ifdef PSTSOP_VECT
#undef PSTOP_VECT
#endif
//#define PSTOP_MSK	PCMSK1
//#define PSTOP_PCINT	PCINT11
//#define PSTOP_PCIE	PCIE1
//#define PSTOP_VECT	PCINT1_vect

#define PSTOP2_PORT	Pin(PORTK,2)
#define PSTOP2_MSK	PCMSK2
#define PSTOP2_PCINT	PCINT18
#define PSTOP2_PCIE	PCIE2
#define PSTOP2_VECT	PCINT2_vect

#define Z_STEPPER_STEP		STEPPER_PORT(L,3)	//active rising edge
#define Z_STEPPER_DIR		STEPPER_PORT(L,1)	//forward on high
#define Z_STEPPER_ENABLE	STEPPER_PORT(K,0)	//active low
#define Z_STEPPER_MIN		STEPPER_PORT(D,3)	//active high
#define Z_STEPPER_MAX		STEPPER_PORT(D,2)	//active high

#ifdef PSTOP_ZMIN_LEVEL
// The Z min endstop is used
#define Z_MIN_STOP_PORT		Pin(PortD,3)
#endif

#define A_STEPPER_STEP		STEPPER_PORT(A,4)	//active rising edge
#define A_STEPPER_DIR		STEPPER_PORT(A,6)	//forward on high
#define A_STEPPER_ENABLE	STEPPER_PORT(A,2)	//active low

#define B_STEPPER_STEP		STEPPER_PORT(C,1)	//active rising edge
#define B_STEPPER_DIR		STEPPER_PORT(C,3)	//forward on high
#define B_STEPPER_ENABLE	STEPPER_PORT(C,7)	//active low

// ViKi Encoder pins
// PA0
#define VIKI_ENC_PORT_A		PORTA
#define VIKI_ENC_DDR_A		DDRA
#define VIKI_ENC_PIN_A		PINA
#define VIKI_ENC_MASK_A		0b00000001

// PH4
#define VIKI_ENC_PORT_B		PORTH
#define VIKI_ENC_DDR_B		DDRH
#define VIKI_ENC_PIN_B		PINH
#define VIKI_ENC_MASK_B		0b00010000

#define DIGIPOT_SUPPORT		1

#define X_POT_PIN		0 // P0W on MCP4451 00 (A1=0 A0=0)
#define Y_POT_PIN		1 // P1W on MCP4451 00 (A1=0 A0=0)
#define Z_POT_PIN		2 // P2W on MCP4451 00 (A1=0 A0=0)
#define A_POT_PIN		3 // P3W on MCP4451 00 (A1=0 A0=0)
#define B_POT_PIN		4 // P0W on MCP4451 10 (A1=1 A0=0)

#define X_POT_DEFAULT		127
#define Y_POT_DEFAULT		127
#define Z_POT_DEFAULT		127
#define A_POT_DEFAULT		127
#define B_POT_DEFAULT		127

// i2c pots SCL pin
//#define POTS_SCL		Pin(PortA,6)

// --- Debugging configuration ---
// The pin which controls the debug LED (active high)
#define DEBUG_PIN	Pin(PortB,7)  // X3's SD LED pin; Alternate choice PL7 == J4 D42
#define DEBUG_PIN1	Pin(PortL,7)  // J4 D42
#define DEBUG_PIN2	Pin(PortL,6)  // J4 D43
#define DEBUG_PIN3	Pin(PortL,5)  // J4 D44
#define DEBUG_PIN4	Pin(PortL,4)  // J4 D45
#define DEBUG_PIN5	Pin(PortG,0)  // J3 D41
#define DEBUG_PIN6	Pin(PortG,1)  // J3 D40
#define DEBUG_PIN7	Pin(PortC,0)  // J3 D37
#define DEBUG_PIN8	Pin(PortC,2)  // J3 D35

// Analog pin for reading an Analog Button Panel
#define ANALOG_BUTTONS_PIN	 0

#ifdef HAS_ANALOG_BUTTONS
//PortF,0 is the ADC0, since we're using that for the buttons,
//Redefine another unused debug pin for DEBUG_PIN4
#undef DEBUG_PIN4
#define DEBUG_PIN4	Pin(PortF,4)
#endif

/// Analog pins for reading digipot output (VREF Pins)
#define XVREF_Pin	NULL
#define YVREF_Pin	NULL
#define ZVREF_Pin	NULL
#define AVREF_Pin	NULL
#define BVREF_Pin	NULL

// By default, debugging packets should be honored; this is made
// configurable if we're short on cycles or EEPROM.
// Define as 1 if debugging packets are honored; 0 if not.
#define HONOR_DEBUG_PACKETS	0

#define HAS_INTERFACE_BOARD	1
#define HAS_VIKI_INTERFACE	1

/// Character LCD screen geometry
#define LCD_SCREEN_WIDTH	20
#define LCD_SCREEN_HEIGHT	4

///// **** HBP and Extruder  ***************/////

// ADC pins for thermistor and thermocouple connections

#define HBP_THERMISTOR_PIN			14	// PK6 / ADC14
#define EXA_THERMISTOR_PIN			13	// PK5 / ADC13
#define EXB_THERMISTOR_PIN			15	// PK7 / ADC15
#define EXA_THERMOCOUPLE_PIN			11	// PK3 / ADC11
#define EXB_THERMOCOUPLE_PIN			4	// PF4 / ADC04

// Following are used to quiesce unused ADC pins
#define EXA_THERMISTOR_DDR			DDRK
#define EXA_THERMISTOR_PORT			PORTK
#define EXA_THERMISTOR_MASK			0b00100000

#define EXB_THERMISTOR_DDR			DDRK
#define EXB_THERMISTOR_PORT			PORTK
#define EXB_THERMISTOR_MASK			0b10000000

#define EXA_THERMOCOUPLE_DDR			DDRK
#define EXA_THERMOCOUPLE_PORT			PORTK
#define EXA_THERMOCOUPLE_MASK			0b00001000

#define EXB_THERMOCOUPLE_DDR			DDRF
#define EXB_THERMOCOUPLE_PORT			PORTF
#define EXB_THERMOCOUPLE_MASK			0b00010000

#define USE_THERMOCOUPLE_DUAL		1
#define FOO_ARG(x)			0
#define THERMOCOUPLE_SCK		0
#define THERMOCOUPLE_SO			0
#define DEFAULT_THERMOCOUPLE_VAL	1024

// Unused ADC pins -- we turn them off so they don't
// float and introduce ADC noise.

// ADC 00 / PF0 -- Used, X Step
// ADC 01 / PF1 -- Used, X Dir
// ADC 02 / PF2 -- Used, Y Enable
// ADC 03 / PF3 --
// ADC 04 / PF4 -- Used, ADC 04, Thermocouple, Tool 1
// ADC 05 / PF5 -- 
// ADC 06 / PF6 -- Used, Y Step
// ADC 07 / PF7 -- Used, Y Dir
//
// ADC 08 / PK0 -- Used, Z Enable
// ADC 09 / PK1 --
// ADC 10 / PK2 -- Used, secondary P-Stop on ViKi
// ADC 11 / PK3 -- Used, ADC 11, Thermocouple, Tool 0
// ADC 12 / PK4 --
// ADC 13 / PK5 -- Used, ADC 13, Thermistor, Tool 0
// ADC 14 / PK6 -- Used, ADC 14, Thermistor, Platform
// ADC 15 / PK7 -- Used, ADC 15, Thermistor, Tool 1

#define ADC_UNUSED_DDR1				DDRF
#define ADC_UNUSED_PORT1			PORTF
#define ADC_UNUSED_MASK1			0b00101000
#define ADC_UNUSED_DDR2				DDRK
#define ADC_UNUSED_PORT2			PORTK
#if defined(HAS_VIKI_INTERFACE) || defined(HAS_VIKI2_INTERFACE)
#define ADC_UNUSED_MASK2			0b00010010
#else
#define ADC_UNUSED_MASK2			0b00010110
#endif

/// POWER Pins for extruders, fans and heated build platform
#define EXA_PWR			Pin(PortB,4)	// OC2A
#define EXA_PWR_OCRn		OCR2A
#define EXA_PWR_TCCRn		TCCR2A
#define EXA_PWR_TCCRn_on	0b10000000
#define EXA_PWR_TCCRn_off	0b00111111

#define EXB_PWR			Pin(PortH,6)	// OC2B
#define EXB_PWR_OCRn		OCR2B
#define EXB_PWR_TCCRn		TCCR2A
#define EXB_PWR_TCCRn_on	0b00100000
#define EXB_PWR_TCCRn_off	0b11001111

#define HBP_HEAT		Pin(PortH,5)	// OC4C

// Extruder heat sink fans
// We use the extra two extruder/fan outputs on the expansion board for these.
// First, the firmware doesn't support more than two extruders.  Second, they're
// nice screw down terminals and easy to use.
#define EXA_FAN			Pin(PortH,0)	// OC4B / D17 / HOT-END 3 / FAN
#define EXB_FAN			Pin(PortH,1)	// OC3B / D16 / HOT-END 4 / FAN

// Print cooling fan
#define ACTIVE_COOLING_FAN
#define EX_FAN			Pin(PortG,5)	// OC0B / D4
#define EXTRA_FET		EX_FAN

// Rep 1: wait 0.5 seconds, then handle Extruder 1
//        wait tiny amount, then handle Extruder 2
//
// Sampling rate is 2 Hz

// Rep 2: wait 0.25 seconds, sample ambient (cold junction)
//        wait 0.25 seconds, sample extruder 1
//        wait 0.25 seconds, sample extruder 2
//
// Sampling is thus 1.333 Hz

// 1/6 s Read A, initiate read B
// 2/6 s Read B, initiate read HBP
// 3/6 s Read HBP, initiate read A
//
// Sampling is 2 Hz

#if defined(SAMPLE_INTERVAL_MICROS_THERMISTOR)
#undef SAMPLE_INTERVAL_MICROS_THERMISTOR
#endif

//#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE 166667L
// 1000000 us / ( <number of sensors> * <ADC samples per temp> * <temps per sensor per second>)
// 1000000 us / ( TEMP_NSENSORS * TEMP_OVERSAMPLE * TEMPS_PER_SECOND_PER_SENSOR )
// 1000000 us / ( 3 * 8 * 2 )
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE 20833L

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

#define ADVANCE_TIMER			0
#define ADVANCE_TIMSKn			TIMSK0
#define ADVANCE_OCRnA			OCR0A
#define ADVANCE_OCIEnA			OCIE0A
#define ADVANCE_TCCRnA			TCCR0A
#define ADVANCE_TCCRnB			TCCR0B
#define ADVANCE_CTC			0x02 // (WGM01)
#define ADVANCE_PRESCALE_64		0x03 // (CS01 | CS00)
#define ADVANCE_TIMERn_COMPA_vect	TIMER0_COMPA_vect

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

//If a BUILD_CLEAR_a value isn't defined, then no motion along axis "a"
//is executed whilst clearing the build platform.

#define BUILD_CLEAR_MARGIN 5.0 // 5.0 mm
#ifndef X_HOME_MIN

// ***** WARNING ***** Math for _X assumes X home offset is positive....
#define BUILD_CLEAR_X ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + X_AXIS * sizeof(uint32_t), stepperAxis[X_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(X_AXIS)) )

#else

// ***** WARNING ***** Math for _X assumes X home offset is negative....
#define BUILD_CLEAR_X ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + X_AXIS * sizeof(uint32_t), stepperAxis[X_AXIS].max_axis_steps_limit) + (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(X_AXIS)) )

#endif

#ifndef Y_HOME_MIN

// ***** WARNING ***** Math for __Y assumes Y home offsets is positive....
#define BUILD_CLEAR_Y ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Y_AXIS * sizeof(uint32_t), stepperAxis[Y_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(Y_AXIS)) )

#else

// ***** WARNING ***** Math for __Y assumes Y home offsets is negative....
#define BUILD_CLEAR_Y ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Y_AXIS * sizeof(uint32_t), stepperAxis[Y_AXIS].max_axis_steps_limit) + (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(Y_AXIS)) )

#endif

#ifndef Z_HOME_MAX

// ****** WARNING ***** Overall math for _Z, including [Z_AXIS].max_axis_steps_limit from StepperAxis.cc,
// assumes Z home offset/position is close to zero.
#define BUILD_CLEAR_Z (stepperAxis[Z_AXIS].max_axis_steps_limit)

#else

// We home to Z max and so we want to clear down to Z max - 5 mm.  This works UNLESS the build is so tall
// that when the pause occurs we're actually closer than 5 mm to the Z end stop.  In that case we actually
// drive the print back into the nozzle!!!!
#define BUILD_CLEAR_Z  ( (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + Z_AXIS * sizeof(uint32_t), stepperAxis[Z_AXIS].max_axis_steps_limit) - (int32_t)(BUILD_CLEAR_MARGIN * stepperAxisStepsPerMM(Z_AXIS)) )

#endif

//When pausing, filament is retracted to stop stringing / blobbing.
//This sets the amount of filament in mm's to be retracted
#define PAUSE_RETRACT_FILAMENT_AMOUNT_MM        2.0

//When defined, the Ditto Printing setting is added to General Settings
// Note ditto printing is okay for Rep 2X
#define DITTO_PRINT

//When defined, the Z axis is clipped to it's maximum limit
//Applicable to old, bogus gcode MBI supplied for the Replicator 1.
//#define CLIP_Z_AXIS

// Our software variant id for the advanced version command
#define SOFTWARE_VARIANT_ID 0x80

//When defined, acceleration stats are displayed on the LCD screen
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

// Safety Cutoff circuit
#ifndef CUTOFF_PRESENT
#define CUTOFF_PRESENT   0
#endif // CUTOFF_PRESENT

#endif // BOARDS_MBV40_CONFIGURATION_HH_
