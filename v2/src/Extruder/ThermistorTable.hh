#ifndef THERMISTOR_TABLE
#define THERMISTOR_TABLE

#include <stdint.h>

int16_t thermistorToCelsius(int16_t reading, int8_t table_idx);

// initThermTable should be called on boot.
void initThermistorTables();

#endif // THERMISTOR_TABLE
