/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "Host.hh"
#include "UART.hh"
#include "Timeout.hh"
#include <util/atomic.h>
#include <avr/eeprom.h>
#include "DebugPacketProcessor.hh"
#include "Configuration.hh"
#include "ExtruderBoard.hh"
#include "Commands.hh"
#include "Version.hh"
#include "MotorController.hh"
#include "Main.hh"
#include "EepromMap.hh"

// Timeout from time first bit recieved until we abort packet reception
Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20L
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)
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

inline void handlePause(const InPacket& from_host, OutPacket& to_host) {
	MotorController::getController().pause();
	to_host.append8(RC_OK);
}

bool do_host_reset = false;

bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	ExtruderBoard& board = ExtruderBoard::getBoard();
	if (from_host.getLength() >= 1) {
		MotorController& motor = MotorController::getController();
		uint8_t command = from_host.read8(1);
		// All commands are query commands.

		switch (command) {
		case SLAVE_CMD_VERSION:
			to_host.append8(RC_OK);
			to_host.append16(firmware_version);
			return true;
		case SLAVE_CMD_GET_BUILD_NAME:
			to_host.append8(RC_OK);
			{
			  for (uint8_t idx = 0; idx < 31; idx++) {
			    to_host.append8(build_name[idx]);
			    if (build_name[idx] == '\0') { break; }
			  }
			}
			return true;
		case SLAVE_CMD_INIT:
			do_host_reset = true;
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_GET_TEMP:
			to_host.append8(RC_OK);
			to_host.append16(board.getExtruderHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_SET_TEMP:
			board.getExtruderHeater().set_target_temperature(from_host.read16(2));
			to_host.append8(RC_OK);
		    return true;
		case SLAVE_CMD_READ_FROM_EEPROM:
			handleReadEeprom(from_host, to_host);
			return true;
		case SLAVE_CMD_WRITE_TO_EEPROM:
			handleWriteEeprom(from_host, to_host);
			return true;
		case SLAVE_CMD_PAUSE_UNPAUSE:
			handlePause(from_host, to_host);
			return true;
		case SLAVE_CMD_SET_MOTOR_1_PWM:
			motor.setSpeed(from_host.read8(2));
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_SET_MOTOR_1_DIR:
			motor.setDir(from_host.read8(2) == 1);
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_TOGGLE_MOTOR_1:
			motor.setDir((from_host.read8(2) & 0x02) != 0);
			motor.setOn((from_host.read8(2) & 0x01) != 0);
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_SET_MOTOR_1_RPM:
			motor.setRPMSpeed(from_host.read32(2));
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_TOGGLE_FAN:
			board.setFan((from_host.read8(2) & 0x01) != 0);
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_TOGGLE_VALVE:
			board.setValve((from_host.read8(2) & 0x01) != 0);
		case SLAVE_CMD_IS_TOOL_READY:
			to_host.append8(RC_OK);
			to_host.append8(board.getExtruderHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_PLATFORM_TEMP:
			to_host.append8(RC_OK);
			to_host.append16(board.getPlatformHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_SET_PLATFORM_TEMP:
			board.setUsingPlatform(true);
			board.getPlatformHeater().set_target_temperature(from_host.read16(2));
			to_host.append8(RC_OK);
			return true;
		case SLAVE_CMD_SET_SERVO_1_POS:
#if HAS_SERVOS
		{
			uint8_t v = from_host.read8(2);
			if (v == 255) {
				board.setServo(0,-1);
			} else {
				if (v > 180) v = 180;
				board.setServo(0,v);
			}
		}
			to_host.append8(RC_OK);
#else
			to_host.append8(RC_CMD_UNSUPPORTED);
#endif
			return true;
		case SLAVE_CMD_GET_SP:
			to_host.append8(RC_OK);
			to_host.append16(board.getExtruderHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_GET_PLATFORM_SP:
			to_host.append8(RC_OK);
			to_host.append16(board.getPlatformHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_IS_PLATFORM_READY:
			to_host.append8(RC_OK);
			to_host.append8(board.getPlatformHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_TOOL_STATUS:
			to_host.append8(RC_OK);
			to_host.append8( (board.getExtruderHeater().has_failed()?128:0)
					        | (board.getResetFlags() << 2)
							| (board.getExtruderHeater().has_reached_target_temperature()?1:0));
			to_host.append16(board.getExtruderHeater().getPIDErrorTerm());
			to_host.append16(board.getExtruderHeater().getPIDDeltaTerm());
			return true;
		}
	}
	return false;
}

extern int cycles;

void runHostSlice() {
	UART& uart = ExtruderBoard::getBoard().getHostUART();
	InPacket& in = uart.in;
	OutPacket& out = uart.out;
	if (out.isSending()) {
		// still sending; wait until send is complete before reading new host packets.
		return;
	}
	if (do_host_reset) {
		do_host_reset = false;
		reset();
	}
	if (in.isStarted() && !in.isFinished()) {
		if (!packet_in_timeout.isActive()) {
			// initiate timeout
			packet_in_timeout.start(HOST_PACKET_TIMEOUT_MICROS);
		} else if (packet_in_timeout.hasElapsed()) {
			in.timeout();
			uart.reset();
		}
	}
	if (in.hasError()) {
		packet_in_timeout.abort();
		// REPORTING: report error.
		// Reset packet quickly and start handling the next packet.
		in.reset();
		uart.reset();
	}
	if (in.isFinished()) {
		out.reset();
		const uint8_t slave_id = eeprom::getEeprom8(eeprom::SLAVE_ID, 0);
		const uint8_t target = in.read8(0);
		packet_in_timeout.abort();
		// SPECIAL CASE: we always process debug packets!
		if (processDebugPacket(in,out)) {
			// okay, processed
		} else if ( (target == slave_id) || (target == 255) ) {
			// only process packets for us
			if (processQueryPacket(in, out)) {
				// okay, processed
			} else {
				// Unrecognized command
				out.append8(RC_CMD_UNSUPPORTED);
			}
		} else {
			// Not for us-- no response
			in.reset();
			return;
		}
		in.reset();
		uart.beginSend();
	}
}
