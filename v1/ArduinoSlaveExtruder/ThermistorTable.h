#ifndef THERMISTOR_TABLE
#define THERMISTOR_TABLE

#include <stdint.h>

#define NUMTEMPS 20

int16_t thermistorToCelsius(int16_t reading);

// Load the thermistor table from EEPROM if available.
// initThermTable should be called on boot.
void initThermTable();

#endif // THERMISTOR_TABLE
