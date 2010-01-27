#include "EEPROMOffsets.h"
#include <EEPROM.h>

bool hasEEPROMSettings() {
  return (EEPROM.read(0) == EEPROM_CHECK_LOW &&
	  EEPROM.read(1) == EEPROM_CHECK_HIGH);
}
