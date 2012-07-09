
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

std::ostream& operator<<(std::ostream& stream, const uint8_t val) {
  return stream << (unsigned int)val;
}

const char* default_port = "/dev/ttyUSB0";
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

const int INDIVIDUAL_TEST_COUNT = 10;
const int MIXED_TEST_COUNT = 100;
const int STRESS_TEST_COUNT = 5000;

class SerialTest : public ::testing::Test {
protected:
	const char* port_name_;
	int serial_fd_;
	struct termios oldtio_;
	InPacket in_;
	OutPacket out_;
  uint16_t sequence_number_;
  uint16_t tests_run;
  uint16_t tests_succeeded;
  void recordTest() {
    tests_run++;
  }
  void recordSuccess() {
    tests_succeeded++;
  }
  void clearStats() {
    tests_run = 0;
    tests_succeeded = 0;
  }
  void reportStats() {
    if (HasFatalFailure()) {
      FAIL() << " (" << (tests_run-tests_succeeded) <<
	" out of " << tests_run << " failed)";
    }
  }
public:
	SerialTest() : sequence_number_(0) { srandom(time(0)); }
	void SetUp() {
	  clearStats();
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
		tcflush(serial_fd_, TCIOFLUSH);
		tcsetattr(serial_fd_, TCSANOW, &newtio);
	}
	void TearDown() {
	  reportStats();
		/* restore old port settings */
		tcsetattr(serial_fd_, TCSANOW, &oldtio_);
		close(serial_fd_);
	}
	uint8_t makePacket(bool passthru);
	uint8_t makeVersionPacket();
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

uint8_t SerialTest::makeVersionPacket() {
	// remove one byte for echo, another for passthru
  uint8_t length_in_bytes = 3;
  out_.append8(0x0);
  out_.append8(0x0);
  out_.append8(0x15);
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

void SerialTest::runPacket(bool passthru,bool timeout) {
	recordTest();
	// write a packet
	in_.reset();
	out_.reset();
	sequence_number_++;
	uint8_t packet_length = makePacket(passthru);
	uint8_t to_send = timeout? ((random()%(packet_length+1))+1) : -1;
	writePacket(to_send);
	readPacketWithTimeout(80);
	if (timeout) {
		ASSERT_FALSE(in_.isFinished());
		ASSERT_TRUE(in_.hasError());
		ASSERT_EQ(in_.getErrorCode(),PacketError::PACKET_TIMEOUT);
	} else {
		ASSERT_TRUE(in_.isFinished());
		ASSERT_FALSE(in_.hasError());
		if (in_.getLength() == 1 && packet_length != 1) {
		  // Get the presumed response code, like a downstream timeout,
		  // logged
		  ASSERT_EQ(RC_OK,in_.read8(0));
		}
		ASSERT_EQ(in_.getLength(),packet_length);
		for (int i = 0; i < packet_length; i++) {
			uint8_t val = (i % 2 == 0) ? (sequence_number_ & 0xff)
					: (sequence_number_ >> 8);
			ASSERT_EQ(in_.read8(i),val);
		}
	}
	recordSuccess();
}

TEST_F(SerialTest,GoodVersionPacket) {
  for (int i = 0; i < INDIVIDUAL_TEST_COUNT; i++) {
    // write a packet
    in_.reset();
    out_.reset();
    sequence_number_++;
    makeVersionPacket();
    writePacket();
    readPacketWithTimeout(80);
    ASSERT_TRUE(in_.isFinished());
    ASSERT_FALSE(in_.hasError());
    if (in_.getLength() == 2) {
      // Get the presumed response code, like a downstream timeout,
      // logged
      ASSERT_EQ(RC_OK,in_.read8(0));
    }
  }
}

TEST_F(SerialTest,GoodLocalPacket) {
	for (int i = 0; i < INDIVIDUAL_TEST_COUNT; i++) {
	  runPacket(false,false);
	}
}


TEST_F(SerialTest,TimeoutLocalPacket) {
	for (int i = 0; i < INDIVIDUAL_TEST_COUNT; i++) {
	  runPacket(false,true);
	}
}


TEST_F(SerialTest,GoodPassthruPacket) {
	for (int i = 0; i < INDIVIDUAL_TEST_COUNT; i++) {
	  runPacket(true,false);
	}
}

TEST_F(SerialTest,TimeoutPassthruPacket) {
	for (int i = 0; i < INDIVIDUAL_TEST_COUNT; i++) {
	  runPacket(true,true);
	}
}


TEST_F(SerialTest,MixedLocalPackets) {
	for (int i = 0; i < MIXED_TEST_COUNT; i++) {
		bool timeout = (random()%2) == 0;
		runPacket(false,timeout);
	}
}


TEST_F(SerialTest,MixedPassthruPackets) {
	for (int i = 0; i < MIXED_TEST_COUNT; i++) {
		bool timeout = (random()%2) == 0;
		runPacket(true,timeout);
	}
}

TEST_F(SerialTest,MixedAllPackets) {
	for (int i = 0; i < STRESS_TEST_COUNT; i++) {
		bool passthru = (random()%2) == 0;
		bool timeout = (random()%2) == 0;
		runPacket(passthru,timeout);
	}
}
