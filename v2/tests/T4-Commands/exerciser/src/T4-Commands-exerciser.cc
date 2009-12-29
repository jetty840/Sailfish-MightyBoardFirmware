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
#include "Commands.hh"

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

	void writePacket();
	void readPacketWithTimeout(uint16_t timeout);

	void runPacket();
	void reset();
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

void SerialTest::runPacket() {
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished());
	ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_OK);
}

void SerialTest::reset() {
	in_.reset();
	out_.reset();
}

TEST_F(SerialTest,CheckVersion) {
	reset();
	out_.append8(0x00);
	runPacket();
	/// Version should be >=200, <300
	ASSERT_GE(in_.read8(1), 200);
	ASSERT_LT(in_.read8(1), 300);
}


TEST_F(SerialTest,Init) {
	reset();
	out_.append8(0x01);
	runPacket();
	/// No response
}


TEST_F(SerialTest,IsFinished) {
	reset();
	out_.append8(0x01);
	runPacket();
	/// No response
}

TEST_F(SerialTest,NoSuchCommand) {
	reset();
	out_.append8(0x75);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished());
	ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_CMD_UNSUPPORTED);
}
