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

#ifndef SHARED_PACKET_HH_
#define SHARED_PACKET_HH_

#include <stdint.h>

#define START_BYTE 0xD5
#define MAX_PACKET_PAYLOAD 32

#define SLAVE_ID_BROADCAST 127

namespace PacketError {
enum {
	NO_ERROR,
	NOISE_BYTE,
	EXCEEDED_MAX_LENGTH,
	BAD_CRC,
	PACKET_TIMEOUT,
};
} // namespace PacketError

/// These are the response codes returned to the requestor
/// as the first byte of the response payload.  You can find
/// a list of the expected codes in the protocol specification
/// document:
/// http://docs.google.com/Doc?docid=0AcWKwJ2SAxDzZGd6amZyY2NfMmdtODRnZ2Ri&hl=en
typedef enum {
        RC_PACKET_ERROR    = 0x80, /* Generic Error. Is this packet simply discarded? */
        RC_OK               = 0x81,
        RC_BUFFER_OVERFLOW  = 0x82,
        RC_CRC_MISMATCH     = 0x83,
        RC_PACKET_LENGTH    = 0x84,
        RC_CMD_UNSUPPORTED  = 0x85,
        RC_EXPECT_MORE      = 0x86,
        RC_DOWNSTREAM_TIMEOUT = 0x87,
        RC_TOOL_LOCK_TIMEOUT = 0x88,
        RC_CANCEL_BUILD		= 0x89, 
        RC_BOT_BUILDING		= 0x8A,  // this response is returned if the bot is building from SD card and the host attempts to send action commands
        RC_BOT_OVERHEAT		= 0x8B,	// if the bot overheats, it will not respond to commands
        RC_PACKET_TIMEOUT	= 0x8C
} ResponseCode;

/// Convenience function to accept old response codes
/// (missing the high bits) as well as the new forms.
inline bool rcCompare(uint8_t data, ResponseCode code) {
	return (data & 0x7f) == (code & 0x7f);
}

class Packet {
protected:
	// packet states
	typedef enum {
		PS_START,
		PS_LEN,
		PS_PAYLOAD,
		PS_CRC,
		PS_LAST
	} PacketState;

    volatile uint8_t length; /// The current length of the payload (data[0] if raw packets)
    volatile uint8_t crc; /// The CRC of the current contents of the payload (data[-1] of raw packets)
    volatile uint8_t payload[MAX_PACKET_PAYLOAD]; /// Data payload (starts at data[2] of raw packet)
	volatile uint8_t error_code; // Have any errors cropped up during processing?
	volatile PacketState state;


	/// Append a byte and update the CRC
	void appendByte(uint8_t data);
	/// Reset this packet to an empty state
	void reset();

	void error(uint8_t error_code_in) {
		reset();
		error_code = error_code_in;
	}
public:
	uint8_t getLength() const { return length; }

	bool hasError() const {
		return error_code != PacketError::NO_ERROR;
	}

	uint8_t getErrorCode() const { return error_code; }

	// Reads an 8-bit byte from the specified index of the payload
	uint8_t read8(uint8_t idx) const;
	uint16_t read16(uint8_t idx) const;
	uint32_t read32(uint8_t idx) const;

	uint8_t debugGetState() const { return state; }

	const volatile uint8_t* getData() const { return payload; }
};

/// Input Packet.
class InPacket: public Packet {
private:
	volatile uint8_t expected_length;
public:
	InPacket();

	/// Reset the entire packet reception.
	void reset();

	//process a byte for our packet.
	void processByte(uint8_t b);

	bool isFinished() const {
		return state == PS_LAST;
	}

	bool isStarted() const {
		return state != PS_START;
	}

	/// Indicate that this packet has timed out.  This means:
	/// * setting the PACKET_TIMEOUT error on the packet
	/// * the packet gets reset
	void timeout() {
		error(PacketError::PACKET_TIMEOUT);
	}
};

/// Output Packet.
class OutPacket: public Packet {
private:
	uint8_t send_payload_index;
public:
	OutPacket();

	/// Reset the entire packet transmission.
	void reset();

	bool isFinished() const {
		return state == PS_LAST;
	}

	bool isSending() const {
		return state != PS_START && state != PS_LAST;
	}

	uint8_t getNextByteToSend();

	// Prepare the output packet for resending with the current data
	void prepareForResend();

	// Add an 8-bit byte to the end of the payload
	void append8(uint8_t value);
	void append16(uint16_t value);
	void append32(uint32_t value);
};

#endif // SHARED_PACKET_HH_
