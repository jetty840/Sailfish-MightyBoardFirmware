#include <gtest/gtest.h>
#include <stdlib.h>
#include <util/crc16.h>
#include "Packet.hh"

using namespace std;

/// Check that all simple, correctly sent packets are interpreted correctly.
TEST(PacketTest, InPacket)
{
	InPacket packet;
	// Test all valid packet sizes
	for (int packet_size = MAX_PACKET_PAYLOAD - 1; packet_size >= 0; packet_size--) {
		uint8_t payload[packet_size];
		uint8_t expected_crc = 0;
		for (int i = 0; i < packet_size; i++) {
			payload[i] = random(); // Randomize packet contents
			expected_crc = _crc_ibutton_update(expected_crc, payload[i]);
		}
		// process packet
		packet.processByte(START_BYTE);
		ASSERT_FALSE(packet.isFinished());
		packet.processByte(packet_size);
		ASSERT_FALSE(packet.isFinished());
		for (int i = 0; i < packet_size; i++) {
			packet.processByte(payload[i]);
			ASSERT_FALSE(packet.isFinished());
		}
		packet.processByte(expected_crc);
		ASSERT_FALSE(packet.hasError()) << " with error code " << (int)packet.getErrorCode();
		ASSERT_TRUE(packet.isFinished());
		// check payload
		for (int i = 0; i < packet_size; i++) {
			ASSERT_EQ(packet.read8(i),payload[i]);
		}
		packet.reset();
	}
}

TEST(PacketTest, InBadCRC)
{
	InPacket packet;
	// Test all valid packet sizes
	for (int packet_size = MAX_PACKET_PAYLOAD - 1; packet_size >= 0; packet_size--) {
		uint8_t payload[packet_size];
		uint8_t expected_crc = 0;
		for (int i = 0; i < packet_size; i++) {
			payload[i] = random(); // Randomize packet contents
			expected_crc = _crc_ibutton_update(expected_crc, payload[i]);
		}
		// process packet
		packet.processByte(START_BYTE);
		packet.processByte(packet_size);
		for (int i = 0; i < packet_size; i++) {
			packet.processByte(payload[i]);
		}
		uint8_t false_crc = random();
		if (false_crc == expected_crc) {
			false_crc++;
		}
		packet.processByte(false_crc);
		ASSERT_TRUE(packet.hasError());
		ASSERT_EQ(packet.getErrorCode(),PacketError::BAD_CRC);
		packet.reset();
	}
}

TEST(PacketTest, InMissingStart)
{
	InPacket packet;
	// Test all valid packet sizes
	for (int packet_size = MAX_PACKET_PAYLOAD - 1; packet_size >= 0; packet_size--) {
		uint8_t payload[packet_size];
		uint8_t expected_crc = 0;
		for (int i = 0; i < packet_size; i++) {
			payload[i] = random(); // Randomize packet contents
			expected_crc = _crc_ibutton_update(expected_crc, payload[i]);
		}
		// process packet
		// Missing start byte would go here.
		packet.processByte(packet_size);
		for (int i = 0; i < packet_size; i++) {
			packet.processByte(payload[i]);
		}
		uint8_t false_crc = random();
		if (false_crc == expected_crc) {
			false_crc++;
		}
		packet.processByte(false_crc);
		// It is possible that the start byte appeared somewhere in the payload,
		// *and* that the packet is interpreted as being in the right length range,
		// in which case we may be in progress and waiting for a timeout.
		ASSERT_TRUE(packet.hasError() ||
				(packet.isStarted() && !packet.isFinished()));
		packet.reset();
	}
}

// Output packets
TEST(PacketTest, OutPacket)
{
	OutPacket packet;
	srand(time(0));
	// Test all valid packet sizes
	for (int packet_size = MAX_PACKET_PAYLOAD - 1; packet_size >= 0; packet_size--) {
		uint8_t payload[packet_size];
		uint8_t expected_crc = 0;
		for (int i = 0; i < packet_size; i++) {
			payload[i] = random(); // Randomize packet contents
			expected_crc = _crc_ibutton_update(expected_crc, payload[i]);
		}
		// prepare packet
		for (int i = 0; i < packet_size; i++) {
			packet.append8(payload[i]);
			ASSERT_FALSE(packet.hasError());
		}
		ASSERT_EQ(packet.getLength(),packet_size);
		// check packet
		ASSERT_EQ(packet.getNextByteToSend(), START_BYTE);
		ASSERT_EQ(packet.getNextByteToSend(), packet_size);
		for (int i = 0; i < packet_size; i++) {
			uint8_t tx_byte = packet.getNextByteToSend();
			ASSERT_EQ(tx_byte, payload[i]) << " Packet index " << i << '/' << packet_size
			<< hex << " expect " << payload[i] << " got " << tx_byte;
		}
		ASSERT_FALSE(packet.isFinished());
		{
			uint8_t tx_byte = packet.getNextByteToSend();
			ASSERT_EQ(tx_byte,expected_crc) << "CRC expect " << hex << (int)expected_crc
			<< " got " << (int)tx_byte;
		}
		ASSERT_TRUE(packet.isFinished());
		packet.reset();
	}
}

// Output packets
TEST(PacketTest, PacketTrip)
{
	OutPacket out_packet;
	InPacket in_packet;
	// Test all valid packet sizes
	for (int packet_size = MAX_PACKET_PAYLOAD - 1; packet_size >= 0; packet_size--) {
		uint8_t payload[packet_size];
		uint8_t expected_crc = 0;
		for (int i = 0; i < packet_size; i++) {
			payload[i] = random(); // Randomize packet contents
			expected_crc = _crc_ibutton_update(expected_crc, payload[i]);
		}
		// prepare packet
		for (int i = 0; i < packet_size; i++) {
			out_packet.append8(payload[i]);
			ASSERT_FALSE(out_packet.hasError());
		}
		ASSERT_EQ(out_packet.getLength(),packet_size);
		// transfer packet
		while (!out_packet.isFinished()) {
			in_packet.processByte(out_packet.getNextByteToSend());
		}
		// check packet receipt
		ASSERT_FALSE(in_packet.hasError()) << "In error code: " << hex << in_packet.getErrorCode();
		ASSERT_FALSE(out_packet.hasError()) << "Out error code: " << hex << out_packet.getErrorCode();
		ASSERT_EQ(in_packet.getLength(), packet_size);
		ASSERT_EQ(out_packet.getLength(), packet_size);
		for (int i = 0; i < packet_size; i++) {
			ASSERT_EQ(in_packet.read8(i), out_packet.read8(i));
		}
		ASSERT_TRUE(in_packet.isFinished());
		in_packet.reset();
		out_packet.reset();
	}
}

TEST(PacketTest, PacketSizes)
{
	OutPacket out_packet;
	InPacket in_packet;
	// Test word sizes
	const int packet_size = 4*2 + 2*2 + 1;
	uint8_t expected_crc = 0;
	// prepare packet
	uint32_t p32 = random();
	uint16_t p16 = random();
	uint8_t p8 = random();

	out_packet.append8(p8);
	out_packet.append32(p32);
	out_packet.append16(p16);
	out_packet.append32(p32);
	out_packet.append16(p16);

	// transfer packet
	while (!out_packet.isFinished()) {
		in_packet.processByte(out_packet.getNextByteToSend());
	}
	// check packet receipt
	ASSERT_FALSE(in_packet.hasError()) << "In error code: " << hex << in_packet.getErrorCode();
	ASSERT_FALSE(out_packet.hasError()) << "Out error code: " << hex << out_packet.getErrorCode();
	ASSERT_EQ(out_packet.getLength(),in_packet.getLength());
	ASSERT_EQ(in_packet.read8(0),p8);
	ASSERT_EQ(in_packet.read32(1),p32);
	ASSERT_EQ(in_packet.read16(5),p16);
	ASSERT_EQ(in_packet.read32(7),p32);
	ASSERT_EQ(in_packet.read16(11),p16);
}
