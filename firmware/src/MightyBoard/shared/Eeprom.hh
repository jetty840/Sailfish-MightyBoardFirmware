#ifndef EEPROM_HH
#define EEPROM_HH

#include <stdint.h>

#ifndef SIMULATOR
#include <avr/pgmspace.h>
#include "Configuration.hh"
#endif

namespace eeprom {

void init();

#if defined(ERASE_EEPROM_ON_EVERY_BOOT) || defined(EEPROM_MENU_ENABLE)
        void erase();
#endif

#ifdef EEPROM_MENU_ENABLE
	bool saveToSDFile(const prog_char *filename);
	bool restoreFromSDFile(const prog_char *filename);
#endif

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value);
uint16_t getEeprom16(const uint16_t location, const uint16_t default_value);
uint32_t getEeprom32(const uint16_t location, const uint32_t default_value);
float getEepromFixed16(const uint16_t location, const float default_value);
void setEepromFixed16(const uint16_t location, const float new_value);
//float getEepromFixed32(const uint16_t location, const float default_value);	//Disabled for now, not used and incorrect
int64_t getEepromInt64(const uint16_t location, const int64_t default_value);
void setEepromInt64(const uint16_t location, const int64_t value);

}

#endif // EEPROM_HH
