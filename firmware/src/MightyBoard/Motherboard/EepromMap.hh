/*
 *  MW shows pids: 7.00, 0.33, 36.00
 *  RepG: 7.02745104, 0.32549, 36.141178
 */

/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
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


#ifndef EEPROMMAP_HH_
#define EEPROMMAP_HH_

#include <stdint.h>
#include "Model.hh"

enum LEDColors {
    LED_DEFAULT_WHITE = 0,
    LED_DEFAULT_RED,
    LED_DEFAULT_ORANGE,
    LED_DEFAULT_PINK,
    LED_DEFAULT_GREEN,
    LED_DEFAULT_BLUE,
    LED_DEFAULT_PURPLE,
    LED_DEFAULT_OFF,
    LED_DEFAULT_CUSTOM
};

#define DEFAULT_BUZZ_ON 1
#define DEFAULT_BUZZ_HEAT 1

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

#include "TemperatureTable.hh"

// For the Azteeg we default these to OFF since the bot may or may
// not have RGB LEDs installed.  And, running the software PWM needed
// is a waste of CPU cycles (at interrupt level) for something not
// present.

#define LED_DEFAULT_COLOR      LED_DEFAULT_OFF
#define LED_DEFAULT_HEAT_COLOR LED_DEFAULT_RED

#else

#define LED_DEFAULT_COLOR      LED_DEFAULT_WHITE
#define LED_DEFAULT_HEAT_COLOR LED_DEFAULT_RED

#endif

#define ALEVEL_MAX_ZDELTA_DEFAULT 200 // 200 steps = 0.5 mm
#define ALEVEL_MAX_ZDELTA_CALIBRATED 60 // 60 steps = 0.15 mm

// front probe position may be off by 0.1mm due to probe pushing
// force and Z rods weakness
#define ALEVEL_PROBE_P1_COMP  0 // 0.0 mm
#define ALEVEL_PROBE_P2_COMP  0 // 0.0 mm
#ifdef ZYYX_3D_PRINTER
#define ALEVEL_PROBE_P3_COMP 40 // 0.1 mm
#else
#define ALEVEL_PROBE_P3_COMP  0 // 0.0 mm
#endif

// X & Y offset between the probe and extruder in units of steps
//  +X is to the right; -X to the left
//  +Y is to the back; -Y is to the front
#define ALEVEL_PROBE_OFFSETS_Y  0
#if defined(ZYYX_3D_PRINTER)
// 27 mm * 88.573186 steps/mm
#define ALEVEL_PROBE_OFFSETS_X  2391
#else
#define ALEVEL_PROBE_OFFSETS_X  0
#endif

typedef struct {
     uint8_t  flags;      // == 1 if valid
     int32_t  max_zdelta; // Max allowed difference between P1z, P2z and P3z
     int32_t  p1[3];      // Probed point 1, units of steps
     int32_t  p2[3];      // Probed point 2, units of steps
     int32_t  p3[3];      // Probed point 3, units of steps
} auto_level_t;

#define ALEVEL_MAX_ZPROBE_HITS_DEFAULT  3
#define ALEVEL_ZPROBE_HITS_RESET_MM 3

/** EEPROM storage offsets for cooling data */
namespace cooler_eeprom_offsets{
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1
const static uint16_t ENABLE_OFFSET   =     0;
//$BEGIN_ENTRY
//$type:B $constraints:m,0,300 $unit:C
const static uint16_t SETPOINT_C_OFFSET  =  1;
}

/** EEPROM storage offsets for PID data */
namespace pid_eeprom_offsets{
//$BEGIN_ENTRY
//$type:H $floating_point:True $constraints:m,0,100
const static uint16_t P_TERM_OFFSET = 0;
//$BEGIN_ENTRY
//$type:H $floating_point:True $constraints:m,0,1
const static uint16_t I_TERM_OFFSET = 2;
//$BEGIN_ENTRY
//$type:H $floating_point:True $constraints:m,0,100
const static uint16_t D_TERM_OFFSET = 4;
}

/** EEPROM storage offsets for distance delta between toolheads
 *  and the ideal 'center' of the toolhead system, in steps
 */
namespace replicator_axis_offsets{

#if !defined(PLATFORM_X_OFFSET_STEPS)
#define X_OFFSET_STEPS 14309L
#else
#define X_OFFSET_STEPS PLATFORM_X_OFFSET_STEPS
#endif

#if !defined(PLATFORM_Y_OFFSET_STEPS)
#define Y_OFFSET_STEPS 6778L
#else
#define Y_OFFSET_STEPS PLATFORM_Y_OFFSET_STEPS
#endif

	/// Footnote:
	/// mm offsets
	/// XDUAL: 152mm,
	/// XSINGLE: 152mm,
	/// Y: 75mm
	/// YSINGLE: 72mm

	/// steps per mm (from replicator.xml in RepG/machines)
	/// XY : 94.139704
	/// Z : 400

}

namespace replicator_axis_lengths{
#if !defined(PLATFORM_AXIS_LENGTHS)
	// These are the maximum lengths of all axis, and are populated from Replicator G
	// on connection.  These are reasonable defaults for X/Y/Z/A/B
	// Each one is the length(in mm's) * steps_per_mm  (from the xml file and the result is rounded down)
	const static uint32_t axis_lengths[5] = {227L, 148L, 150L, 100000L, 100000L};
#else
	const static uint32_t axis_lengths[5] = PLATFORM_AXIS_LENGTHS;
#endif
}

namespace replicator_axis_max_feedrates{
#if !defined(PLATFORM_MAX_FEEDRATES)
	// These are the maximum feedrates of all axis, and are populated from Replicator G
	// on connection.  These are reasonable defaults for X/Y/Z/A/B
	// Each one is the feedrate in mm per minute (extruders are the feedrate of the input filament)
	const static uint32_t axis_max_feedrates[5] = {18000, 18000, 1170, 1600, 1600};
#else
	const static uint32_t axis_max_feedrates[5] = PLATFORM_MAX_FEEDRATES;
#endif
}

namespace replicator_axis_steps_per_mm{
#if !defined(PLATFORM_AXIS_STEPS_PER_MM)
	const static uint32_t axis_steps_per_mm[5] = { 94139704, 94139704, 400000000, 96275202, 96275202};
#else
	const static uint32_t axis_steps_per_mm[5] = PLATFORM_AXIS_STEPS_PER_MM;
#endif

	/// Footnote:
	/// Steps per mm for all axis, all values multiplied by 1,000,000
	/// These values are updated from the settings contained in the machines.xml
	/// (if different) when ReplicatorG connects to the bot
	/// X: 94.139704
	/// Y: 94.139704
	/// Z: 400
	/// A: 96.275201870333662468889989185642
	/// B: 96.275201870333662468889989185642
}

/**
 * structure define eeprom map for storing toolhead specific EEPROM
 * values. This is a sub-map of EEPROM offsets
 */
namespace toolhead_eeprom_offsets {
//// Uninitialized memory is 0xff.  0xff should never
//// be used as a valid value for initialized memory!

/// Extruder heater base location: 6 bytes
//$BEGIN_ENTRY
//$eeprom_map:pid_eeprom_offsets
const static uint16_t EXTRUDER_PID_BASE         = 0x000A;
/// HBP heater base location: 6 bytes data
//$BEGIN_ENTRY
//$eeprom_map:pid_eeprom_offsets $ignore:True
const static uint16_t HBP_PID_BASE              = 0x0010;
/// Extra features word: 2 bytes
//$BEGIN_ENTRY
//$eeprom_map:cooler_eeprom_offsets
const static uint16_t COOLING_FAN_SETTINGS 	= 	0x001A;
// TOTAL MEMORY SIZE PER TOOLHEAD = 28 bytes
}

/** EEPROM storage offsets for profiles */
namespace profile_offsets {
#define PROFILES_QUANTITY 4
#define PROFILE_NAME_SIZE 8
#define PROFILES_INITIALIZED 0xAC

#define PROFILES_HOME_POSITIONS_STORED 3	// X, Y, Z = 3

/// The name of the profile (8 bytes)
//$BEGIN_ENTRY
//$type:cccccccc $ignore:True
const static uint16_t PROFILE_NAME			= 0x0000;
/// Default locations for axis in steps for X/Y/Z axis
/// Same as AXIS_HOME_POSITIONS_STEPS but for only 3 axis
/// 3 x 32 bit = 12 bytes
//$BEGIN_ENTRY
//$type:iii $ignore:True $unit:steps
const static uint16_t PROFILE_HOME_POSITIONS_STEPS	= 0x0008;
//Preheat settings for
//$BEGIN_ENTRY
//$type:H $ignore:True
const static uint16_t PROFILE_PREHEAT_RIGHT_TEMP	= 0x0014;
//Preheat settings for
//$BEGIN_ENTRY
//$type:H $ignore:True
const static uint16_t PROFILE_PREHEAT_LEFT_TEMP		= 0x0016;
//Preheat settings for
//$BEGIN_ENTRY
//$type:H $ignore:True
const static uint16_t PROFILE_PREHEAT_PLATFORM_TEMP	= 0x0018;
// TOTAL MEMORY SIZE PER PROFILE = 26 bytes
#define PROFILE_SIZE      26
}


/**
 * structure to define the general EEPROM map for storing all kinds
 * of data onboard the bot
 */
namespace eeprom_offsets {
/// Firmware Version, low byte: 1 byte
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:m,0,100
const static uint16_t VERSION_LOW				= 0x0000;
/// Firmware Version, high byte: 1 byte
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:m,0,10
const static uint16_t VERSION_HIGH				= 0x0001;
/// Axis inversion flags: 1 byte.
/// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
/// Bit 7 is used for HoldZ OFF: 1 = off, 0 = on
//$BEGIN_ENTRY
//$type:B  $constraints:a $axis_expand:True  $tooltip:A Bitfield representing the XYZAB axes, with X as bit 0. If an axis is moving in the wrong direction, toggle the bit for that axis
const static uint16_t AXIS_INVERSION			= 0x0002;
/// Endstop inversion flags: 1 byte.
/// The endstops for axis N (where X=0, Y=1, etc.) are considered
/// to be logically inverted if the Nth bit is set.
/// Bit 7 is set to indicate endstops are present; it is zero to indicate
/// that endstops are not present.
/// Ordinary endstops (H21LOB et. al.) are inverted.
//$type:B $constraints:a $axis_expand:True
const static uint16_t ENDSTOP_INVERSION			= 0x0004;
/// Digital Potentiometer Settings : 5 Bytes
//$BEGIN_ENTRY
//$type:BBBBB $constraints:a $unit:scale(1-118)
const static uint16_t DIGI_POT_SETTINGS			= 0x0006;
/// axis home direction (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:a $axis_expand:True $tooltip:A Bitfield representing the XYZAB axes, with X as bit 0. If an axis is homing in the wrong direction, toggle the bit for that axis
const static uint16_t AXIS_HOME_DIRECTION 		= 0x000C;
/// Default locations for the axis in step counts: 5 x 32 bit = 20 bytes
//$BEGIN_ENTRY
//$type:IIIII $constraints:a $unit:steps
const static uint16_t AXIS_HOME_POSITIONS_STEPS	= 0x000E;
/// Name of this machine: 16 bytes (16 bytes extra buffer)
//$BEGIN_ENTRY
//$type:s  $length:16 $constraints:a
const static uint16_t MACHINE_NAME				= 0x0022;
/// Tool count : 1 byte
//$BEGIN_ENTRY
//$type:B  $constraints:l,1,2
const static uint16_t TOOL_COUNT 				= 0x0042;
/// Hardware ID. Must exactly match the USB VendorId/ProductId pair: 4 bytes
//$BEGIN_ENTRY
//$type:HH $ignore:True $constraints:a
const static uint16_t VID_PID_INFO				= 0x0044;
/// Version Number for internal releases
//$BEGIN_ENTRY
//$type:H  $ignore:True $constraints:a
const static uint16_t INTERNAL_VERSION			= 0x0048;
/// Version number to be tagged with Git Commit
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t COMMIT_VERSION			= 0x004A;
/// HBP Present or not
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 if this machine has a heated build platform; otherwise, uncheck or set to 0 if it does not.  The bot should be power cycled after changing this field.
const static uint16_t HBP_PRESENT			= 0x004C;
/// 40 bytes padding
/// Padding: 8 bytes
// Toolhead 0 data: 28 bytes (see above)
//$BEGIN_ENTRY
//$eeprom_map:toolhead_eeprom_offsets $tool_index:0
const static uint16_t T0_DATA_BASE				= 0x0100;
// Toolhead 0 data: 28 bytes (see above)
//$BEGIN_ENTRY
//$eeprom_map:toolhead_eeprom_offsets $tool_index:0
const static uint16_t T1_DATA_BASE				= 0x011C;
/// unused 8 bytes								= 0x0138;
/// Light Effect table. 3 Bytes x 3 entries
//$BEGIN_ENTRY
//$eeprom_map:blink_eeprom_offsets
const static uint16_t LED_STRIP_SETTINGS		= 0x0140;
/// Buzz Effect table. 4 Bytes x 3 entries
//$BEGIN_ENTRY
//$eeprom_map:buzz_eeprom_offsets
const static uint16_t BUZZ_SETTINGS		= 0x014A;
///  1 byte. 0x01 for 'never booted before' 0x00 for 'have been booted before)
///$BEGIN_ENTRY
///$type:B $ignore:True $constraints:l,0,1
///const static uint16_t FIRST_BOOT_FLAG  = 0x0156;
/// 7 bytes, short int x 3 entries, 1 byte on/off
//$BEGIN_ENTRY
//$eeprom_map:preheat_eeprom_offsets
const static uint16_t PREHEAT_SETTINGS = 0x0158;
/// 1 byte,  0x01 for help menus on, 0x00 for off
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:l,0,1 $tooltip:Display extra help during the load filament scripts? Any non-zero value indicates enables the display of extra help.
const static uint16_t FILAMENT_HELP_SETTINGS = 0x0160;
/// This indicates how far out of tolerance the toolhead0 toolhead1 distance is
/// in steps.  3 x 32 bits = 12 bytes
/// WARNING IF THE COUNT OF STORED VALUES IS REDUCED TO TWO
/// THEN HomeOffsetsModeScreen needs to be changed!!!
//$BEGIN_ENTRY
//$type:iii $constraints:m,-2000,20000 $unit:steps
const static uint16_t TOOLHEAD_OFFSET_SETTINGS = 0x0162;
/// Acceleraton settings 22 bytes: 1 byte (on/off), 2 bytes default acceleration rate,
//$BEGIN_ENTRY
//$eeprom_map:acceleration_eeprom_offsets
const static uint16_t ACCELERATION_SETTINGS	     = 0x016E;
/// 2 bytes bot status info bytes
//$BEGIN_ENTRY
//$type:BB $ignore:True $constraints:a
const static uint16_t BOT_STATUS_BYTES = 0x018A;
/// axis lengths XYZ AB 5*32bit = 20 bytes
//$BEGIN_ENTRY
//$type:iiiii $ignore:True $constraints:m,0,2147483647 $unit:steps
const static uint16_t AXIS_LENGTHS				= 0x018C;
/// total lifetime print hours, 3bytes
//$BEGIN_ENTRY
//$eeprom_map:build_time_offsets $ignore:True
const static uint16_t TOTAL_BUILD_TIME			= 0x01A0;
/// axis steps per mm XYZAB 5*32bit = 20 bytes
//$BEGIN_ENTRY
//$type:IIIII $constraints:a $unit:1,000,000 * steps/mm
const static uint16_t AXIS_STEPS_PER_MM		= 0x01A4;
/// Filament lifetime counter (in steps) 8 bytes (int64) x 2 (for 2 extruders)
//$BEGIN_ENTRY
//$type:qq $ignore:True $constraints:a
const static uint16_t FILAMENT_LIFETIME		= 0x01B8;
/// Filament trip counter (in steps) 8 bytes (int64) x 2 (for 2 extruders)
//$BEGIN_ENTRY
//$type:qq $ignore:True $constraints:a
const static uint16_t FILAMENT_TRIP		= 0x01C8;
/// Acceleraton settings 60 bytes: 1 byte (on/off) + acceleration settings
//$BEGIN_ENTRY
//$eeprom_map:acceleration2_eeprom_offsets
const static uint16_t ACCELERATION2_SETTINGS	 = 0x01D8;
/// axis max feedrates XYZAB 5*32bit = 20 bytes
//$BEGIN_ENTRY
//$type:IIIII $constraints:a $unit:mm/sec $ignore:True
const static uint16_t AXIS_MAX_FEEDRATES	 = 0x01F4;
/// Hardware configuration settings
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:l,1,2
const static uint16_t BOTSTEP_TYPE      = 0x0208;
/// temperature offset calibration: 1 byte x 3 heaters = 3 bytes
//$BEGIN_ENTRY
//$type:BBB $constraints:a $unit:scaling factor $ignore:True
const static uint16_t HEATER_CALIBRATION = 0x020A;

//EXTRUDER_DEPRIME_ON_TRAVEL (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:When set, the firmware will deprime the extruder on detected travel moves as well as on pauses, planned or otherwise.  When not set, the firmware will only deprime the extruder on pauses, planned or otherwise.  Unplanned pauses occur when the acceleration planner falls behind and the printer waits briefly for another segment to print.
const static uint16_t EXTRUDER_DEPRIME_ON_TRAVEL        = 0x020B;

const static uint16_t FREE_EEPROM_STARTS        = 0x020C;

//Sailfish specific settings work backwards from the end of the eeprom 0xFFF

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

// Azteeg X3 thermistor table indices
// [0] - tool 0
// [1] - tool 1
// [2] - hbp
//$BEGIN_ENTRY
//$type:BBB
const static uint16_t TEMP_TABLE_INDICES = 0x0F45;

#ifdef THERM_INDEX_EPCOS
#define DEFAULT_THERM_TABLE_EXT THERM_INDEX_EPCOS
#define DEFAULT_THERM_TABLE_HBP THERM_INDEX_EPCOS
#else
#define DEFAULT_THERM_TABLE_EXT TABLE_EXT_THERMISTOR
#define DEFAULT_THERM_TABLE_HBP TABLE_HBP_THERMISTOR
#endif

#endif

//P-Stop enable (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check if the P-Stop switch is an inverted switch and goes HIGH when triggered and LOW when not triggered.  Uncheck if using a typical endstop switch which goes LOW when triggered.
const static uint16_t PSTOP_INVERTED                    = 0x0F48;

//Storage for X & Y probe offsets from extruder nozzle
// 2 x 32 bit = 8 bytes
//$BEGIN_ENTRY
//$type:ii $unit:steps $tooltip:The X and Y displacements of the auto-leveling Z probe from the extruder nozzle.  negative X values are to the left of the nozzle while positive X values are to the right.  Negative Y values are in front of the nozzle while positive Y values are behind the nozzle.
const static uint16_t ALEVEL_PROBE_OFFSETS	        = 0x0F49;

//Storage for deflection compensation for each of the auto-leveling
//probing points.
// 3 x 32 bit = 12 bytes
//$BEGIN_ENTRY
//$type:iii $unit:steps $tooltip:The deflection compensation values for each of the auto-leveling probing points, P1, P2, and P3.  These values are stored in units of Z axis steps.
const static uint16_t ALEVEL_PROBE_COMP_SETTINGS	= 0x0F51;

//Fan PWM level (0 - 100)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,100 $tooltip:Set a blower strength (duty cycle) to use for the print cooling fan when it is activated by the print commands.  Select a value between 0% (off) and 100% (on full).  For example, if your fan is too strong, you may want to set this value to 50 so that the fan operates at 50% strength.  If set this value to 0, then the cooling fan will not activate at all when the print commands request it to.
const static uint16_t COOLING_FAN_DUTY_CYCLE    = 0x0F63;
#define COOLING_FAN_DUTY_CYCLE_DEFAULT 100

//Auto level max Z probe hits (0 = unlimited)
//$BEGIN_ENTRY
//$type:b $constraints:l,0,200 $tooltip:Trigger a pause if the auto-leveling probe registers too many hits during a print. Set to the value 0 to allow an unlimited number of hits without pausing; otherwise, set to a value in the range 1 to 200.
const static uint16_t ALEVEL_MAX_ZPROBE_HITS   = 0x0F64;

//Auto level reserved byte
//$BEGIN_ENTRY
//$type:B $ignore:True
const static uint16_t ALEVEL_FLAGS             = 0x0F65;

//Auto level max Z difference between probed points
//$BEGIN_ENTRY
//$type:i $unit:steps  $tooltip:The maximum vertical difference between any two probed leveling points may not exceed this value.  Default value is 50 steps (0.5 mm).
const static uint16_t ALEVEL_MAX_ZDELTA        = 0x0F66;

//Auto level probing point P1 = (X1, Y1, Z1)
//$BEGIN_ENTRY
//$type:iii $ignore:True $unit:steps
const static uint16_t ALEVEL_P1                = 0x0F6A;

//Auto level probing point P2 = (X2, Y2, Z2)
//$BEGIN_ENTRY
//$type:iii $ignore:True $unit:steps
const static uint16_t ALEVEL_P2                = 0x0F76;

//Auto level probing point P3 = (X3, Y3, Z3)
//$BEGIN_ENTRY
//$type:iii $ignore:True $unit:steps
const static uint16_t ALEVEL_P3                = 0x0F82;

//Stop clears build platform (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to instruct the printer to clear the build away from the extruder before stopping.  Uncheck or set to zero to immediately stop the printer (e.g., perform an Emergency Stop).
const static uint16_t CLEAR_FOR_ESTOP          = 0x0F8E;

//Alternate UART enable (1 byte)
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:l,0,1 $tooltip:Check or set to 1 to enable use of the alternate UART, UART1, for serial comms communication.  This UART will then be used instead of the USB interface for receiving s3g/x3g commands.  The USB interface must still be used for firmware updates.
const static uint16_t ENABLE_ALTERNATE_UART     = 0x0F8F;

//P-Stop enable (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to enable the optional Pause Stop hardware.  Set to zero or uncheck to disable.  The bot should be power cycled after changing this field.
const static uint16_t PSTOP_ENABLE              = 0x0F90;

//Use SD card CRC(1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to enable SD card error checkin.  Uncheck or set to 0 to disable.
const static uint16_t SD_USE_CRC                = 0x0F91;
//Extruder hold (1 byte)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to enable the Extruder Hold feature.  Uncheck or set to 0 to disable.
const static uint16_t EXTRUDER_HOLD             = 0x0F92;
//Toolhead offset system (1 byte; 0x00 == RepG 39; 0x01 == RepG 40+)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check to use the NEW dualstrusion system. Uncheck to use the old (RepG 39 and earlier) dualstrusion system.  The bot should be power cycled after changing this field.
const static uint16_t TOOLHEAD_OFFSET_SYSTEM    = 0x0F93;
///Location of the profiles, 4 x 26 bytes (PROFILES_QUANTITY * PROFILE_SIZE)
//$BEGIN_ENTRY
//$type:BB $ignore:True $constraints:a
const static uint16_t PROFILES_BASE		= 0x0F94;
///1 byte, set to PROFILES_INITIALIZED (0xAC) when profiles have been initialized
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:a
const static uint16_t PROFILES_INIT	        = 0x0FFC;
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to override non-zero gcode temperature settings with the pre-heat temperature settings.  Uncheck or set to zero to honor temperature settings in the gcode.
const static uint16_t OVERRIDE_GCODE_TEMP	= 0x0FFD;
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to leave heaters enabled for up to 30 minutes while the bot is pasued.  Uncheck or set to zero to disable the heaters upon entering a paused state.
const static uint16_t HEAT_DURING_PAUSE	        = 0x0FFE;
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to enable ditto printing. Uncheck or set to zero to disable ditto printing.
const static uint16_t DITTO_PRINT_ENABLED       = 0x0FFF;
}

#ifdef ZYYX_3D_PRINTER

#define DEFAULT_OVERRIDE_GCODE_TEMP     0
#define DEFAULT_PREHEAT_TEMP            230
#define DEFAULT_PREHEAT_HBP             0
#define DEFAULT_HEAT_DURING_PAUSE       0

#define DEFAULT_MAX_ACCELERATION_AXIS_X 850
#define DEFAULT_MAX_ACCELERATION_AXIS_Y 850
#define DEFAULT_MAX_ACCELERATION_AXIS_Z 50
#define DEFAULT_MAX_ACCELERATION_AXIS_A 5000
#define DEFAULT_MAX_ACCELERATION_AXIS_B 5000

#define DEFAULT_MAX_ACCELERATION_NORMAL_MOVE   850
#define DEFAULT_MAX_ACCELERATION_EXTRUDER_MOVE 850

#define DEFAULT_MAX_SPEED_CHANGE_X 12
#define DEFAULT_MAX_SPEED_CHANGE_Y 12
#define DEFAULT_MAX_SPEED_CHANGE_Z 12
#define DEFAULT_MAX_SPEED_CHANGE_A 100
#define DEFAULT_MAX_SPEED_CHANGE_B 100

#define DEFAULT_JKN_ADVANCE_K                  500             // 0.005 Multiplied by 100000
#define DEFAULT_JKN_ADVANCE_K2                 5500            // 0.055 Multiplied by 100000

#define DEFAULT_EXTRUDER_DEPRIME_STEPS_A 8
#define DEFAULT_EXTRUDER_DEPRIME_STEPS_B 8
#define DEFAULT_EXTRUDER_DEPRIME_ON_TRAVEL 0

#define DEFAULT_SLOWDOWN_FLAG 0x01
#define DEFAULT_EXTRUDER_HOLD 0x01
#define DEFAULT_TOOLHEAD_OFFSET_SYSTEM 0x01
#define DEFAULT_SD_USE_CRC    0x00

#define DEFAULT_PSTOP_ENABLE   0
#define DEFAULT_PSTOP_INVERTED 1

#define ACCELERATION_INIT_BIT 7

#else

#define DEFAULT_OVERRIDE_GCODE_TEMP     0
#define DEFAULT_PREHEAT_TEMP            230
#define DEFAULT_PREHEAT_HBP             100
#define DEFAULT_HEAT_DURING_PAUSE       0

#define DEFAULT_MAX_ACCELERATION_AXIS_X 1000
#define DEFAULT_MAX_ACCELERATION_AXIS_Y 1000
#define DEFAULT_MAX_ACCELERATION_AXIS_Z 150
#define DEFAULT_MAX_ACCELERATION_AXIS_A 2000
#define DEFAULT_MAX_ACCELERATION_AXIS_B 2000

#define DEFAULT_MAX_ACCELERATION_NORMAL_MOVE   2000
#define DEFAULT_MAX_ACCELERATION_EXTRUDER_MOVE 2000

#define DEFAULT_MAX_SPEED_CHANGE_X 15
#define DEFAULT_MAX_SPEED_CHANGE_Y 15
#define DEFAULT_MAX_SPEED_CHANGE_Z 10
#define DEFAULT_MAX_SPEED_CHANGE_A 20
#define DEFAULT_MAX_SPEED_CHANGE_B 20

#define DEFAULT_JKN_ADVANCE_K                  500             // 0.00850 Multiplied by 100000
#define DEFAULT_JKN_ADVANCE_K2                 5500            // 0.00900 Multiplied by 100000

#define DEFAULT_EXTRUDER_DEPRIME_STEPS_A  16
#define DEFAULT_EXTRUDER_DEPRIME_STEPS_B  16
#define DEFAULT_EXTRUDER_DEPRIME_ON_TRAVEL 0

#define DEFAULT_SLOWDOWN_FLAG 0x01
#define DEFAULT_EXTRUDER_HOLD 0x01
#define DEFAULT_TOOLHEAD_OFFSET_SYSTEM 0x01
#define DEFAULT_SD_USE_CRC    0x00

#define ACCELERATION_INIT_BIT 7

#define DEFAULT_PSTOP_ENABLE   0
#define DEFAULT_PSTOP_INVERTED 0

#endif // ZYYX_3D_PRINTER

namespace acceleration_eeprom_offsets{
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to use acceleration.  Uncheck or set to 0 for no acceleration.  Note that you must turn acceleration on to print safely at speeds over 50mm/s.
const static uint16_t ACCELERATION_ACTIVE         = 0x00;
//$BEGIN_ENTRY
//$type:H $constraints:a $unit:mm/s²
const static uint16_t MAX_ACCELERATION_NORMAL_MOVE  = 0x02; //uint16_t
//$BEGIN_ENTRY
//$type:HHHHH $constraints:a $unit:mm/s²
const static uint16_t MAX_ACCELERATION_AXIS     = 0x04; //5 * uint16_t
//$BEGIN_ENTRY
//$type:HHHHH $floating_point:True $constraints:a $unit:mm/s
const static uint16_t MAX_SPEED_CHANGE          = 0x0E; //5 * uint16_t
//$BEGIN_ENTRY
//$type:H $constraints:a $unit:mm/s²
const static uint16_t MAX_ACCELERATION_EXTRUDER_MOVE    = 0x18; //uint16_t
//$BEGIN_ENTRY
//$type:B $constraints:a $ignore:True
const static uint16_t DEFAULTS_FLAG         = 0x1A; //uint8_t Bit 7 == 1 is defaults written
}

namespace acceleration2_eeprom_offsets{
//$BEGIN_ENTRY
//$type:I $constraints:a $unit:factor * 100000
const static uint16_t JKN_ADVANCE_K         = 0x00; //uint32_t
//$BEGIN_ENTRY
//$type:I $constraints:a $unit:factor * 100000
const static uint16_t JKN_ADVANCE_K2        = 0x04; //uint32_t
//$BEGIN_ENTRY
//$type:HH $constraints:a $unit:steps
const static uint16_t EXTRUDER_DEPRIME_STEPS = 0x08; //2 * uint16_t (A & B axis)
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to enable automatic print slowdown when the queue of planned segments is running low.  Uncheck or set to 0 to disable automatic slowdown.
const static uint16_t SLOWDOWN_FLAG         = 0x0C; //uint8_t Bit 0 == 1 is slowdown enabled
const static uint16_t FUTURE_USE            = 0x0E; //18 bytes for future use
//0x1C is end of acceleration2 settings (28 bytes long)
}

namespace build_time_offsets{
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t HOURS = 0x00;
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:m,0,60
const static uint16_t MINUTES = 0x02;
}

// buzz on/off settings
namespace buzz_eeprom_offsets{
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to play bot sounds.  Uncheck or set to 0 for no sounds.
const static uint16_t SOUND_ON		= 0x00;
//$BEGIN_ENTRY
//$type:B $constraints:l,0,1 $tooltip:Check or set to 1 to always play temperatures reached tune.  Uncheck or set to 0 to mute the tune while printing.
const static uint16_t HEAT_BUZZ_OFFSET		= 0x02;
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:l,0,1
const static uint16_t ERROR_BUZZ_OFFSET 	= 0x04;
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:l,0,1
const static uint16_t DONE_BUZZ_OFFSET		= 0x08;
}

/** blink/LED EERROM offset values */

//Offset table for the blink entries. Each entry is an R,G,B entry
namespace blink_eeprom_offsets{
//$BEGIN_ENTRY
//$type:B $constraints:m,0,10
const static uint16_t BASIC_COLOR_OFFSET	= 0x00;
//$BEGIN_ENTRY
//$type:B $constraints:l,1,0
const static uint16_t LED_HEAT_OFFSET	= 0x02;
//$BEGIN_ENTRY
//$type:BBB $constraints:a $unit:RGB
const static uint16_t CUSTOM_COLOR_OFFSET 	= 0x04;
}


/** preheat EERROM offset values and on/off settings for each heater */
namespace preheat_eeprom_offsets{
//$BEGIN_ENTRY
//$type:H $constraints:m,0,280 $unit:C
const static uint16_t PREHEAT_RIGHT_TEMP                = 0x00;
//$BEGIN_ENTRY
//$type:H $constraints:m,0,280 $unit:C
const static uint16_t PREHEAT_LEFT_TEMP                = 0x02;
//$BEGIN_ENTRY
//$type:H $constraints:m,0,130 $unit:C
const static uint16_t PREHEAT_PLATFORM_TEMP           = 0x04;
// this byte is firmware local to note if preheat is active
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:a
const static uint16_t PREHEAT_ON_OFF_OFFSET             = 0x06;
}

/**
 * mask to set on/off settings for preheat
 */
enum HeatMask{
    HEAT_MASK_PLATFORM = 0,
    HEAT_MASK_LEFT = 1,
    HEAT_MASK_RIGHT = 2
};


namespace eeprom_info {

//$BEGIN_INFO_ENTRY
//$name:software_variant $value:0x80

//$BEGIN_INFO_ENTRY
//$name:dependent_toolhead_map $value:None

const static uint16_t EEPROM_SIZE = 0x0200;
const int MAX_MACHINE_NAME_LEN = 16;


/**
 * EXTRA_FEATURES Misc eeprom features
 */
enum {
	EF_SWAP_MOTOR_CONTROLLERS	= 1 << 0,
	EF_USE_BACKOFF			= 1 << 1,

	// Two bits to indicate mosfet channel.
	// Channel A = 0, B = 1, C = 2
	// Defaults:
	//   A - HBP heater
	//   B - extruder heater
	//   C - ABP motor
	EF_EX_HEATER_0			= 1 << 2,
	EF_EX_HEATER_1			= 1 << 3,
	EF_HBP_HEATER_0			= 1 << 4,
	EF_HBP_HEATER_1			= 1 << 5,
	EF_ABP_MOTOR_0			= 1 << 6,
	EF_ABP_MOTOR_1			= 1 << 7,

	// These are necessary to deal with horrible "all 0/all 1" problems
	// we introduced back in the day
	EF_ACTIVE_0				= 1 << 14,  // Set to 1 if EF word is valid
	EF_ACTIVE_1				= 1 << 15	// Set to 0 if EF word is valid
};

/**
 * This is the set of flags for the Toolhead Features memory
 */
enum {
        HEATER_0_PRESENT        = 1 << 0,
        HEATER_0_THERMISTOR     = 1 << 1,
        HEATER_0_THERMOCOUPLE   = 1 << 2,

        HEATER_1_PRESENT        = 1 << 3,
        HEATER_1_THERMISTOR     = 1 << 4,
        HEATER_1_THERMOCOUPLE   = 1 << 5,

        // Legacy settins for Cupcake and Thing-o-Matic
        DC_MOTOR_PRESENT                = 1 << 6,

        HBRIDGE_STEPPER                 = 1 << 8,
        EXTERNAL_STEPPER                = 1 << 9,
        RELAY_BOARD                     = 1 << 10,
        MK5_HEAD                        = 1 << 11
};



//const static uint16_t EF_DEFAULT = 0x4084;



}

namespace eeprom {
#if defined(ZYYX_3D_PRINTER)
    void factoryResetEEPROM(bool full_reset);
    void setDefaultAxisHomePositions(bool full_reset);
#   define FACTORYRESETEEPROM(x) eeprom::factoryResetEEPROM(x)
#   define SETDEFAULTAXISHOMEPOSITIONS(x) eeprom::setDefaultAxisHomePositions(x)
#else
    void factoryResetEEPROM();
    void setDefaultAxisHomePositions();
#   define FACTORYRESETEEPROM(x) eeprom::factoryResetEEPROM()
#   define SETDEFAULTAXISHOMEPOSITIONS(x) eeprom::setDefaultAxisHomePositions()
#endif
    void fullResetEEPROM();
    void setToolHeadCount(uint8_t count);
    void setCustomColor(uint8_t red, uint8_t green, uint8_t blue);
#ifdef HAS_RGB_LED
	LEDColors getColor();
	void setColor(uint8_t color);
#endif
    bool isSingleTool();
    bool hasHBP();
    void setDefaultsAcceleration();
    void storeToolheadToleranceDefaults();
    void updateBuildTime(uint16_t new_hours, uint8_t new_minutes);
    void setDefaultsProfiles(uint16_t eeprom_base);
    void getBuildTime(uint16_t *hours, uint8_t *minutes);
    void setBuildTime(uint16_t hours, uint8_t minutes);
#ifdef HAS_RGB_LED
    bool heatLights();
#endif

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
    uint8_t getThermistorTable(uint8_t idx);
    void setThermistorTable(uint8_t idx, uint8_t index);
#endif

}

#endif // EEPROMMAP_HH
