#include <util/delay.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "StepperAxis.hh"

#include "Version.hh"
#include <avr/eeprom.h>

#ifdef EEPROM_MENU_ENABLE
	#include <avr/wdt.h>
	#include "SDCard.hh"
#endif

namespace eeprom {

/**
 * if the EEPROM is initalized and matches firmware version, exit
 * if the EEPROM is not initalized, write defaults, and set a new version
 * if the EEPROM is initalized but is not the current version, re-write the version number
 */
void init() {
        uint8_t prom_version[2];
        eeprom_read_block(prom_version,(const uint8_t*)eeprom_offsets::VERSION_LOW,2);
	if ((prom_version[1]*100+prom_version[0]) == firmware_version)
		return;

	// Delay a bit to prevent a reset from avrdude from
	// hitting us while updating the eeprom
	wdt_reset();
	_delay_us(1000000);
	wdt_reset();

        /// if our eeprom is empty (version is still FF, ie unwritten)
        if (prom_version[0] >= 100 || prom_version[1] >= 20 ||
	    prom_version[1] < 6)
        	fullResetEEPROM();

       //Update eeprom version # to match current firmware version
       prom_version[0] = firmware_version % 100;
       prom_version[1] = firmware_version / 100;
       eeprom_write_block(prom_version,(uint8_t*)eeprom_offsets::VERSION_LOW,2);
}

#if defined(ERASE_EEPROM_ON_EVERY_BOOT) || defined(EEPROM_MENU_ENABLE)

#if defined (__AVR_ATmega168__)
        #define EEPROM_SIZE 512
#elif defined (__AVR_ATmega328__)
        #define EEPROM_SIZE 1024
#elif defined (__AVR_ATmega644P__)
        #define EEPROM_SIZE 2048
#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
        #define EEPROM_SIZE 4096
#else
        #define EEPROM_SIZE 0
#endif

//Complete erase of eeprom to 0xFF
void erase() {
        for (uint16_t i = 0; i < EEPROM_SIZE; i ++ ) {
                eeprom_write_byte((uint8_t*)i, 0xFF);
		wdt_reset();
	}
}

#endif

#ifdef EEPROM_MENU_ENABLE

//Saves the eeprom to filename on the sd card
bool saveToSDFile(const char *filename) {
	uint8_t v;

	//Open the file for writing
	if ( sdcard::startCapture((char *)filename) != sdcard::SD_SUCCESS )	return false;

	//Write the eeprom contents to the file
	bool ret = true;
        for (uint16_t i = 0; i < EEPROM_SIZE; i ++ ) {
                v = eeprom_read_byte((uint8_t*)i);
		if ( !sdcard::writeByte(v) ) {
		    ret = false;
		    break;
		}
		wdt_reset();
	}

	sdcard::finishCapture();

	return ret;
}

//Restores eeprom from filename on the sdcard
bool restoreFromSDFile(const char *filename) {
	uint8_t v;

	if ( sdcard::startPlayback((char *)filename) != sdcard::SD_SUCCESS )	return false;

        for (uint16_t i = 0; i < EEPROM_SIZE; i ++ ) {
		if ( sdcard::playbackHasNext() ) {
			v = sdcard::playbackNext();
                	eeprom_write_byte((uint8_t*)i, v);
			wdt_reset();
		}
		else break;
	}

    	sdcard::finishPlayback();

	return true;
}

#endif

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value) {
        uint8_t data = eeprom_read_byte((uint8_t*)location);
        if (data == 0xff) data = default_value;
        return data;
}

uint16_t getEeprom16(const uint16_t location, const uint16_t default_value) {
        uint16_t data = eeprom_read_word((uint16_t*)location);
        if (data == 0xffff) data = default_value;
        return data;
}

uint32_t getEeprom32(const uint16_t location, const uint32_t default_value) {
        uint32_t data = eeprom_read_dword((uint32_t*)location);
        if (data == 0xffffffff) return default_value;
        return data;
}

/* Disabled for now, not used and incorrect
float getEepromFixed32(const uint16_t location, const float default_value) {
        int32_t data = getEeprom32(location, 0xffffffff);
        if (data == 0xffffffff) return default_value;
        return ((float)data)/65536.0;
}
*/


/// Fetch a fixed 16 value from eeprom
float getEepromFixed16(const uint16_t location, const float default_value) {
        uint8_t data[2];
        eeprom_read_block(data,(uint8_t*)location,2);
        if (data[0] == 0xff && data[1] == 0xff) return default_value;
        return ((float)data[0]) + ((float)data[1])/256.0;
}


/// Write a fixed 16 value to eeprom
void setEepromFixed16(const uint16_t location, const float new_value)
{
    uint8_t data[2];
    data[0] = (uint8_t)new_value;
    data[1] = (int)((new_value - data[0])*256.0);
    eeprom_write_block(data,(uint8_t*)location,2);
}


/// Fetch an int64 value from eeprom
int64_t getEepromInt64(const uint16_t location, const int64_t default_value) {
        int64_t *ret;
        uint8_t data[8];
        eeprom_read_block(data,(const uint8_t*)location,8);
        if (data[0] == 0xff && data[1] == 0xff && data[2] == 0xff && data[3] == 0xff &&
            data[4] == 0xff && data[5] == 0xff && data[6] == 0xff && data[7] == 0xff)
                 return default_value;
        ret = (int64_t *)&data[0];
        return *ret;
}


/// Write an int64 value to eeprom
void setEepromInt64(const uint16_t location, const int64_t value) {
        void *data;
        data = (void *)&value;
        eeprom_write_block(data,(void*)location,8);
}

} // namespace eeprom
