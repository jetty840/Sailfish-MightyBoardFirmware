/*
 * QueryPacketProcessor.cc
 *
 *  Created on: Feb 24, 2010
 *      Author: phooky
 */

#include <util/atomic.h>
#include <avr/eeprom.h>
#include "QueryPacketProcessor.hh"
#include "Commands.hh"
#include "Version.hh"
#include "DebugPin.hh"
#include "Heater.hh"
#include "ExtruderMotor.hh"

inline void handleReadEeprom(const InPacket& from_host, OutPacket& to_host) {
	const uint16_t offset = from_host.read16(2);
	const uint8_t count = from_host.read8(4);
	if (count > 16) {
		to_host.append8(RC_BUFFER_OVERFLOW);
	} else {
		uint8_t data[16];
		eeprom_read_block(data,(const void*)offset, count);
		to_host.append8(RC_OK);
		for (int i = 0; i < count; i++) {
			to_host.append8(data[i]);
		}
	}
}

inline void handleWriteEeprom(const InPacket& from_host, OutPacket& to_host) {
	const uint16_t offset = from_host.read16(2);
	const uint8_t count = from_host.read8(4);
	if (count > 16) {
		to_host.append8(RC_BUFFER_OVERFLOW);
	} else {
		uint8_t data[16];
		eeprom_read_block(data,(const void*)offset, count);
		for (int i = 0; i < count; i++) {
			data[i] = from_host.read8(i+5);
		}
		eeprom_write_block(data,(void*)offset, count);
		to_host.append8(RC_OK);
		to_host.append8(count);
	}
}

class Motor {
private:
	bool direction_;
	int speed_;
	int on_;
	void update() {
		if (on_) {
			int value = direction_?speed_:-speed_;
			setExtruderMotor( value );
		} else {
			setExtruderMotor( 0 );
		}
	}
public:
	Motor() : direction_(true), speed_(0), on_(false) {}
	void setSpeed(int speed) { speed_ = speed; update(); }
	void setDir(bool forward) { direction_ = forward; update(); }
	void setOn(bool on) { on_ = on; update(); }
	void toggleOn() { on_ = !on_; update(); }
};

Motor motor1;

bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(1);
		// All commands are query commands.

		switch (command) {
		case SLAVE_CMD_VERSION:
			to_host.append8(RC_OK);
			to_host.append16(firmware_version);
			return true;
		case SLAVE_CMD_INIT:
			//init();
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_GET_TEMP:
			to_host.append8(RC_OK);
			to_host.append16(extruder_heater.get_current_temperature());
			return true;
		case SLAVE_CMD_SET_TEMP:
			setDebugLED(true);
		    extruder_heater.set_target_temperature(from_host.read16(2));
			to_host.append8(RC_OK);
		    return true;
		case SLAVE_CMD_READ_FROM_EEPROM:
			handleReadEeprom(from_host, to_host);
			return true;
		case SLAVE_CMD_WRITE_TO_EEPROM:
			handleWriteEeprom(from_host, to_host);
			return true;
		case SLAVE_CMD_SET_MOTOR_1_PWM:
			motor1.setSpeed(from_host.read8(2));
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_SET_MOTOR_1_DIR:
			motor1.setDir(from_host.read8(2) == 1);
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_TOGGLE_MOTOR_1:
			motor1.setDir((from_host.read8(2) & 0x02) != 0);
			motor1.setOn((from_host.read8(2) & 0x01) != 0);
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_IS_TOOL_READY:
			to_host.append(RC_OK);
			to_host.append(extruder_heater.hasReachedTargetTemperature()?1:0);
			return true;
		}
	}
    setDebugLED(true);
	return false;
}
