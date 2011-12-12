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

namespace eeprom {

/**
 *
 * @param eeprom_base start of eeprom map of cooling settings
 */
void setDefaultCoolingFan(uint16_t eeprom_base){

#define DEFAULT_COOLING_FAN_SETPOINT_C  50
	float setpoint = DEFAULT_COOLING_FAN_SETPOINT_C;
	setEepromFixed16(setpoint, ( eeprom_base +	cooler_eeprom_offsets::SETPOINT_C_OFFSET ));
    eeprom_write_byte( (uint8_t*)(eeprom_base + cooler_eeprom_offsets::ENABLE_OFFSET),1);
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
void SetDefaulstThermal(uint16_t eeprom_base)
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
void 	setDefaultLedEffects(uint16_t eeprom_base)
{
	Color colors = {0xFF,0xFF,0xFF};
	eeprom_write_block((void*)&colors,(uint8_t*)(eeprom_base + blink_eeprom_offsets::BASIC_COLOR_OFFSET),sizeof(colors));
	colors.red=0xFF; colors.green =colors.blue =0x00;
	eeprom_write_block((void*)&colors,(uint8_t*)(eeprom_base + blink_eeprom_offsets::ERROR_COLOR_OFFSET),sizeof(colors));
	colors.red=0x00;colors.green =colors.blue =0xFF;
	eeprom_write_block((void*)&colors,(uint8_t*)(eeprom_base + blink_eeprom_offsets::DONE_COLOR_OFFSET),sizeof(colors));
}


void eeprom_write_sound(Sound sound, uint16_t dest)
{
	eeprom_write_word((uint16_t*)dest, 	sound.freq);
	eeprom_write_word((uint16_t*)dest + 2, sound.durationMs);
}

/**
 *
 * @param eeprom_base start of buzz effects table
 */
void 	setDefaultBuzzEffects(uint16_t eeprom_base)
{
	Sound blare = {NOTE_B2, 500};
	eeprom_write_sound(blare,eeprom_base + buzz_eeprom_offsets::BASIC_BUZZ_OFFSET);
	blare.freq = NOTE_C8;
	eeprom_write_sound(blare,eeprom_base + buzz_eeprom_offsets::ERROR_BUZZ_OFFSET);
	blare.freq  = NOTE_B0;
	eeprom_write_sound(blare,eeprom_base + buzz_eeprom_offsets::DONE_BUZZ_OFFSET);
}



void setDefaults() {
    // Initialize eeprom map
    // Default: enstops inverted, Z axis inverted

	uint8_t endstop_invert = 0b00011111; // all endstops inverted

	uint8_t axis_invert = 0b111<<2; // A,B,Z axis = 1

	// NOTE: Firmware does not use these, they are legacy
	uint8_t vRefBase[]  = {75,75,75,75,75};  //~ 1.0 volts
	uint32_t endstops[] = {5,10,15,20,25};//test values

	// un-hardcode from HostCommands section of firmware, use this

	/// Write 'MainBoard' settings
	eeprom_write_block("The Beplicator",(uint8_t*)eeprom_offsets::MACHINE_NAME,20); // name is null
    //eeprom_write_block((uint8_t*)(eeprom_offsets::DIGI_POT_SETTINGS), vRefBase, 5 );
    eeprom_write_block(&(vRefBase[0]),(uint8_t*)(eeprom_offsets::DIGI_POT_SETTINGS), 5 );
   //eeprom_write_block((uint8_t*)(eeprom_offsets::AXIS_HOME_POSITIONS), endstops, 20 );
    eeprom_write_block(&(endstops[0]),(uint8_t*)(eeprom_offsets::AXIS_HOME_POSITIONS), 20 );
    eeprom_write_byte((uint8_t*)(eeprom_offsets::HARDWARE_ID),HARDWARE_ID_LMIGHTYBOARD_A);

	eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_INVERSION, axis_invert);
    eeprom_write_byte((uint8_t*)eeprom_offsets::ENDSTOP_INVERSION, endstop_invert);

    /// Thermal table settings
    SetDefaulstThermal(eeprom_offsets::THERM_TABLE);

    /// Write 'extruder 0' settings
    setDefaultsExtruder(0,eeprom_offsets::T0_DATA_BASE);

    /// Write 'extruder 1' stttings
    setDefaultsExtruder(1,eeprom_offsets::T1_DATA_BASE);

    /// write blink and buzz defaults
    setDefaultLedEffects(eeprom_offsets::LED_STRIP_SETTINGS);
    setDefaultBuzzEffects(eeprom_offsets::BUZZ_SETTINGS);
}

}
