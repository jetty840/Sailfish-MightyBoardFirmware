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

#include "Compat.hh"
#include "TemperatureTable.hh"
#include "Configuration.hh"
#include "EepromMap.hh"
#include <avr/eeprom.h>
#include <stdint.h>
#include <avr/pgmspace.h>

typedef struct {
     int16_t adc;
     int16_t value;
} Entry;

// TODO: Clean this up...

// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4066 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4066
// max adc: 1023

#if (BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_E)

const static Entry table_hbp_thermistor[] PROGMEM = {
     {   1, 841},
     {  54, 255},
     { 107, 209},
     { 160, 184},
     { 213, 166},
     { 266, 153},
     { 319, 142},
     { 372, 132},
     { 425, 124},
     { 478, 116},
     { 531, 108},
     { 584, 101},
     { 637,  93},
     { 690,  86},
     { 743,  78},
     { 796,  70},
     { 849,  61},
     { 902,  50},
     { 955,  34},
     {1008,   3},
     {1023,   0}
};

#define HBP_THERMISTOR_NUM_TEMPS 21

#elif BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G

// Convert from scaled mV to Celsius (32767 adc-counts/256 mV)

// **** Can reduce to an empty table for Azteeg

const static Entry table_thermocouple_k[] PROGMEM = {
     { -304, -64},
     { -232, -48},
     { -157, -32},
     {  -79, -16},
     {    0,   0},
     {   82,  16},
     {  164,  32},
     {  248,  48},
     {  333,  64},
     {  418,  80},
     {  503,  96},
     {  588, 112},
     {  672, 128},
     {  755, 144},
     {  837, 160},
     {  919, 176},
     { 1001, 192},
     { 1083, 208},
     { 1165, 224},
     { 1248, 240},
     { 1331, 256},
     { 1415, 272},
     { 1499, 288},
     { 1584, 304},
     { 1754, 336},
#if 0 // cut table off at 336C
     { 1840, 352},
     { 1926, 368},
     { 2012, 384},
     { 2099, 400},
#endif
};

#define THERMOCOUPLE_K_NUM_TEMPS 25

const static Entry table_hbp_thermistor[] PROGMEM = {
     {   1, 916},
     {  54, 265},
     { 107, 216},
     { 160, 189},
     { 213, 171},
     { 266, 157},
     { 319, 135}, // Temps above 135 will be invalid
     { 372, 127},
     { 425, 119},
     { 478, 112},
     { 531, 104},
     { 584,  98},
     { 637,  91},
     { 690,  84},
     { 743,  77},
     { 796,  68},
     { 849,  58},
     { 902,  48},
     { 955,  34},
     {1008,   2},
     {1023,   0}
};

#define HBP_THERMISTOR_NUM_TEMPS 21

#elif BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

#ifdef MY_TABLE
#undef MY_TABLE
#endif

#ifdef MY_NAME
#undef MY_NAME
#endif

#ifdef MY_INDEX
#undef MY_INDEX
#endif

#ifndef THERM_INDEX_EXT
#define THERM_INDEX_EXT 2
#endif

#define MY_INDEX THERM_INDEX_EXT
#define MY_TABLE table_ext_thermistor
#define MY_NAME  table_ext_name

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#include "ThermistorTables.h"

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifndef THERM_INDEX_HBP
#define THERM_INDEX_HBP 1
#endif

#define MY_INDEX THERM_INDEX_HBP
#define MY_TABLE table_hbp_thermistor
#define MY_NAME  table_hbp_name

#include "ThermistorTables.h"

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifndef THERM_INDEX_3
#define THERM_INDEX_3 3
#endif

#define MY_INDEX THERM_INDEX_3
#define MY_TABLE table_3_thermistor
#define MY_NAME  table_3_name

#include "ThermistorTables.h"

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifndef THERM_INDEX_4
#define THERM_INDEX_4 4
#endif

#define MY_INDEX THERM_INDEX_4
#define MY_TABLE table_4_thermistor
#define MY_NAME  table_4_name

#include "ThermistorTables.h"

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifndef THERM_INDEX_5
#define THERM_INDEX_5 5
#endif

#define MY_INDEX THERM_INDEX_5
#define MY_TABLE table_5_thermistor
#define MY_NAME  table_5_name

#include "ThermistorTables.h"

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

// Magic Epcos 100K is #6
// If you re-arrange, you may want to change THERM_INDEX_EPCOS in .hh

#ifndef THERM_INDEX_6
#define THERM_INDEX_6 6
#endif

#define MY_INDEX THERM_INDEX_6
#define MY_TABLE table_6_thermistor
#define MY_NAME  table_6_name

#include "ThermistorTables.h"

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifdef THERMISTOR_TABLES_H_
#undef THERMISTOR_TABLES_H_
#endif

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#ifndef THERM_INDEX_7
#define THERM_INDEX_7 7
#endif

#define MY_INDEX THERM_INDEX_7
#define MY_TABLE table_7_thermistor
#define MY_NAME  table_7_name

#include "ThermistorTables.h"

#undef MY_INDEX
#undef MY_TABLE
#undef MY_NAME

#define EXT_THERMISTOR_NUM_TEMPS  TEMP_TABLE_SIZE(table_ext_thermistor)
#define HBP_THERMISTOR_NUM_TEMPS  TEMP_TABLE_SIZE(table_hbp_thermistor)
#define THERMISTOR_NUM_TEMPS_3    TEMP_TABLE_SIZE(table_3_thermistor)
#define THERMISTOR_NUM_TEMPS_4    TEMP_TABLE_SIZE(table_4_thermistor)
#define THERMISTOR_NUM_TEMPS_5    TEMP_TABLE_SIZE(table_5_thermistor)
#define THERMISTOR_NUM_TEMPS_6    TEMP_TABLE_SIZE(table_6_thermistor)
#define THERMISTOR_NUM_TEMPS_7    TEMP_TABLE_SIZE(table_7_thermistor)

#else

#error Unknown board type

#endif

static uint8_t num_temps[] = {
#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G
     THERMOCOUPLE_K_NUM_TEMPS - 1,
#endif
     HBP_THERMISTOR_NUM_TEMPS - 1,
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     EXT_THERMISTOR_NUM_TEMPS - 1,
	 THERMISTOR_NUM_TEMPS_3 - 1,
	 THERMISTOR_NUM_TEMPS_4 - 1,
	 THERMISTOR_NUM_TEMPS_5 - 1,
	 THERMISTOR_NUM_TEMPS_6 - 1,
	 THERMISTOR_NUM_TEMPS_7 - 1
#endif
};

namespace TemperatureTable {

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

const static PROGMEM prog_uchar THERM_NAME_0[] = "0. K Thermocouple";

const prog_uchar *getThermistorName(uint8_t idx)
{
	if (idx == TABLE_THERMOCOUPLE_K) return(THERM_NAME_0);
	else if (idx == TABLE_EXT_THERMISTOR) return(table_ext_name);
	else if (idx == TABLE_HBP_THERMISTOR) return(table_hbp_name);
	else if (idx == TABLE_3_THERMISTOR) return(table_3_name);
	else if (idx == TABLE_4_THERMISTOR) return(table_4_name);
	else if (idx == TABLE_5_THERMISTOR) return(table_5_name);
	else if (idx == TABLE_6_THERMISTOR) return(table_6_name);
	else return(table_7_name);
}

#endif

/// get value from lookup tables stored in progmem
///
/// @param[in] entryIdx table entry offset to read
/// @param[in] table_id  which table to read (valid values defined by therm_table struct)
/// @return  table Entry, a pair of the format (adc_read, temperature)
inline void getEntry(Entry *rv, uint8_t entryIdx, uint8_t table_id) {

#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G
	// Rev G, H boards need to do thermocouple and HBP lookups
	//  (cold junction for ADS1118 is computed directly
	if ( table_id == TABLE_THERMOCOUPLE_K )
		memcpy_PF(rv, (uint_farptr_t)&(table_thermocouple_k[entryIdx]), sizeof(Entry));
	else
		memcpy_PF(rv, (uint_farptr_t)&(table_hbp_thermistor[entryIdx]), sizeof(Entry));
#elif BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
	// Azteeg computes thermocouple directly and does lookups for thermistors
	if ( table_id == TABLE_EXT_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_ext_thermistor[entryIdx]), sizeof(Entry));
	else if ( table_id == TABLE_HBP_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_hbp_thermistor[entryIdx]), sizeof(Entry));
	else if ( table_id == TABLE_3_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_3_thermistor[entryIdx]), sizeof(Entry));
	else if ( table_id == TABLE_4_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_4_thermistor[entryIdx]), sizeof(Entry));
	else if ( table_id == TABLE_5_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_5_thermistor[entryIdx]), sizeof(Entry));
	else if ( table_id == TABLE_6_THERMISTOR )
		memcpy_PF(rv, (uint_farptr_t)&(table_6_thermistor[entryIdx]), sizeof(Entry));
	else
		memcpy_PF(rv, (uint_farptr_t)&(table_7_thermistor[entryIdx]), sizeof(Entry));
#else
	// Rev E mightyboards only need to do thermistor lookups
	memcpy_PF(rv, (uint_farptr_t)&(table_hbp_thermistor[entryIdx]), sizeof(Entry));
#endif
}

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @param[in] max_allowed_value default temperature if reading is outside of lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, uint8_t table_idx,
						float max_allowed_value) {
     uint8_t bottom = 0;
#if BOARD_TYPE == BOARD_TYPE_MIGHTYBOARD_G
	 // Tables include thermocouple table which has index 0
     uint8_t numtemps = num_temps[table_idx];
#else
	 // Tables do not include a thermocouple table; subtract 1 from indices
     uint8_t numtemps = num_temps[table_idx-1];
#endif
     uint8_t top = numtemps;
     uint8_t mid = (bottom + top) >> 1;
     Entry e;
     while (mid > bottom) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	  getEntry(&e, mid, table_idx);
	  if (reading < e.adc) {
	       top = mid;
	       mid = (bottom + top) >> 1;
	  } else {
	       bottom = mid;
	       mid = (bottom + top) >> 1;
	  }
     }
     Entry eb, et;
     getEntry(&eb, bottom, table_idx);
     getEntry(&et, top,    table_idx);
#pragma GCC diagnostic pop
     if (bottom == 0 && reading < eb.adc) {
	  // out of scale; safety mode
	  return max_allowed_value;
     }
     if (top == numtemps && reading > et.adc) {
	  // out of scale; safety mode
	  return max_allowed_value;
     }

     // Interpolate
     return (float)eb.value + (float)((reading - eb.adc) * (et.value - eb.value)) /
	  (float)(et.adc - eb.adc);
}

}
