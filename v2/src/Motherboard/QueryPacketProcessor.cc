#include <util/atomic.h>
#include <avr/eeprom.h>
#include "QueryPacketProcessor.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "CommandQueue.hh"
#include "Version.hh"
#include "DebugPin.hh"

void init() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		steppers.abort();
		command_buffer.reset();
	}
}

uint32_t getBufferSize() {
	uint32_t sz;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sz = command_buffer.getRemainingCapacity();
	}
	return sz;
}

bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) == 0) {
			// Is query command.
			switch (command) {
			case HOST_CMD_VERSION:
				to_host.append8(RC_OK);
				to_host.append16(firmware_version);
				return true;
			case HOST_CMD_INIT:
			case HOST_CMD_CLEAR_BUFFER: // equivalent at current time
			case HOST_CMD_ABORT: // equivalent at current time
				init();
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_GET_BUFFER_SIZE:
				to_host.append8(RC_OK);
				to_host.append32(getBufferSize());
				return true;
			case HOST_CMD_GET_POSITION:
				ATOMIC_BLOCK(ATOMIC_FORCEON) {
					const Point p = steppers.getPosition();
					to_host.append8(RC_OK);
					to_host.append32(p[0]);
					to_host.append32(p[1]);
					to_host.append32(p[2]);
					to_host.append8(0); // todo: endstops
				}
				return true;
			case HOST_CMD_GET_RANGE:
			case HOST_CMD_SET_RANGE:
				break; // not yet implemented
			case HOST_CMD_PAUSE:
				pauseCommandThread(!isCommandThreadPaused());
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_TOOL_QUERY:
				break; // not yet implemented
			case HOST_CMD_IS_FINISHED:
				to_host.append8(RC_OK);
				ATOMIC_BLOCK(ATOMIC_FORCEON) {
					bool done = !steppers.isRunning() && command_buffer.isEmpty();
					to_host.append8(done?1:0);
				}
				return true;
			case HOST_CMD_READ_EEPROM:
				{
					uint16_t offset = from_host.read16(1);
					uint8_t length = from_host.read8(3);
					uint8_t data[16];
					eeprom_read_block(data,(const void*)offset, length);
					to_host.append8(RC_OK);
					for (int i = 0; i < length; i++) {
						to_host.append8(data[i]);
					}
				}
				return true;
			case HOST_CMD_WRITE_EEPROM:
				{
					uint16_t offset = from_host.read16(1);
					uint8_t length = from_host.read8(3);
					uint8_t data[16];
					eeprom_read_block(data,(const void*)offset, length);
					for (int i = 0; i < length; i++) {
						data[i] = from_host.read8(i+4);
					}
					eeprom_write_block(data,(void*)offset, length);
					to_host.append8(RC_OK);
					to_host.append8(length);
				}
				return true;
			}
		}
	}
	return false;
}
