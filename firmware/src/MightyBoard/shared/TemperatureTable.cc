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

#else

// Epcos 100K
const static Entry table_ext_thermistor[] PROGMEM = {
     {  23 * TEMP_OVERSAMPLE, 300 },
     {  25 * TEMP_OVERSAMPLE, 295 },
     {  27 * TEMP_OVERSAMPLE, 290 },
     {  28 * TEMP_OVERSAMPLE, 285 },
     {  31 * TEMP_OVERSAMPLE, 280 },
     {  33 * TEMP_OVERSAMPLE, 275 },
     {  35 * TEMP_OVERSAMPLE, 270 },
     {  38 * TEMP_OVERSAMPLE, 265 },
     {  41 * TEMP_OVERSAMPLE, 260 },
     {  44 * TEMP_OVERSAMPLE, 255 },
     {  48 * TEMP_OVERSAMPLE, 250 },
     {  52 * TEMP_OVERSAMPLE, 245 },
     {  56 * TEMP_OVERSAMPLE, 240 },
     {  61 * TEMP_OVERSAMPLE, 235 },
     {  66 * TEMP_OVERSAMPLE, 230 },
     {  71 * TEMP_OVERSAMPLE, 225 },
     {  78 * TEMP_OVERSAMPLE, 220 },
     {  84 * TEMP_OVERSAMPLE, 215 },
     {  92 * TEMP_OVERSAMPLE, 210 },
     { 100 * TEMP_OVERSAMPLE, 205 },
     { 109 * TEMP_OVERSAMPLE, 200 },
     { 120 * TEMP_OVERSAMPLE, 195 },
     { 131 * TEMP_OVERSAMPLE, 190 },
     { 143 * TEMP_OVERSAMPLE, 185 },
     { 156 * TEMP_OVERSAMPLE, 180 },
     { 171 * TEMP_OVERSAMPLE, 175 },
     { 187 * TEMP_OVERSAMPLE, 170 },
     { 205 * TEMP_OVERSAMPLE, 165 },
     { 224 * TEMP_OVERSAMPLE, 160 },
     { 245 * TEMP_OVERSAMPLE, 155 },
     { 268 * TEMP_OVERSAMPLE, 150 },
     { 293 * TEMP_OVERSAMPLE, 145 },
     { 320 * TEMP_OVERSAMPLE, 140 },
     { 348 * TEMP_OVERSAMPLE, 135 },
     { 379 * TEMP_OVERSAMPLE, 130 },
     { 411 * TEMP_OVERSAMPLE, 125 },
     { 445 * TEMP_OVERSAMPLE, 120 },
     { 480 * TEMP_OVERSAMPLE, 115 },
     { 516 * TEMP_OVERSAMPLE, 110 },
     { 553 * TEMP_OVERSAMPLE, 105 },
     { 591 * TEMP_OVERSAMPLE, 100 },
     { 628 * TEMP_OVERSAMPLE,  95 },
     { 665 * TEMP_OVERSAMPLE,  90 },
     { 702 * TEMP_OVERSAMPLE,  85 },
     { 737 * TEMP_OVERSAMPLE,  80 },
     { 770 * TEMP_OVERSAMPLE,  75 },
     { 801 * TEMP_OVERSAMPLE,  70 },
     { 830 * TEMP_OVERSAMPLE,  65 },
     { 857 * TEMP_OVERSAMPLE,  60 },
     { 881 * TEMP_OVERSAMPLE,  55 },
     { 903 * TEMP_OVERSAMPLE,  50 },
     { 922 * TEMP_OVERSAMPLE,  45 },
     { 939 * TEMP_OVERSAMPLE,  40 },
     { 954 * TEMP_OVERSAMPLE,  35 },
     { 966 * TEMP_OVERSAMPLE,  30 },
     { 977 * TEMP_OVERSAMPLE,  25 },
     { 985 * TEMP_OVERSAMPLE,  20 },
     { 993 * TEMP_OVERSAMPLE,  15 },
     { 999 * TEMP_OVERSAMPLE,  10 },
     {1004 * TEMP_OVERSAMPLE,   5 },
     {1008 * TEMP_OVERSAMPLE,   0 }
};

#define EXT_THERMISTOR_NUM_TEMPS 61

// For the time being, assume Epcos 100K for the heater bed as well

#define table_hbp_thermistor table_ext_thermistor
#define HBP_THERMISTOR_NUM_TEMPS EXT_THERMISTOR_NUM_TEMPS

#endif

// Convert from scaled mV to Celsius (32767 adc-counts/256 mV)

const static Entry table_thermocouple_k[] PROGMEM = {
     {-304, -64},
     {-232, -48},
     {-157, -32},
     {-79, -16},
     {0, 0},
     {82, 16},
     {164, 32},
     {248, 48},
     {333, 64},
     {418, 80},
     {503, 96},
     {588, 112},
     {672, 128},
     {755, 144},
     {837, 160},
     {919, 176},
     {1001, 192},
     {1083, 208},
     {1165, 224},
     {1248, 240},
     {1331, 256},
     {1415, 272},
     {1499, 288},
     {1584, 304},
     {1754, 336},
#if 0 // cut table off at 336C
     {1840, 352},
     {1926, 368},
     {2012, 384},
     {2099, 400},
//#define THERMOCOUPLE_NUM_TEMPS 29
#endif

};

#define THERMOCOUPLE_K_NUM_TEMPS 25

static uint8_t num_temps[] = {
     THERMOCOUPLE_K_NUM_TEMPS - 1,
     HBP_THERMISTOR_NUM_TEMPS - 1,
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     EXT_THERMISTOR_NUM_TEMPS - 1
#endif
};

namespace TemperatureTable {

/// get value from lookup tables stored in progmem
///
/// @param[in] entryIdx table entry offset to read
/// @param[in] table_id  which table to read (valid values defined by therm_table struct)
/// @return  table Entry, a pair of the format (adc_read, temperature)
inline void getEntry(Entry *rv, uint8_t entryIdx, uint8_t table_id) {
     if ( table_id == TABLE_THERMOCOUPLE_K )
	  memcpy_PF(rv, (uint_farptr_t)&(table_thermocouple_k[entryIdx]), sizeof(Entry));
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
     else if ( table_id == TABLE_EXT_THERMISTOR )
	  memcpy_PF(rv, (uint_farptr_t)&(table_ext_thermistor[entryIdx]), sizeof(Entry));
#endif
     else
	  memcpy_PF(rv, (uint_farptr_t)&(table_hbp_thermistor[entryIdx]), sizeof(Entry));
}

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @param[in] max_allowed_value default temperature if reading is outside of lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, uint8_t table_idx, float max_allowed_value) {
     uint8_t bottom = 0;
     uint8_t numtemps = num_temps[table_idx];
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
