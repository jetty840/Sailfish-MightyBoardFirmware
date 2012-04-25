#ifndef EEPROM_HH
#define EEPROM_HH

#include <stdint.h>

namespace eeprom {

void init();

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value);
uint16_t getEeprom16(const uint16_t location, const uint16_t default_value);
uint32_t getEeprom32(const uint16_t location, const uint32_t default_value);
float getEepromFixed16(const uint16_t location, const float default_value);
void setEepromFixed16(const uint16_t location, const float new_value);
float getEepromFixed32(const uint16_t location, const float default_value);
}

#endif // EEPROM_HH
