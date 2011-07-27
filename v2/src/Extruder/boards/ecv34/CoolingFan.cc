#include "CoolingFan.hh"
#include "ExtruderMotor.hh"
#include "EepromMap.hh"

#define FAN_ENABLED 1
#define FAN_DISABLED 0

#define DEFAULT_COOLING_FAN_SETPOINT_C  50
#define DEFAULT_COOLING_FAN_ENABLE      FAN_DISABLED

CoolingFan::CoolingFan(Heater heater) :
	heater(heater)
{
	reset();
}

void CoolingFan::reset() {
        setSetpoint(eeprom::getEeprom16(eeprom::COOLING_FAN_SETPOINT_C,
                                        DEFAULT_COOLING_FAN_SETPOINT_C));

        if (eeprom::getEeprom8(eeprom::COOLING_FAN_ENABLE,
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
	setExtruderMotor(255);
}

void CoolingFan::disableFan() {
	setExtruderMotor(0);
}
