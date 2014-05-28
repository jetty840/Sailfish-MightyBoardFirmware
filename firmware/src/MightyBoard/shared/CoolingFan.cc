#include "CoolingFan.hh"
//#include "ExtruderMotor.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#ifdef IS_EXTRUDER_BOARD
#include "ExtruderBoard.hh"
#endif

#define FAN_ENABLED 1
#define FAN_DISABLED 0



// TODO: Come up with a unified strategy for these.
// EEPROM map


CoolingFan::CoolingFan(Heater& heater_in, uint16_t eeprom_base_in, const Pin &fan) :
        heater(heater_in),
        eeprom_base(eeprom_base_in),
        Fan_Pin(fan)
{
	reset();
}

void CoolingFan::reset() {
	uint16_t offset = eeprom_base + cooler_eeprom_offsets::SETPOINT_C_OFFSET;
	setPoint = (uint8_t)eeprom::getEeprom8(offset, DEFAULT_COOLING_FAN_SETPOINT_C);

	Fan_Pin.setValue(false);
	Fan_Pin.setDirection(true);

	offset = eeprom_base + cooler_eeprom_offsets::ENABLE_OFFSET;
	enable(eeprom::getEeprom8(offset ,DEFAULT_COOLING_FAN_ENABLE) == FAN_ENABLED);
}

void CoolingFan::enable(uint8_t enable) {
	enabled = enable;
	if ( !enable ) enableFan(false);
}

void CoolingFan::manageCoolingFan() {
	// TODO: only change the state if necessary
	if (enabled) {
		int16_t temp = heater.get_current_temperature();
		if ( (temp <= (int16_t)(setPoint - 1)) || (temp == DEFAULT_THERMOCOUPLE_VAL) )
		     enableFan(false);
		else if (temp >= (int16_t)setPoint)
		     enableFan(true);
	}
}

void CoolingFan::enableFan(bool enable) {
	Fan_Pin.setValue(enable);
}

