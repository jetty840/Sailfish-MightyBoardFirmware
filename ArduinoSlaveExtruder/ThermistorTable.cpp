#include "ThermistorTable.h"
#include <EEPROM.h>
#include "EEPROMOffsets.h"

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
int16_t temptable[NUMTEMPS][2] = {
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

int16_t thermistorToCelsius(int16_t reading) {
  int16_t celsius = 0;
  int8_t i;
  for (i=1; i<NUMTEMPS; i++)
  {
    if (temptable[i][0] > reading)
    {
      celsius  = temptable[i-1][1] + 
        (reading - temptable[i-1][0]) * 
        (temptable[i][1] - temptable[i-1][1]) /
        (temptable[i][0] - temptable[i-1][0]);

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

bool hasEEPROMSettings();

void initThermTable() {
  // Check for valid table in eeprom.
  if (hasEEPROMSettings()) {
    uint8_t* dest = (uint8_t*)temptable;
    for (int8_t i = 1; i < NUMTEMPS*sizeof(int16_t)*2; i++) {
      *(dest+i) = EEPROM.read(EEPROM_THERM_TABLE_OFFSET+i);
    }
  }
}
