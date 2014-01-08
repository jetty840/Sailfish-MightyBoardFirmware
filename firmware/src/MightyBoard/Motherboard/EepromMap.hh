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
#if 0
	const static uint32_t DUAL_X_OFFSET_MM = 152L;
        const static uint32_t SINGLE_X_OFFSET_MM = 152L;
	const static uint32_t DUAL_Y_OFFSET_MM = 75L;
	const static uint32_t SINGLE_Y_OFFSET_MM = 72L;
#endif
#ifdef MODEL_REPLICATOR2
	const static uint32_t DUAL_X_OFFSET_STEPS   = 13463L;
        const static uint32_t SINGLE_X_OFFSET_STEPS = 13463L;
	const static uint32_t DUAL_Y_OFFSET_STEPS   =  6643L;
	const static uint32_t SINGLE_Y_OFFSET_STEPS =  6377L;
#else
	const static uint32_t DUAL_X_OFFSET_STEPS   = 14309L;
        const static uint32_t SINGLE_X_OFFSET_STEPS = 14309L;
	const static uint32_t DUAL_Y_OFFSET_STEPS   =  7060L;
	const static uint32_t SINGLE_Y_OFFSET_STEPS =  6778L;
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
	// These are the maximum lengths of all axis, and are populated from Replicator G
	// on connection.  These are reasonable defaults for X/Y/Z/A/B
	// Each one is the length(in mm's) * steps_per_mm  (from the xml file and the result is rounded down)
#ifdef MODEL_REPLICATOR
    // Replicator 1
    const static uint32_t axis_lengths[5] = {227L, 148L, 150L, 100000L, 100000L};
#else
#ifdef SINGLE_EXTRUDER
    // Replicator 2
    const static uint32_t axis_lengths[5] = {285L, 152L, 155L, 100000L, 100000L};
#else
    // Replicator 2X
    const static uint32_t axis_lengths[5] = {246L, 152L, 155L, 100000L, 100000L};
#endif
#endif
}

namespace replicator_axis_max_feedrates{
	// These are the maximum feedrates of all axis, and are populated from Replicator G
	// on connection.  These are reasonable defaults for X/Y/Z/A/B
	// Each one is the feedrate in mm per minute (extruders are the feedrate of the input filament)
	const static uint32_t axis_max_feedrates[5] = {18000, 18000, 1170, 1600, 1600};
}

namespace replicator_axis_steps_per_mm{
#ifdef MODEL_REPLICATOR2
	const static uint32_t axis_steps_per_mm[5] = { 88573186, 88573186, 400000000, 96275202, 96275202};
#else
	const static uint32_t axis_steps_per_mm[5] = { 94139704, 94139704, 400000000, 96275202, 96275202};
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

//// Feature map: 2 bytes
//$BEGIN_ENTRY
//$type:BB $ignore:True $constraints:a
const static uint16_t FEATURES			= 0x0000;
/// Backoff stop time, in ms: 2 bytes
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t BACKOFF_STOP_TIME         = 0x0002;
/// Backoff reverse time, in ms: 2 bytes
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t BACKOFF_REVERSE_TIME      = 0x0004;
/// Backoff forward time, in ms: 2 bytes
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t BACKOFF_FORWARD_TIME      = 0x0006;
/// Backoff trigger time, in ms: 2 bytes
//$BEGIN_ENTRY
//$type:H $ignore:True $constraints:a
const static uint16_t BACKOFF_TRIGGER_TIME      = 0x0008;
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
//$type:BB $ignore:True $constraints:a
const static uint16_t EXTRA_FEATURES            = 0x0016;
/// Extruder identifier; defaults to 0: 1 byte 
/// Padding: 1 byte of space
//$BEGIN_ENTRY
//$type:B $ignore:True $constraints:a
const static uint16_t SLAVE_ID                  = 0x0018;
/// Cooling fan info: 2 bytes 
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
#define PROFILES_HOME_POSITIONS_STORED 3	//X,Y,Z = 3

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
/// Thermistor table 0: 128 bytes
//$BEGIN_ENTRY
//$eeprom_map:therm_eeprom_offsets $ignore:True
const static uint16_t THERM_TABLE				= 0x0074;
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
/// start of free space
const static uint16_t FREE_EEPROM_STARTS        = 0x020B;

//Sailfish specific settings work backwards from the end of the eeprom 0xFFF

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

#define DEFAULT_EXTRUDER_DEPRIME_STEPS_A 16
#define DEFAULT_EXTRUDER_DEPRIME_STEPS_B 16

#define DEFAULT_SLOWDOWN_FLAG 0x01
#define DEFAULT_EXTRUDER_HOLD 0x00
#define DEFAULT_TOOLHEAD_OFFSET_SYSTEM 0x01
#define DEFAULT_SD_USE_CRC    0x00

#define ACCELERATION_INIT_BIT 7

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


/** thermal EERROM offset values and on/off settings for each heater */
namespace therm_eeprom_offsets{
//$BEGIN_ENTRY
//$type:i $constraints:a
const static uint16_t THERM_R0_OFFSET                   = 0x00;
//$BEGIN_ENTRY
//$type:i $constraints:a
const static uint16_t THERM_T0_OFFSET                   = 0x04;
//$BEGIN_ENTRY
//$type:i $constraints:a
const static uint16_t THERM_BETA_OFFSET                 = 0x08;
/// this is legacy storage for alternate thermistor tables
//$BEGIN_ENTRY
//$type:B $mult:40 $ignore:True $constraints:a
const static uint16_t THERM_DATA_OFFSET                 = 0x10;
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
    void factoryResetEEPROM();
    void fullResetEEPROM();
    void setToolHeadCount(uint8_t count);
    void setCustomColor(uint8_t red, uint8_t green, uint8_t blue);
    bool isSingleTool();
    bool hasHBP();
    void setDefaultsAcceleration();
    void storeToolheadToleranceDefaults();
    void updateBuildTime(uint8_t new_hours, uint8_t new_minutes);
    void setDefaultAxisHomePositions();
    void setDefaultsProfiles(uint16_t eeprom_base);
    void getBuildTime(uint16_t *hours, uint8_t *minutes);
    void setBuildTime(uint16_t hours, uint8_t minutes);
    bool heatLights();
}
#endif // EEPROMMAP_HH
