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
		}
	}
	setDebugLED(false);
	return false;
}
