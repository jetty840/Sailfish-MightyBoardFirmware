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


// TODO: Clean this up...
#if defined HAS_THERMISTOR_TABLES
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4066 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4066
// max adc: 1023

#ifdef MODEL_REPLICATOR

const TempTable default_therm_table PROGMEM = {
  {1, 841},
  {54, 255},
  {107, 209},
  {160, 184},
  {213, 166},
  {266, 153},
  {319, 142},
  {372, 132},
  {425, 124},
  {478, 116},
  {531, 108},
  {584, 101},
  {637, 93},
  {690, 86},
  {743, 78},
  {796, 70},
  {849, 61},
  {902, 50},
  {955, 34},
  {1008, 3}
};

#define THERMISTOR_TABLE_NUM_TEMPS 20

#else // MODEL_REPLICATOR2

const TempTable default_therm_table PROGMEM = {
  {1, 916},
   {54, 265},
   {107, 216},
   {160, 189},
   {213, 171},
   {266, 157},
   {319, 135}, // Temps above 135 will be invalid
   {372, 127},
   {425, 119},
   {478, 112},
   {531, 104},
   {584, 98},
   {637, 91},
   {690, 84},
   {743, 77},
   {796, 68},
   {849, 58},
   {902, 48},
   {955, 34},
   {1008, 2}
};

#define THERMISTOR_TABLE_NUM_TEMPS 20

#endif

// Convert from scaled mV to Celsius (32767 adc-counts/256 mV)

const static Entry thermocouple_lookup[] PROGMEM = {
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

#define THERMOCOUPLE_NUM_TEMPS 25

#ifdef COLDJUNCTION

// Convert cold junction temps to millivolts * 32767 adc-counts / 256 mV
// This is just the inverse table of thermocouple_table[]

const static Entry coldjunction_lookup[] PROGMEM = {
// {-64, -304},
// {-48, -232},
{-32, -157},
{-16,  -79},
{  0,    0},
{ 16,   81},
{ 32,  164},
{ 48,  248},
{ 64,  333},
{ 80,  418},
{ 96,  503},
{112,  587},
{128,  671},
{144,  754},
#if 0
{160,  837},
{176,  919},
{192, 1000},
{208, 1082},
{224, 1164},
{240, 1247},
{256, 1330},
{272, 1414},
{288, 1499},
{304, 1583},
{336, 1754},
{352, 1840},
{368, 1926},
{384, 2012},
{400, 2098}
#endif
};

#define COLDJUNCTION_NUM_TEMPS 12

#else

#define COLDJUNCTION_NUM_TEMPS 1

#endif

namespace TemperatureTable {

int8_t num_temps[3] = { THERMISTOR_TABLE_NUM_TEMPS - 1,
			THERMOCOUPLE_NUM_TEMPS - 1,
			COLDJUNCTION_NUM_TEMPS - 1 };

/// get value from lookup tables stored in progmem
/// 
/// @param[in] entryIdx table entry offset to read
/// @param[in] table_id  which table to read (valid values defined by therm_table struct)
/// @return  table Entry, a pair of the format (adc_read, temperature) 
inline void getEntry(Entry *rv, int8_t entryIdx, int8_t table_id) {
    if ( table_id == table_thermocouple )
	memcpy_PF(rv, (uint_farptr_t)&(thermocouple_lookup[entryIdx]), sizeof(Entry));
#if COLDJUNCTION
    else if ( table_id == table_coldjunction )
	memcpy_PF(rv, (uint_farptr_t)&(coldjunction_lookup[entryIdx]), sizeof(Entry));
#endif
    else
	memcpy_PF(rv, (uint_farptr_t)&(default_therm_table[entryIdx]), sizeof(Entry));
}

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @param[in] max_allowed_value default temperature if reading is outside of lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, int8_t table_idx, float max_allowed_value) {
  int8_t bottom = 0;
  int8_t numtemps = num_temps[table_idx];
  int8_t top = numtemps;
  int8_t mid = (bottom+top)/2;
  Entry e;
  while (mid > bottom) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
          getEntry(&e,mid,table_idx);
	  if (reading < e.adc) {
		  top = mid;
		  mid = (bottom+top)/2;
	  } else {
		  bottom = mid;
		  mid = (bottom+top)/2;
	  }
  }
  Entry eb, et;
  getEntry(&eb,bottom,table_idx);
  getEntry(&et,top,table_idx);
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
  return (float)eb.value + (float)((reading - eb.adc) * (et.value - eb.value)) / (float)(et.adc - eb.adc);
}

}

#endif
