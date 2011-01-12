#ifndef __EEPROM_OFFSETS_H__
#define __EEPROM_OFFSETS_H__

#define EEPROM_CHECK_LOW 0x5A
#define EEPROM_CHECK_HIGH 0x78

#define EEPROM_THERM_TABLE_OFFSET 0x100

bool hasEEPROMSettings();

#endif // __EEPROM_OFFSETS_H__
