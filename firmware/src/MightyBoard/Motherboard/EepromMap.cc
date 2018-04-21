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

#include "Compat.hh"
#include "Model.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>
#include <avr/wdt.h>

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

//for thermistor generation
#include "TemperatureTable.hh"
#include "ThermocoupleReader.hh"

#endif

// for sound definition
#include "Piezo.hh"

// for LED definition
#ifdef HAS_RGB_LED
#include "RGB_LED.hh"
#endif

// for cooling fan definition
#include "CoolingFan.hh"

// To convert millimeters to steps
#include "StepperAxis.hh"

// For default machine name and machine id
#include "MachineId.hh"

namespace eeprom {

#define DEFAULT_P_VALUE  (7.0f)
#define DEFAULT_I_VALUE  (0.325f)
#define DEFAULT_D_VALUE  (36.0f)

/**
 *
 * @param eeprom_base start of eeprom map of cooling settings
 */
void setDefaultCoolingFan(uint16_t eeprom_base){

	uint8_t fan_settings[] = {1, DEFAULT_COOLING_FAN_SETPOINT_C};
    eeprom_write_block( fan_settings, (uint8_t*)(eeprom_base + cooler_eeprom_offsets::ENABLE_OFFSET),2);
}


/**
 * Start of PID block of EEPROM. Can be extruder or HPB
 * @param eeprom_base
 */
void setDefaultPID(uint16_t eeprom_base)
{
	setEepromFixed16(( eeprom_base + pid_eeprom_offsets::P_TERM_OFFSET ), DEFAULT_P_VALUE);
	setEepromFixed16(( eeprom_base + pid_eeprom_offsets::I_TERM_OFFSET ), DEFAULT_I_VALUE);
	setEepromFixed16(( eeprom_base + pid_eeprom_offsets::D_TERM_OFFSET ), DEFAULT_D_VALUE);
}


#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

void setDefaultsThermistorTables()
{
	uint8_t defs[3] = {
		DEFAULT_THERM_TABLE_EXT,
		DEFAULT_THERM_TABLE_EXT,
		DEFAULT_THERM_TABLE_HBP };
	eeprom_write_block((void*)defs,
			   (uint8_t*)(eeprom_offsets::TEMP_TABLE_INDICES),
			   sizeof(defs));
}

uint8_t getThermistorTable(uint8_t idx)
{
	uint8_t def = (idx != THERM_CHANNEL_HBP) ?
		DEFAULT_THERM_TABLE_EXT : DEFAULT_THERM_TABLE_HBP;
	uint8_t index = eeprom::getEeprom8(
		eeprom_offsets::TEMP_TABLE_INDICES + idx, def);
	if ( index > (TABLE_COUNT - 1) )
		index = def;
	return(index);
}

void setThermistorTable(uint8_t idx, uint8_t index)
{
	eeprom_write_byte((uint8_t*)(eeprom_offsets::TEMP_TABLE_INDICES + idx),
					  index);
}

#endif

/**
 *
 * @param index
 * @param eeprom_base start of this extruder's data in the eeprom
 */
void setDefaultsExtruder(uint16_t eeprom_base)
{
     setDefaultPID(eeprom_base + toolhead_eeprom_offsets::EXTRUDER_PID_BASE);
     setDefaultPID(eeprom_base + toolhead_eeprom_offsets::HBP_PID_BASE);
     setDefaultCoolingFan(eeprom_base + toolhead_eeprom_offsets::COOLING_FAN_SETTINGS);
}

typedef struct Color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} Color;

/**
 *
 * @param eeprom_base start of Led effects table
 */
void setDefaultLedEffects(uint16_t eeprom_base)
{
     // default color is white
     eeprom_write_byte(
	  (uint8_t*)(eeprom_base + blink_eeprom_offsets::BASIC_COLOR_OFFSET),
	  LED_DEFAULT_COLOR);
     eeprom_write_byte(
	  (uint8_t*)(eeprom_base + blink_eeprom_offsets::LED_HEAT_OFFSET),
	  LED_DEFAULT_HEAT_COLOR);

     Color colors;

     // Match default
     colors.red   = 0xFF;
     colors.green = 0xFF;
     colors.blue  = 0xFF;
     eeprom_write_block(
	  (void*)&colors,
	  (uint8_t*)(eeprom_base + blink_eeprom_offsets::CUSTOM_COLOR_OFFSET),
	  sizeof(colors));
}

#ifdef HAS_RGB_LED

LEDColors getColor() {
	return static_cast<LEDColors>(eeprom::getEeprom8(
		eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::BASIC_COLOR_OFFSET,
		LED_DEFAULT_COLOR));
}

void setColor(uint8_t color) {
	eeprom_write_byte((uint8_t*)eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::BASIC_COLOR_OFFSET, color);
}

#endif

    /**
     *
     * @param red value
     * @param green value
     * @param blue value
     */

void setCustomColor(uint8_t red, uint8_t green, uint8_t blue) {
	eeprom_write_byte(
	     (uint8_t*)(eeprom_offsets::LED_STRIP_SETTINGS +
			blink_eeprom_offsets::BASIC_COLOR_OFFSET),
	     LED_DEFAULT_CUSTOM);

	Color colors;
	colors.red   = red;
	colors.green = green;
	colors.blue  = blue;
	eeprom_write_block(
	     (void*)&colors,
	     (uint8_t*)(eeprom_offsets::LED_STRIP_SETTINGS +
			blink_eeprom_offsets::CUSTOM_COLOR_OFFSET),
	     sizeof(colors));
}

/**
 *
 * @param eeprom_base start of buzz effects table
 */
void setDefaultBuzzEffects(uint16_t eeprom_base)
{
	eeprom_write_byte((uint8_t *)(eeprom_base + buzz_eeprom_offsets::SOUND_ON), DEFAULT_BUZZ_ON);
	eeprom_write_byte((uint8_t *)(eeprom_base + buzz_eeprom_offsets::HEAT_BUZZ_OFFSET), DEFAULT_BUZZ_HEAT);
}

/**
 *
 * @param eeprom_base start of preheat settings table
 */
void setDefaultsPreheat(uint16_t eeprom_base)
{
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_RIGHT_TEMP), DEFAULT_PREHEAT_TEMP);
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_LEFT_TEMP), DEFAULT_PREHEAT_TEMP);
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP), DEFAULT_PREHEAT_HBP);
    eeprom_write_byte((uint8_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET), (1<<HEAT_MASK_RIGHT) + (1<<HEAT_MASK_PLATFORM));
}


/**
 *
 * break with the form here as eeprom_base is available in class and we
 * want to cleanly call this function externally
 */
void setDefaultsAcceleration()
{
	eeprom_write_byte((uint8_t *) (eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACCELERATION_ACTIVE), 0x01);

	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_AXIS + sizeof(uint16_t)*0), DEFAULT_MAX_ACCELERATION_AXIS_X);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_AXIS + sizeof(uint16_t)*1), DEFAULT_MAX_ACCELERATION_AXIS_Y);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_AXIS + sizeof(uint16_t)*2), DEFAULT_MAX_ACCELERATION_AXIS_Z);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_AXIS + sizeof(uint16_t)*3), DEFAULT_MAX_ACCELERATION_AXIS_A);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_AXIS + sizeof(uint16_t)*4), DEFAULT_MAX_ACCELERATION_AXIS_B);

#ifdef OLD_ACCEL_LIMITS
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_NORMAL_MOVE),   DEFAULT_MAX_ACCELERATION_NORMAL_MOVE);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_ACCELERATION_EXTRUDER_MOVE), DEFAULT_MAX_ACCELERATION_EXTRUDER_MOVE);
#endif

	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t)*0), DEFAULT_MAX_SPEED_CHANGE_X);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t)*1), DEFAULT_MAX_SPEED_CHANGE_Y);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t)*2), DEFAULT_MAX_SPEED_CHANGE_Z);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t)*3), DEFAULT_MAX_SPEED_CHANGE_A);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t)*4), DEFAULT_MAX_SPEED_CHANGE_B);

	eeprom_write_dword((uint32_t *)(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::JKN_ADVANCE_K),  DEFAULT_JKN_ADVANCE_K);
	eeprom_write_dword((uint32_t *)(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::JKN_ADVANCE_K2), DEFAULT_JKN_ADVANCE_K2);

	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::EXTRUDER_DEPRIME_STEPS + sizeof(uint16_t)*0), DEFAULT_EXTRUDER_DEPRIME_STEPS_A);
	eeprom_write_word((uint16_t *)(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::EXTRUDER_DEPRIME_STEPS + sizeof(uint16_t)*1), DEFAULT_EXTRUDER_DEPRIME_STEPS_B);
	eeprom_write_byte((uint8_t *)eeprom_offsets::EXTRUDER_DEPRIME_ON_TRAVEL, DEFAULT_EXTRUDER_DEPRIME_ON_TRAVEL);
	eeprom_write_byte((uint8_t *)(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::SLOWDOWN_FLAG), DEFAULT_SLOWDOWN_FLAG);

	eeprom_write_byte((uint8_t *)(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::DEFAULTS_FLAG), _BV(ACCELERATION_INIT_BIT));
}

/// Writes to EEPROM the default toolhead 'home' values to idicate toolhead offset
/// from idealized point-center of the toolhead
#if defined(ZYYX_3D_PRINTER)
void setDefaultAxisHomePositions(bool full_reset)
#else
void setDefaultAxisHomePositions()
#endif
{
	uint32_t homes[5] = {
		X_OFFSET_STEPS,
		Y_OFFSET_STEPS,
		0, 0, 0};
	size_t len;
#if defined(ZYYX_3D_PRINTER)
	len = full_reset ? sizeof(uint32_t) * 2 : sizeof(uint32_t) * 5;
#else
	len = sizeof(uint32_t) * 5;
#endif
	eeprom_write_block((uint8_t*)&(homes[0]),(uint8_t*)(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS), len );
}

/// Write to EEPROM the default profiles
/// These are Abs, Pla, Profile 3, Profile 4
/// Assumes setDefaultAxisHomePositions() has been called first
void setDefaultsProfiles(uint16_t eeprom_base) {
	uint32_t homeOffsets[PROFILES_HOME_POSITIONS_STORED];
	const char *profileNames[] = {"ABS", "PLA", "Profile3", "Profile4" };

	eeprom_read_block((void *)homeOffsets, (void *)eeprom_offsets::AXIS_HOME_POSITIONS_STEPS, PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));

	for (uint8_t i = 0; i < PROFILES_QUANTITY; i ++ ) {
		uint16_t profile_offset = eeprom_base + i * PROFILE_SIZE;

		//Note this will overflow the string when strlen + 1 < PROFILE_NAME_SIZE, however it doesn't
		//matter because it will overflow into the same array, and AVR isn't bright enough to segv,
		//so we ignore that and save the 20 odd cycles it would take to check the length.
		eeprom_write_block(profileNames[i],(uint8_t*)(profile_offset + profile_offsets::PROFILE_NAME), PROFILE_NAME_SIZE);

		eeprom_write_block((void *)homeOffsets,(void *)(profile_offset + profile_offsets::PROFILE_HOME_POSITIONS_STEPS), PROFILES_HOME_POSITIONS_STORED * sizeof(uint32_t));

    		eeprom_write_word((uint16_t*)(profile_offset + profile_offsets::PROFILE_PREHEAT_RIGHT_TEMP), DEFAULT_PREHEAT_TEMP);
    		eeprom_write_word((uint16_t*)(profile_offset + profile_offsets::PROFILE_PREHEAT_LEFT_TEMP), DEFAULT_PREHEAT_TEMP);
    		eeprom_write_word((uint16_t*)(profile_offset + profile_offsets::PROFILE_PREHEAT_PLATFORM_TEMP), (i == 1)?45:DEFAULT_PREHEAT_HBP);
	}

	//Initialize a flag to tell us profiles have been initialized
	eeprom_write_byte((uint8_t*)eeprom_offsets::PROFILES_INIT,PROFILES_INITIALIZED);
}


/// Does a factory reset (resets all defaults except home/endstops, axis direction, filament lifetime counter and tool count)
#if defined(ZYYX_3D_PRINTER)
void factoryResetEEPROM(bool full_reset)
#else
void factoryResetEEPROM()
#endif
{

#if !defined(PLATFORM_ENDSTOP_INVERT)
	// Default: enstops inverted, Z axis inverted
	uint8_t endstop_invert = 0b10011111; // all endstops inverted
#else
	uint8_t endstop_invert = PLATFORM_ENDSTOP_INVERT;
#endif

#if !defined(PLATFORM_HOME_DIRECTION)
	// X,Y Max, Z min  (AB max - to never halt on edge in stepper interface)
	uint8_t home_direction = 0b11011;
#else
	uint8_t home_direction = PLATFORM_HOME_DIRECTION;
#endif

#if !defined(PLATFORM_VREF_DEFAULTS)
	uint8_t vRefBase[] = { 118, 118, 40, 118, 118 };
#else
	uint8_t vRefBase[] = PLATFORM_VREF_DEFAULTS;
#endif

	/// Write 'MainBoard' settings
	eeprom_write_block(THE_REPLICATOR_STR,
			   (uint8_t*)eeprom_offsets::MACHINE_NAME,sizeof(THE_REPLICATOR_STR));

	eeprom_write_block(&(vRefBase[0]),(uint8_t*)(eeprom_offsets::DIGI_POT_SETTINGS), 5 );
	eeprom_write_byte((uint8_t*)eeprom_offsets::ENDSTOP_INVERSION, endstop_invert);
	eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_HOME_DIRECTION, home_direction);

	SETDEFAULTAXISHOMEPOSITIONS(full_reset);

	setDefaultsProfiles(eeprom_offsets::PROFILES_BASE);

	wdt_reset();

	/// store the default axis lengths for the machine
	eeprom_write_block((uint8_t*)&(replicator_axis_lengths::axis_lengths[0]), (uint8_t*)(eeprom_offsets::AXIS_LENGTHS), 20);

	/// store the default axis steps per mm for the machine
	eeprom_write_block((uint8_t*)&(replicator_axis_steps_per_mm::axis_steps_per_mm[0]), (uint8_t*)(eeprom_offsets::AXIS_STEPS_PER_MM), 20);

	/// store the default axis max feedrates for the machine
	eeprom_write_block((uint8_t*)&(replicator_axis_max_feedrates::axis_max_feedrates[0]), (uint8_t*)(eeprom_offsets::AXIS_MAX_FEEDRATES), 20);

	setDefaultsAcceleration();

	/// write MightyBoard VID/PID. Only after verification does production write
	/// a proper 'The Replicator' PID/VID to eeprom, and to the USB chip
	uint16_t vidPid[] = { 0x23C1, MACHINE_ID };
	eeprom_write_block(&(vidPid[0]), (uint8_t*)eeprom_offsets::VID_PID_INFO, 4);

	/// Write 'extruder 0' settings
	setDefaultsExtruder(eeprom_offsets::T0_DATA_BASE);

	/// Write 'extruder 1' stttings
	setDefaultsExtruder(eeprom_offsets::T1_DATA_BASE);

	// filament trip counter
	setEepromInt64(eeprom_offsets::FILAMENT_TRIP, 0);
	setEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t), 0);

	/// write blink and buzz defaults
	setDefaultLedEffects(eeprom_offsets::LED_STRIP_SETTINGS);
	setDefaultBuzzEffects(eeprom_offsets::BUZZ_SETTINGS);

	/// Preheat heater settings
	setDefaultsPreheat(eeprom_offsets::PREHEAT_SETTINGS);

	eeprom_write_byte((uint8_t*)eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);

	// Set override gcode temp to off
	eeprom_write_byte((uint8_t*)eeprom_offsets::OVERRIDE_GCODE_TEMP, DEFAULT_OVERRIDE_GCODE_TEMP);

	// Set heaters on during pause to a default of on
	eeprom_write_byte((uint8_t*)eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE);

#if defined(DITTO_PRINT) && EXTRUDERS > 1
	// Sets ditto printing, defaults to off
	eeprom_write_byte((uint8_t*)eeprom_offsets::DITTO_PRINT_ENABLED, 0);
#endif

	// Extruder hold
	eeprom_write_byte((uint8_t *)eeprom_offsets::EXTRUDER_HOLD, DEFAULT_EXTRUDER_HOLD);

#ifdef TOOLHEAD_OFFSET_SYSTEM
	// Toolhead offset system
	eeprom_write_byte((uint8_t *)eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
			  DEFAULT_TOOLHEAD_OFFSET_SYSTEM);
#endif

	// Use SD card CRC checking
	eeprom_write_byte((uint8_t *)eeprom_offsets::SD_USE_CRC, DEFAULT_SD_USE_CRC);

	setToolHeadCount(0);

	eeprom_write_byte((uint8_t*)eeprom_offsets::HBP_PRESENT,
					  DEFAULT_HBP_PRESENT);

	eeprom_write_byte((uint8_t*)eeprom_offsets::ENABLE_ALTERNATE_UART, 0);
	eeprom_write_byte((uint8_t*)eeprom_offsets::CLEAR_FOR_ESTOP, 0);

	{
	     int32_t dummy = ALEVEL_MAX_ZDELTA_DEFAULT;
	     eeprom_write_block(&dummy,
				(uint8_t*)eeprom_offsets::ALEVEL_MAX_ZDELTA,
				sizeof(int32_t));
	}
	eeprom_write_byte((uint8_t*)eeprom_offsets::ALEVEL_MAX_ZPROBE_HITS,
			  ALEVEL_MAX_ZPROBE_HITS_DEFAULT);

	eeprom_write_byte((uint8_t*)eeprom_offsets::COOLING_FAN_DUTY_CYCLE,
			  COOLING_FAN_DUTY_CYCLE_DEFAULT);

	{
	     uint32_t dummy[5] = { ALEVEL_PROBE_OFFSETS_X,
				   ALEVEL_PROBE_OFFSETS_Y,
				   ALEVEL_PROBE_P1_COMP,
				   ALEVEL_PROBE_P2_COMP,
				   ALEVEL_PROBE_P3_COMP };
	     eeprom_write_block(
		  (uint8_t*)dummy,
		  (uint8_t*)eeprom_offsets::ALEVEL_PROBE_OFFSETS,
		  sizeof(uint32_t)*5);
	}

#ifdef PSTOP_SUPPORT
	eeprom_write_byte((uint8_t*)eeprom_offsets::PSTOP_ENABLE,   DEFAULT_PSTOP_ENABLE);
	eeprom_write_byte((uint8_t*)eeprom_offsets::PSTOP_INVERTED, DEFAULT_PSTOP_INVERTED);
#endif

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
	setDefaultsThermistorTables();
#endif

}

void setToolHeadCount(uint8_t count) {

	// update toolhead count
#ifdef SINGLE_EXTRUDER
	// Replicator 2
	count = 1;
#elif BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
	// Default to single extruder
	if ( count == 0 )
	     count = 1;
#else
	// Replicator 1 or Replicator 2X
	if ( count != 1 )
	        count = 2;
#endif
	eeprom_write_byte((uint8_t*)eeprom_offsets::TOOL_COUNT, count);

	// update XY axis offsets to match tool head settings
	SETDEFAULTAXISHOMEPOSITIONS(false);
}

#if EXTRUDERS > 1
// check single / dual tool status
//#ifdef SINGLE_EXTRUDER
//bool isSingleTool() { return true; }
//#else
bool isSingleTool(){
	// MBI tested with == 1 BUT when writing this same value,
        //  they treat a value > 2 as implying 1.  SOOO, a better test
	//  is to consider single anything which is != 2.
	return (getEeprom8(eeprom_offsets::TOOL_COUNT, 1) != 2);
}
//#endif
#else
bool isSingleTool() { return true; }
#endif

bool hasHBP() {
	return (getEeprom8(eeprom_offsets::HBP_PRESENT, 1) == 1);
}

//
void storeToolheadToleranceDefaults(){

	// assume t0 to t1 distance is in specifications (0 steps tolerance error)
	uint32_t offsets[3] = {TOOLHEAD_OFFSET_X, TOOLHEAD_OFFSET_Y, 0};
	eeprom_write_block((uint8_t*)&(offsets[0]),(uint8_t*)(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS), 12 );
}

void getBuildTime(uint16_t *hours, uint8_t *minutes) {
	*hours = eeprom::getEeprom16(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::HOURS,0);
	*minutes = eeprom::getEeprom8(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::MINUTES,0);
}

void setBuildTime(uint16_t hours, uint8_t minutes) {
	eeprom_write_word((uint16_t*)(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::HOURS), hours);
	eeprom_write_byte((uint8_t*)(eeprom_offsets::TOTAL_BUILD_TIME + build_time_offsets::MINUTES), minutes);
}

void updateBuildTime(uint16_t new_hours, uint8_t new_minutes) {
	uint16_t hours;
	uint8_t minutes;
	getBuildTime(&hours, &minutes);

	// new_minutes can be 60 or even a bit more
	// minutes is always in [0, 59]
	// so total_minutes is always in the range [0, ~119] but conceivably could run up into 120 ballpark...

	uint8_t total_minutes = new_minutes + minutes;
	minutes = total_minutes % 60;

	// increment hours if minutes are over 60
	if ( total_minutes > 60 )
	    hours += (uint16_t)(total_minutes / 60);

	// update build time
	setBuildTime(hours + new_hours, minutes);
}

enum BOTSTEP_TYPE {
    BOTSTEP_16_STEP = 1,
    BOTSTEP_8_STEP = 2,
};

/// Initialize entire eeprom map, including factor-set settings
void fullResetEEPROM() {

	// axis inversion settings
#if !defined(PLATFORM_AXIS_INVERT)
	uint8_t axis_invert = 0b10111; // invert XYBZ
#else
	uint8_t axis_invert = PLATFORM_AXIS_INVERT;
#endif
	eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_INVERSION, axis_invert);

	// tool count settings
	setToolHeadCount(0);

	// toolhead offset defaults
	storeToolheadToleranceDefaults();

	// set build time to zero
	setBuildTime((uint16_t)0, (uint8_t)0);

	eeprom_write_byte((uint8_t*)(eeprom_offsets::BOTSTEP_TYPE), BOTSTEP_16_STEP);

	// filament lifetime counter
	setEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0);
	setEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t), 0);

	FACTORYRESETEEPROM(true);
}

#ifdef HAS_RGB_LED

bool heatLights() {
     return
	  ( LED_DEFAULT_OFF != eeprom::getEeprom8( // LEDs enabled
	       eeprom_offsets::LED_STRIP_SETTINGS, LED_DEFAULT_OFF) ) &&
	  ( LED_DEFAULT_OFF != eeprom::getEeprom8( // Heat progress enabled
		   eeprom_offsets::LED_STRIP_SETTINGS +
		   blink_eeprom_offsets::LED_HEAT_OFFSET,
		   LED_DEFAULT_HEAT_COLOR) );
}

#endif

}
