/*
 * T1-UART-exerciser.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

/// Based on the hoary old serial-programming HOWTO

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <gtest/gtest.h>

#include "util/Packet.hh"

const char* default_port = "/dev/ttyUSB0";
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

class SerialTest : public ::testing::Test {
protected:
	const char* port_name_;
	int serial_fd_;
	struct termios oldtio_;
	InPacket in_;
	OutPacket out_;
	uint16_t sequence_number_;
public:
	SerialTest() : sequence_number_(0) {}
	void SetUp() {
		port_name_ = getenv("PORT");
		if (port_name_ == NULL) {
			port_name_ = default_port;
		}
		struct termios newtio;
		char buf[255];

		/* open the device to be non-blocking (read will return immediately) */
		serial_fd_ = open(port_name_, O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (serial_fd_ < 0) {
			perror(port_name_);
			return;
		}

		tcgetattr(serial_fd_, &oldtio_); /* save current port settings */
		/* set new port settings for canonical input processing */
		newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

		newtio.c_cflag &= ~PARENB;
		newtio.c_cflag &= ~CSTOPB;
		newtio.c_cflag &= ~CSIZE;
		newtio.c_cflag |= CS8;

		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;
		newtio.c_lflag = 0;
		newtio.c_cc[VMIN] = 1;
		newtio.c_cc[VTIME] = 0;
		tcflush(serial_fd_, TCIFLUSH);
		tcsetattr(serial_fd_, TCSANOW, &newtio);
	}
	void TearDown() {
		/* restore old port settings */
		tcsetattr(serial_fd_, TCSANOW, &oldtio_);
		close(serial_fd_);
	}
	uint8_t makePacket(bool passthru);
	void writePacket(int16_t stop_after = -1);
	void readPacketWithTimeout(uint16_t timeout);

	void runPacket(bool passthru, bool timeout);
};

const char* port_string = default_port;

class Timer {
private:
	struct timeval start_;
public:
	Timer() {
		gettimeofday(&start_, NULL);
	}
	long millisSince() {
		struct timeval now;
		gettimeofday(&now, NULL);
		return ((now.tv_sec - start_.tv_sec) * 1000L)
					+ (now.tv_usec - start_.tv_usec) / 1000L;
	}
};

uint8_t SerialTest::makePacket(bool passthru) {
	// remove one byte for echo, another for passthru
	const uint8_t max_payload_length = passthru?30:31;
	uint8_t length_in_bytes = random() % max_payload_length;
	if (passthru) {
		out_.append8(0x73);
	}
	out_.append8(0x70); // echo test
	for (int i = 0; i < length_in_bytes; i++) {
		uint8_t val = (i % 2 == 0) ? (sequence_number_ & 0xff) : (sequence_number_ >> 8);
		out_.append8(val);
	}
	return length_in_bytes;
}

void SerialTest::writePacket(int16_t stop_after) {
	while (!out_.isFinished() && stop_after-- != 0) {
		uint8_t out_byte = out_.getNextByteToSend();
		write(serial_fd_, &out_byte, 1);
	}
}

void SerialTest::readPacketWithTimeout(uint16_t timeout) {
	Timer mark;
	while (!in_.isFinished()) {
		if (mark.millisSince() > 50) {
			in_.timeout();
			break;
		}
		uint8_t in_byte;
		int count = read(serial_fd_, &in_byte, 1);
		if (count == 1) {
			in_.processByte(in_byte);
		}
	}
}

void SerialTest::runPacket(bool passthru,bool timeout) {
	// write a packet
	in_.reset();
	out_.reset();
	sequence_number_++;
	uint8_t packet_length = makePacket(passthru);
	uint8_t to_send = timeout? ((random()%(packet_length+1))+1) : -1;
	writePacket(to_send);
	readPacketWithTimeout(50);
	if (timeout) {
		ASSERT_FALSE(in_.isFinished());
		ASSERT_TRUE(in_.hasError());
		ASSERT_EQ(in_.getErrorCode(),PacketError::PACKET_TIMEOUT);
	} else {
		ASSERT_TRUE(in_.isFinished());
		ASSERT_FALSE(in_.hasError());
		ASSERT_EQ(in_.getLength(),packet_length);
		for (int i = 0; i < packet_length; i++) {
			uint8_t val = (i % 2 == 0) ? (sequence_number_ & 0xff)
					: (sequence_number_ >> 8);
			ASSERT_EQ(in_.read8(i),val);
		}
	}
}

TEST_F(SerialTest,GoodLocalPacket) {
	runPacket(false,false);
}


TEST_F(SerialTest,TimeoutLocalPacket) {
	runPacket(false,true);
}

TEST_F(SerialTest,GoodPassthruPacket) {
	runPacket(true,false);
}

TEST_F(SerialTest,TimeoutPassthruPacket) {
	runPacket(true,true);
}

TEST_F(SerialTest,MixedLocalPackets) {
	for (int i = 0; i < 1000; i++) {
		bool timeout = (random()%2) == 0;
		runPacket(false,timeout);
	}
}

TEST_F(SerialTest,MixedPassthruPackets) {
	for (int i = 0; i < 1000; i++) {
		bool timeout = (random()%2) == 0;
		runPacket(true,timeout);
	}
}

TEST_F(SerialTest,MixedAllPackets) {
	for (int i = 0; i < 1000; i++) {
		bool passthru = (random()%2) == 0;
		bool timeout = (random()%2) == 0;
		runPacket(passthru,timeout);
	}
}
