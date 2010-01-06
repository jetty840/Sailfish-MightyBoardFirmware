#include "ThermistorTable.h"
#ifdef NEW_THERMISTOR
//generated with T0:100 and T:220
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createT...)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4315 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4315
// max adc: 1023
short temptable[NUMTEMPS][2] = {
   {1, 689},
   {54, 232},
   {107, 192},
   {160, 170},
   {213, 155},
   {266, 143},
   {319, 133},
   {372, 124},
   {425, 116},
   {478, 109},
   {531, 102},
   {584, 95},
   {637, 89},
   {690, 82},
   {743, 75},
   {796, 67},
   {849, 58},
   {902, 48},
   {955, 34},
   {1008, 4}

}; 
#else
//
// Start of temperature lookup table
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
short temptable[NUMTEMPS][2] = {
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
//
// End of temperature lookup table
//
#endif
