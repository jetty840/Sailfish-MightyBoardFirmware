#include "Packet.hh"
#include "Timeout.hh"
#include <util/crc16.h>

// Incoming packet timeout, in ms
#define INCOMING_TIMEOUT 13

/// Append a byte and update the CRC
void Packet::appendByte(uint8_t data) {
	if (length_ < MAX_PACKET_PAYLOAD) {
		crc_ = _crc_ibutton_update(crc_, data);
		payload_[length_] = data;
		length_++;
	}
}
/// Reset this packet to an empty state
void Packet::reset() {
	crc_ = 0;
	length_ = 0;
//#if PARANOID
	for (uint8_t i = 0; i < MAX_PACKET_PAYLOAD; i++) {
		payload_[i] = 0;
	}
//#endif // PARANOID
	error_ = PacketError::NO_ERROR;
	state_ = PS_START;
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
	if (state_ == PS_START) {
		if (b == START_BYTE) {
			state_ = PS_LEN;
			TimeoutManager::addTimeout(*this,INCOMING_TIMEOUT);
		} else {
			error(PacketError::NOISE_BYTE);
		}
	} else if (state_ == PS_LEN) {
		if (b < MAX_PACKET_PAYLOAD) {
			expected_length_ = b;
			state_ = (expected_length_ == 0) ? PS_CRC : PS_PAYLOAD;
		} else {
			error(PacketError::EXCEEDED_MAX_LENGTH);
		}
	} else if (state_ == PS_PAYLOAD) {
		appendByte(b);
		if (length_ >= expected_length_) {
			state_ = PS_CRC;
		}
	} else if (state_ == PS_CRC) {
		TimeoutManager::cancelTimeout(*this);
		if (crc_ == b) {
			state_ = PS_LAST;
		} else {
			error(PacketError::BAD_CRC);
		}
	}
}

// Reads an 8-bit byte from the specified index of the payload
uint8_t Packet::read8(uint8_t index) const {
	return payload_[index];
}
uint16_t Packet::read16(uint8_t index) const {
	return payload_[index] | (payload_[index + 1] << 8);
}
uint32_t Packet::read32(uint8_t index) const {
	return payload_[index] | (payload_[index + 1] << 8) | (payload_[index + 2]
			<< 16) | (payload_[index + 3] << 24);
}

OutPacket::OutPacket() {
	reset();
}

/// Reset the entire packet transmission.
void OutPacket::reset() {
	Packet::reset();
	send_payload_index_ = 0;
}

uint8_t OutPacket::getNextByteToSend() {
	uint8_t next_byte = 0;
	if (state_ == PS_START) {
		next_byte = START_BYTE;
		state_ = PS_LEN;
	} else if (state_ == PS_LEN) {
		next_byte = length_;
		state_ = (length_==0)?PS_CRC:PS_PAYLOAD;
	} else if (state_ == PS_PAYLOAD) {
		next_byte= payload_[send_payload_index_++];
		if (send_payload_index_ >= length_) {
			state_ = PS_CRC;
		}
	} else if (state_ == PS_CRC) {
		next_byte = crc_;
		state_ = PS_LAST;
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
