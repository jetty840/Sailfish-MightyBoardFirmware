#include "ThermistorTable.hh"
#include "EepromMap.hh"
#include <avr/eeprom.h>
#include <stdint.h>
#include <avr/pgmspace.h>

const static int NUMTEMPS = 20;

// Default thermistor table.  If no thermistor table is loaded into eeprom,
// this will be copied in by the initTable() method.
//
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4066 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4066
// max adc: 1023
typedef int16_t TempTable[NUMTEMPS][2];

TempTable default_table PROGMEM = {
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

TempTable thermistor_tables[2];

int16_t thermistorToCelsius(int16_t reading, int8_t table_idx) {
  int16_t celsius = 0;
  int8_t i;
  const TempTable& table = thermistor_tables[table_idx];
  for (i=1; i<NUMTEMPS; i++)
  {
    if (table[i][0] > reading)
    {
      celsius  = table[i-1][1] +
        (reading - table[i-1][0]) *
        (table[i][1] - table[i-1][1]) /
        (table[i][0] - table[i-1][0]);

      if (celsius > 255)
        celsius = 255; 

      break;
    }
  }
  // Overflow: We just clamp to 255 degrees celsius to ensure
  // that the heater gets shut down if something goes wrong.
  if (i == NUMTEMPS) {
    celsius = 255;
  }
  return celsius;
}

bool isTableSet(const void* offset) {
	uint8_t first_byte;
	eeprom_read_block(&first_byte,offset,1);
	return first_byte != 0xff;
}

void initThermTable(TempTable& table, uint16_t offset) {
	// Check for valid table in eeprom.
	void* dest = (void*)&table;
	if (isTableSet((const void*)offset)) {
		eeprom_read_block(dest,(const void*)offset,sizeof(table));
	} else {
		memcpy_P(dest, (const void*)&(default_table[0][0]), sizeof(table));
	}
}

void initThermistorTables() {
	initThermTable(thermistor_tables[0],eeprom::THERM_TABLE_0_DATA);
	initThermTable(thermistor_tables[1],eeprom::THERM_TABLE_1_DATA);
}
