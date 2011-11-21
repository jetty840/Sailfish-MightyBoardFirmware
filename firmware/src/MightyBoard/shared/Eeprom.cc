#include "Eeprom.hh"
#include "EepromMap.hh"

#include "Version.hh"
#include <avr/eeprom.h>

namespace eeprom {

void init() {
        uint8_t version[2];
        eeprom_read_block(version,(const uint8_t*)eeprom::VERSION_LOW,2);
        if ((version[1]*100+version[0]) == firmware_version) return;
        if (version[1] == 0xff || version[1] < 2) {
            setDefaults();
        }
        // Write version
        version[0] = firmware_version % 100;
        version[1] = firmware_version / 100;
        eeprom_write_block(version,(uint8_t*)eeprom::VERSION_LOW,2);
}

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value) {
        uint8_t data;
        eeprom_read_block(&data,(const uint8_t*)location,1);
        if (data == 0xff) data = default_value;
        return data;
}

uint16_t getEeprom16(const uint16_t location, const uint16_t default_value) {
        uint16_t data;
        eeprom_read_block(&data,(const uint8_t*)location,2);
        if (data == 0xffff) data = default_value;
        return data;
}

float getEepromFixed16(const uint16_t location, const float default_value) {
        uint8_t data[2];
        eeprom_read_block(data,(const uint8_t*)location,2);
        if (data[0] == 0xff && data[1] == 0xff) return default_value;
        return ((float)data[0]) + ((float)data[1])/256.0;
}

} // namespace eeprom
