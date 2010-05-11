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

#include "Packet.hh"

#define COMMAND_BUFFER_SIZE 256

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
		newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

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
	void makeBufferClearPacket();
	void makeBufferFillerPacket(uint8_t size);
	void writePacket();
	void readPacketWithTimeout(uint16_t timeout);
	void makeMoveTo(int x, int y, int z, int dda);
	void makeSetPos(int x, int y, int z);
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

void SerialTest::makeBufferClearPacket() {
	out_.append8(0x74);
}

void SerialTest::makeBufferFillerPacket(uint8_t total_size) {
	ASSERT_GE(total_size,2);
	out_.append8(0xF0);
	out_.append8(total_size-2);
	for (int i = 2; i < total_size; i++) {
		out_.append8(0x55);
	}
}

void SerialTest::makeMoveTo(int x, int y, int z, int dda) {
	out_.append8(129);
	out_.append32(x);
	out_.append32(y);
	out_.append32(z);
	out_.append32(dda);
}

void SerialTest::makeSetPos(int x, int y, int z) {
	out_.append8(130);
	out_.append32(x);
	out_.append32(y);
	out_.append32(z);
}

void SerialTest::writePacket() {
	while (!out_.isFinished()) {
		uint8_t out_byte = out_.getNextByteToSend();
		write(serial_fd_, &out_byte, 1);
	}
}

void SerialTest::readPacketWithTimeout(uint16_t timeout) {
	Timer mark;
	while (!in_.isFinished()) {
		if (mark.millisSince() > timeout) {
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

TEST_F(SerialTest,CheckBufferFill) {
	/// Clear buffer
	in_.reset();
	out_.reset();
	makeBufferClearPacket(); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);
	/// Fill buffer exactly.  Buffer size = 256 byes.
	for (int i = 0; i < 256; i += 8) {
		in_.reset();
		out_.reset();
		makeBufferFillerPacket(8); writePacket(); readPacketWithTimeout(50);
		ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
		ASSERT_EQ(in_.read8(0), RC_OK) << " Response code " << (int)in_.read8(0) << " on loop " << i;
	}
	/// Add another packet, and get a buffer full message.
	in_.reset();
	out_.reset();
	makeBufferFillerPacket(16); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_BUFFER_OVERFLOW);
}

TEST_F(SerialTest,CheckOddFill) {
	/// Clear buffer
	in_.reset();
	out_.reset();
	makeBufferClearPacket(); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);
	/// Fill buffer exactly.  Buffer size = 256 byes.
	const int cmdsize = 7;
	for (int i = 0; i < 256-cmdsize; i += cmdsize) {
		in_.reset();
		out_.reset();
		makeBufferFillerPacket(cmdsize); writePacket(); readPacketWithTimeout(50);
		ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
		ASSERT_EQ(in_.read8(0), RC_OK) << " Response code " << (int)in_.read8(0) << " on loop " << i;
	}
	/// Add another packet, and get a buffer full message.
	in_.reset();
	out_.reset();
	makeBufferFillerPacket(cmdsize); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_BUFFER_OVERFLOW);
}

TEST_F(SerialTest,BasicMove) {
	in_.reset();
	out_.reset();
	makeBufferClearPacket(); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeSetPos(0,0,0); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeMoveTo(500,0,0,1000);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeMoveTo(0,0,0,1000);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);
}

TEST_F(SerialTest,SetPos) {
//	in_.reset();
//	out_.reset();
//	makeBufferClearPacket(); writePacket(); readPacketWithTimeout(50);
//	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
//	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeSetPos(0,0,0); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeMoveTo(0,500,0,1000);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeSetPos(0,0,0); writePacket(); readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);

	in_.reset();
	out_.reset();
	makeMoveTo(0,-500,0,1000);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished()); ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);
}
