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

#include "Packet.hh"
#include <util/crc16.h>

/// Append a byte and update the CRC
void Packet::appendByte(uint8_t data) {
	if (length < MAX_PACKET_PAYLOAD) {
		crc = _crc_ibutton_update(crc, data);
		payload[length] = data;
		length++;
	}
}
/// Reset this packet to an empty state
void Packet::reset() {
	crc = 0;
	length = 0;
#ifdef PARANOID
	for (uint8_t i = 0; i < MAX_PACKET_PAYLOAD; i++) {
		payload[i] = 0;
	}
#endif // PARANOID
	error_code = PacketError::NO_ERROR;
	state = PS_START;
}

InPacket::InPacket() {
	reset();
}

/// Reset the entire packet reception.
void InPacket::reset() {
	Packet::reset();
}

//process a byte for our packet.
void InPacket::processByte(uint8_t b) {
	if (state == PS_START) {
		if (b == START_BYTE) {
			state = PS_LEN;
		} else {
			error(PacketError::NOISE_BYTE);
		}
	} else if (state == PS_LEN) {
		if (b <= MAX_PACKET_PAYLOAD) {
			expected_length = b;
			state = (expected_length == 0) ? PS_CRC : PS_PAYLOAD;
		} else {
			error(PacketError::EXCEEDED_MAX_LENGTH);
		}
	} else if (state == PS_PAYLOAD) {
		appendByte(b);
		if (length >= expected_length) {
			state = PS_CRC;
		}
	} else if (state == PS_CRC) {
		if (crc == b) {
			state = PS_LAST;
		} else {
			error(PacketError::BAD_CRC);
		}
	}
}

// Reads an 8-bit byte from the specified index of the payload
uint8_t Packet::read8(uint8_t index) const {
	return payload[index];
}
uint16_t Packet::read16(uint8_t index) const {
	return payload[index] | (payload[index + 1] << 8);
}
uint32_t Packet::read32(uint8_t index) const {
	union {
		// AVR is little-endian
		int32_t a;
		struct {
			uint8_t data[4];
		} b;
	} shared;
	shared.b.data[0] = payload[index];
	shared.b.data[1] = payload[index+1];
	shared.b.data[2] = payload[index+2];
	shared.b.data[3] = payload[index+3];

	return shared.a;
}

OutPacket::OutPacket() {
	reset();
}

/// Reset the entire packet transmission.
void OutPacket::reset() {
	Packet::reset();
	send_payload_index = 0;
}

void OutPacket::prepareForResend() {
	error_code = PacketError::NO_ERROR;
	state = PS_START;
	send_payload_index = 0;
}
uint8_t OutPacket::getNextByteToSend() {
	uint8_t next_byte = 0;
	if (state == PS_START) {
		next_byte = START_BYTE;
		state = PS_LEN;
	} else if (state == PS_LEN) {
		next_byte = length;
		state = (length==0)?PS_CRC:PS_PAYLOAD;
	} else if (state == PS_PAYLOAD) {
		next_byte= payload[send_payload_index++];
		if (send_payload_index >= length) {
			state = PS_CRC;
		}
	} else if (state == PS_CRC) {
		next_byte = crc;
		state = PS_LAST;
	}
	return next_byte;
}

// Add an 8-bit byte to the end of the payload
void OutPacket::append8(uint8_t value) {
	appendByte(value);
}
void OutPacket::append16(uint16_t value) {
	appendByte(value&0xff);
	appendByte((value>>8)&0xff);
}
void OutPacket::append32(uint32_t value) {
	appendByte(value&0xff);
	appendByte((value>>8)&0xff);
	appendByte((value>>16)&0xff);
	appendByte((value>>24)&0xff);
}
