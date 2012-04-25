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
	setSetpoint(eeprom::getEeprom8(offset, DEFAULT_COOLING_FAN_SETPOINT_C));

	Fan_Pin.setValue(false);
	Fan_Pin.setDirection(true);

	offset = eeprom_base + cooler_eeprom_offsets::ENABLE_OFFSET;
	if (eeprom::getEeprom8(offset ,DEFAULT_COOLING_FAN_ENABLE) == FAN_ENABLED) {
		enable();
	}
	else {
		disable();
	}
}

void CoolingFan::setSetpoint(int temperature) {
	setPoint = temperature;
}

void CoolingFan::enable() {
	enabled = true;
}

void CoolingFan::disable() {
	enabled = false;
	disableFan();
}

void CoolingFan::manageCoolingFan() {
	// TODO: only change the state if necessary
	if (enabled) {
		int temp = heater.get_current_temperature();
		if ((temp > setPoint) && (temp != DEFAULT_THERMOCOUPLE_VAL)){
			enableFan();
		}
		else {
			disableFan();
		}
	}
}

void CoolingFan::enableFan() {
//#ifdef IS_EXTRUDER_BOARD
	Fan_Pin.setValue(true);
//#else
//	#warning cooling fan feature disabled
//#endif
}

void CoolingFan::disableFan() {
//#ifdef IS_EXTRUDER_BOARD
//#warning cooling fan feature disabled
	Fan_Pin.setValue(false);
//#else
//	#warning cooling fan feature disabled
//#endif
}
