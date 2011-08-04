#include "CoolingFan.hh"
//#include "ExtruderMotor.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"

#define FAN_ENABLED 1
#define FAN_DISABLED 0

#define DEFAULT_COOLING_FAN_SETPOINT_C  50
#define DEFAULT_COOLING_FAN_ENABLE      FAN_DISABLED

// TODO: Come up with a unified strategy for these.
// EEPROM map
#define ENABLE_OFFSET       0
#define SETPOINT_C_OFFSET   1


CoolingFan::CoolingFan(Heater heater, uint16_t eeprom_base_in) :
        heater(heater),
        eeprom_base(eeprom_base_in)
{
	reset();
}

void CoolingFan::reset() {
        setSetpoint(eeprom::getEeprom16(eeprom_base + SETPOINT_C_OFFSET,
                                        DEFAULT_COOLING_FAN_SETPOINT_C));

        if (eeprom::getEeprom8(eeprom_base + ENABLE_OFFSET,
                               DEFAULT_COOLING_FAN_ENABLE) == FAN_ENABLED) {
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
		if (heater.get_current_temperature() > setPoint) {
			enableFan();
		}
		else {
			disableFan();
		}
	}
}

void CoolingFan::enableFan() {
#warning cooling fan feature disabled
    //	setExtruderMotor(255);
}

void CoolingFan::disableFan() {
#warning cooling fan feature disabled
//	setExtruderMotor(0);
}
