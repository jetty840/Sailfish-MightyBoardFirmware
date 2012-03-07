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

#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

//for thermistor generation
#include "ThermistorTable.hh"

// for sound definition
#include "Piezo.hh"

// for LED definition
#include "RGB_LED.hh"

// for cooling fan definition
#include "CoolingFan.hh"

namespace eeprom {

/**
 *
 * @param eeprom_base start of eeprom map of cooling settings
 */
void setDefaultCoolingFan(uint16_t eeprom_base){

	uint8_t fan_settings[] = {1, DEFAULT_COOLING_FAN_SETPOINT_C};
    eeprom_write_block( fan_settings, (uint8_t*)(eeprom_base + cooler_eeprom_offsets::ENABLE_OFFSET),2);
}

#define DEFAULT_P_VALUE  7.0f
#define DEFAULT_I_VALUE  0.325f
#define DEFAULT_D_VALUE  36.0f

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
/**
 *
 * @param index
 * @param eeprom_base start of this extruder's data in the eeprom
 */
void setDefaultsExtruder(int index,uint16_t eeprom_base)
{
	uint8_t featuresT0 = eeprom_info::HEATER_0_PRESENT | eeprom_info::HEATER_0_THERMISTOR | eeprom_info::HEATER_0_THERMOCOUPLE;
	uint8_t featuresT1 = eeprom_info::HEATER_1_PRESENT | eeprom_info::HEATER_1_THERMISTOR | eeprom_info::HEATER_1_THERMOCOUPLE;
	if( index == 0 ){
		int slaveId = '12';
	    eeprom_write_byte( (uint8_t*)(eeprom_base + toolhead_eeprom_offsets::FEATURES),featuresT0);
		eeprom_write_byte( (uint8_t*)eeprom_base +toolhead_eeprom_offsets::SLAVE_ID,slaveId);
	}
	else{
		int slaveId = '32';
		eeprom_write_byte( (uint8_t*)(eeprom_base + toolhead_eeprom_offsets::FEATURES),featuresT1);
		eeprom_write_byte( (uint8_t*)eeprom_base +toolhead_eeprom_offsets::SLAVE_ID,slaveId);
	}
	setDefaultPID((eeprom_base + toolhead_eeprom_offsets::EXTRUDER_PID_BASE) );
    setDefaultPID((eeprom_base + toolhead_eeprom_offsets::HBP_PID_BASE) );
    setDefaultCoolingFan(eeprom_base + toolhead_eeprom_offsets::COOLING_FAN_SETTINGS);

    eeprom_write_word((uint16_t*)(eeprom_base + toolhead_eeprom_offsets::BACKOFF_FORWARD_TIME),500);
    eeprom_write_word((uint16_t*)(eeprom_base + toolhead_eeprom_offsets::BACKOFF_STOP_TIME),5);
    eeprom_write_word((uint16_t*)(eeprom_base + toolhead_eeprom_offsets::BACKOFF_REVERSE_TIME),500);
    eeprom_write_word((uint16_t*)(eeprom_base + toolhead_eeprom_offsets::BACKOFF_TRIGGER_TIME),300);



}


#define THERM_R0_DEFAULT_VALUE 100000
#define THERM_T0_DEFAULT_VALUE 25
#define THERM_BETA_DEFAULT_VALUE 4067

/**
 * Set thermal table offsets
 * @param eeprom_base
 */
void SetDefaultsThermal(uint16_t eeprom_base)
{
	eeprom_write_dword( (uint32_t*)(eeprom_base + therm_eeprom_offsets::THERM_R0_OFFSET), THERM_R0_DEFAULT_VALUE);
	eeprom_write_dword( (uint32_t*)(eeprom_base + therm_eeprom_offsets::THERM_T0_OFFSET), THERM_T0_DEFAULT_VALUE);
	eeprom_write_dword( (uint32_t*)(eeprom_base + therm_eeprom_offsets::THERM_BETA_OFFSET), THERM_BETA_DEFAULT_VALUE);
	/// write the default thermal table.
	eeprom_write_block( (const uint8_t*)default_therm_table,
			(uint8_t*)(eeprom_base + therm_eeprom_offsets::THERM_DATA_OFFSET), sizeof(uint16_t)*2*NUMTEMPS);

}

typedef struct {
	int8_t red;
	int8_t green;
	int8_t blue;
} Color;
        

/**
 *
 * @param eeprom_base start of Led effects table
 */
void setDefaultLedEffects(uint16_t eeprom_base)
{
	Color colors;
    // default color is white
	eeprom_write_byte((uint8_t*)(eeprom_base + blink_eeprom_offsets::BASIC_COLOR_OFFSET), LED_DEFAULT_WHITE);
	eeprom_write_byte((uint8_t*)(eeprom_base + blink_eeprom_offsets::LED_HEAT_OFFSET), 1);
    
	colors.red=0xFF; colors.green =colors.blue =0x00;
	eeprom_write_block((void*)&colors,(uint8_t*)(eeprom_base + blink_eeprom_offsets::CUSTOM_COLOR_OFFSET),sizeof(colors));
}
    /**
     *
     * @param red value
     * @param green value
     * @param blue value
     */

void setCustomColor(uint8_t red, uint8_t green, uint8_t blue){
	
	Color colors;
	
	eeprom_write_byte((uint8_t*)(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::BASIC_COLOR_OFFSET), LED_DEFAULT_CUSTOM);
	
	colors.red=red; colors.green = green; colors.blue =blue;
	eeprom_write_block((void*)&colors,(uint8_t*)(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::CUSTOM_COLOR_OFFSET),sizeof(colors));
}

    /**
     *
     * @param sound desired
     * @param dest in eeprom
     */   
void eeprom_write_sound(Sound sound, uint16_t dest)
{
	eeprom_write_word((uint16_t*)dest, 	sound.freq);
	eeprom_write_word((uint16_t*)dest + 2, sound.durationMs);
}

/**
 *
 * @param eeprom_base start of buzz effects table
 */
void setDefaultBuzzEffects(uint16_t eeprom_base)
{
	Sound blare = {NOTE_B2, 500};
	eeprom_write_sound(blare,eeprom_base + buzz_eeprom_offsets::BASIC_BUZZ_OFFSET);
}
    
/**
 *
 * @param eeprom_base start of preheat settings table
 */
void setDefaultsPreheat(uint16_t eeprom_base)
{
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_RIGHT_OFFSET), 220);
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_LEFT_OFFSET), 220);
    eeprom_write_word((uint16_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET), 100);
    eeprom_write_byte((uint8_t*)(eeprom_base + preheat_eeprom_offsets::PREHEAT_ON_OFF_OFFSET), (1<<HEAT_MASK_RIGHT) + (1<<HEAT_MASK_PLATFORM));
}    
    

/// Does a factory reset (resets all defaults except home/endstops, axis direction and tool count)
void factoryResetEEPROM() {

	// Default: enstops inverted, Z axis inverted
	uint8_t endstop_invert = 0b10011111; // all endstops inverted

	uint8_t home_direction = 0b11011; // X,Y Max, Z min  (AB max - to never halt on edge in stepper interface)

	uint8_t vRefBase[] = {118,118,40,118,118};  //(AB maxed out)
	uint16_t vidPid[] = {0x23C1, 0xB404};		/// PID/VID for the MightyBoard!

	/// Write 'MainBoard' settings
	eeprom_write_block("The Replicator",(uint8_t*)eeprom_offsets::MACHINE_NAME,20); // name is null
    eeprom_write_block(&(vRefBase[0]),(uint8_t*)(eeprom_offsets::DIGI_POT_SETTINGS), 5 );
    eeprom_write_byte((uint8_t*)eeprom_offsets::ENDSTOP_INVERSION, endstop_invert);
    eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_HOME_DIRECTION, home_direction);
    
    uint32_t homes[5] = {replicator_axis_offsets::DUAL_X_OFFSET,replicator_axis_offsets::Y_OFFSET,0,0,0};
    /// set axis offsets depending on number of tool heads
    if(getEeprom8(eeprom_offsets::TOOL_COUNT, 1))
		homes[0] = replicator_axis_offsets::SINGLE_X_OFFSET;
	eeprom_write_block((uint8_t*)&(homes[0]),(uint8_t*)(eeprom_offsets::AXIS_HOME_POSITIONS), 20 );
	
	
	
	eeprom_write_byte((uint8_t*)eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
	

    /// Thermal table settings
    SetDefaultsThermal(eeprom_offsets::THERM_TABLE);
    
    /// Preheat heater settings
    setDefaultsPreheat(eeprom_offsets::PREHEAT_SETTINGS);

    /// write MightyBoard VID/PID. Only after verification does production write
    /// a proper 'The Replicator' PID/VID to eeprom, and to the USB chip
    eeprom_write_block(&(vidPid[0]),(uint8_t*)eeprom_offsets::VID_PID_INFO,4);

    /// Write 'extruder 0' settings
    setDefaultsExtruder(0,eeprom_offsets::T0_DATA_BASE);

    /// Write 'extruder 1' stttings
    setDefaultsExtruder(1,eeprom_offsets::T1_DATA_BASE);

    /// write blink and buzz defaults
    setDefaultLedEffects(eeprom_offsets::LED_STRIP_SETTINGS);
    setDefaultBuzzEffects(eeprom_offsets::BUZZ_SETTINGS);
    
    // startup script flag is cleared
    eeprom_write_byte((uint8_t*)eeprom_offsets::FIRST_BOOT_FLAG, 0);
}

void setToolHeadCount(uint8_t count){
	
	// update toolhead count
	if(count > 2)
		count = 1;
	eeprom_write_byte((uint8_t*)eeprom_offsets::TOOL_COUNT, count);
	
	// update XY axis offsets to match tool head settins
	uint32_t homes[5] = {replicator_axis_offsets::DUAL_X_OFFSET,replicator_axis_offsets::Y_OFFSET,0,0,0};
	if(count == 1)
		homes[0] = replicator_axis_offsets::SINGLE_X_OFFSET;
	eeprom_write_block((uint8_t*)&(homes[0]),(uint8_t*)(eeprom_offsets::AXIS_HOME_POSITIONS), 20 );
	
}

    // check single / dual tool status
bool isSingleTool(){
	
	if (getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 1)
		return true;
	else
		return false;
}

    // reset the settings that can be changed via the onboard UI to defaults
void setDefaultSettings(){
    
    /// write blink and buzz defaults
    setDefaultLedEffects(eeprom_offsets::LED_STRIP_SETTINGS);
    setDefaultBuzzEffects(eeprom_offsets::BUZZ_SETTINGS);
    setDefaultsPreheat(eeprom_offsets::PREHEAT_SETTINGS);
    eeprom_write_byte((uint8_t*)eeprom_offsets::FILAMENT_HELP_SETTINGS, 1);
}

void storeToolheadToleranceDefaults(){
	
	// assume t0 to t1 distance is in specifications (0 steps tolerance error)
	uint32_t offsets[3] = {0,0,0};
	eeprom_write_block((uint8_t*)&(offsets[0]),(uint8_t*)(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS), 12 );
	
}

/// Initialize entire eeprom map, including factor-set settings
void fullResetEEPROM() {
	
	// axis inversion settings
	uint8_t axis_invert = 0b011<<2; // A,Z axis = 1
	eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_INVERSION, axis_invert);
	
	// tool count settings
	eeprom_write_byte((uint8_t*)eeprom_offsets::TOOL_COUNT, 1);
	
	// toolhead offset defaults
	storeToolheadToleranceDefaults();
	
	factoryResetEEPROM();

}

}
