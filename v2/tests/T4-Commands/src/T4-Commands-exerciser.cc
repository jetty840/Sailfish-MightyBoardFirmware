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
#include <math.h>

#include "Packet.hh"
#include "Commands.hh"

#define COMMAND_BUFFER_SIZE 256

const char* default_port = "/dev/ttyUSB0";
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

class SerialTest: public ::testing::Test {
protected:
	const char* port_name_;
	int serial_fd_;
	struct termios oldtio_;
	InPacket in_;
	OutPacket out_;
	uint16_t sequence_number_;
public:
	SerialTest() :
		sequence_number_(0) {
	}
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

	void writePacket();
	void readPacketWithTimeout(uint16_t timeout);

	void runPacket();
	void reset();

	void resetPosition();
	void moveTo(int x, int y, int z, int dda);

	void delay(float seconds);

	void waitUntilDone();
};

void SerialTest::moveTo(int x, int y, int z, int dda) {
	bool done = false;
	while (!done) {
		reset();
		out_.append8(HOST_CMD_QUEUE_POINT_ABS);
		out_.append32(x);
		out_.append32(y);
		out_.append32(z);
		out_.append32(dda);
		writePacket();
		readPacketWithTimeout(50);
		ASSERT_TRUE(in_.isFinished());
		ASSERT_FALSE(in_.hasError());
		if (in_.read8(0)== RC_OK) {
			done=true;
		} else {
			ASSERT_EQ(in_.read8(0),RC_BUFFER_OVERFLOW);
			// delay for 1ms
			struct timespec t;
			t.tv_sec = 0;
			t.tv_nsec = 10 * 1000 * 1000;
			nanosleep(&t,NULL);
		}
	}
}

void SerialTest::delay(float seconds) {
	bool done = false;
	int32_t milliseconds = (int32_t) (seconds * 1000.0);
	while (!done) {
		reset();
		out_.append8(HOST_CMD_DELAY);
		out_.append32(milliseconds);
		writePacket();
		readPacketWithTimeout(50);
		ASSERT_TRUE(in_.isFinished());
		ASSERT_FALSE(in_.hasError());
		if (in_.read8(0)== RC_OK) {
			done=true;
		} else {
			ASSERT_EQ(in_.read8(0),RC_BUFFER_OVERFLOW);
			// delay for 1ms
			struct timespec t;
			t.tv_sec = 0;
			t.tv_nsec = 10 * 1000 * 1000;
			nanosleep(&t,NULL);
		}
	}
}

void SerialTest::resetPosition() {
	reset();
	out_.append8(HOST_CMD_SET_POSITION);
	out_.append32(0);
	out_.append32(0);
	out_.append32(0);
	runPacket();
}

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
		return ((now.tv_sec - start_.tv_sec) * 1000L) + (now.tv_usec
				- start_.tv_usec) / 1000L;
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
			printf("state is %d\n",in_.debugGetState());
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

TEST_F(SerialTest,CheckVersion)
{
	reset();
	out_.append8(HOST_CMD_VERSION);
	runPacket();
	/// Version should be >=200, <300
	ASSERT_GE(in_.read8(1), 200);
	ASSERT_LT(in_.read8(1), 300);
}
/*
TEST_F(SerialTest,Init)
{
	reset();
	out_.append8(HOST_CMD_INIT);
	runPacket();
	/// No response
}

TEST_F(SerialTest,IsFinished)
{
	SCOPED_TRACE("Initial finished");
	reset();
	out_.append8(HOST_CMD_IS_FINISHED);
	runPacket();
	/// Should respond with a 1 (yes, we're done)
	ASSERT_EQ(in_.read8(1), 1);
	/// Start a move
	SCOPED_TRACE("Adding moves");
	resetPosition();
	moveTo(500,500,0,500);
	moveTo(0,0,0,500);
	/// Should be busy
	reset();
	out_.append8(HOST_CMD_IS_FINISHED);
	runPacket();
	/// Should respond with a 0 (no, not done)
	ASSERT_EQ(in_.read8(1), 0);
	/// Wait until done
	bool done = false;
	while (!done) {
		reset();
		out_.append8(HOST_CMD_IS_FINISHED);
		runPacket();
		done = in_.read8(1) == 1;
	}
}
*/
/// Wait until done
void SerialTest::waitUntilDone() {
	bool done = false;
	while (!done) {
		reset();
		out_.append8(HOST_CMD_IS_FINISHED);
		runPacket();
		done = in_.read8(1) == 1;
	}
}

TEST_F(SerialTest,Abort)
{
	reset();
	out_.append8(HOST_CMD_IS_FINISHED);
	runPacket();
	/// Should respond with a 1 (yes, we're done)
	ASSERT_EQ(in_.read8(1), 1);
	/// Start a move
	resetPosition();
	moveTo(500,500,0,2000);
	moveTo(500,0,0,2000);
	/// Should be busy
	reset();
	out_.append8(HOST_CMD_IS_FINISHED);
	runPacket();
	/// Should respond with a 0 (no, not done)
	ASSERT_EQ(in_.read8(1), 0);

	/// Abort moves
	//sleep(1);
	reset();
	out_.append8(HOST_CMD_ABORT);
	runPacket();

	reset();
	out_.append8(HOST_CMD_IS_FINISHED);
	runPacket();
	/// Should respond with a 1 (yes, we're done)
	ASSERT_EQ(in_.read8(1), 1);

	/// Return to zero point
	moveTo(0,0,0,2000);
	waitUntilDone();

}

TEST_F(SerialTest,NoSuchCommand)
{
	reset();
	out_.append8(0x75);
	writePacket();
	readPacketWithTimeout(50);
	ASSERT_TRUE(in_.isFinished());
	ASSERT_FALSE(in_.hasError());
	ASSERT_EQ(in_.read8(0), RC_CMD_UNSUPPORTED);
}

TEST_F(SerialTest,PauseUnpause)
{
	reset();
	waitUntilDone();
	reset();
	/// Start a move
	resetPosition();
	moveTo(0, 750, 0, 1000);
	moveTo(0, 0, 0, 1000);
	/// Pause after 1s
	sleep(1);
	reset();
	out_.append8(HOST_CMD_PAUSE);
	runPacket();
	/// Wait 1s, unpause
	sleep(1);
	reset();
	out_.append8(HOST_CMD_PAUSE);
	runPacket();

	waitUntilDone();
}

TEST_F(SerialTest,Delay)
{
	reset();
	waitUntilDone();
	reset();
	/// Start a move
	resetPosition();
	moveTo(500, 0, 0, 1000);
	delay(1.5); // delay for 1.5 seconds
	moveTo(0, 0, 0, 1000);
	waitUntilDone();
}

TEST_F(SerialTest,Circle)
{
	reset();
	waitUntilDone();
	reset();
	/// Start a move
	resetPosition();
	const float radius = 400.0;
	const int speed = 700;
	moveTo(0, radius, 0, speed);
	for (int i = 0; i < 400; i++) {
		double theta = i * M_PI / 50.0;
		moveTo(radius*sin(theta), radius*cos(theta),0,speed);
	}
	moveTo(0,0,0,speed);
	waitUntilDone();
}

TEST_F(SerialTest,Eeprom)
{
	// write 0 1 2 3 4 5 6
	const int pattern_len = 7;
	reset();
	out_.append8(HOST_CMD_WRITE_EEPROM);
	out_.append16(10);
	out_.append8(pattern_len);
	for (int i = 0; i < pattern_len; i++) {
		out_.append8(i);
	}
	runPacket();

	// read back and check
	reset();
	out_.append8(HOST_CMD_READ_EEPROM);
	out_.append16(10);
	out_.append8(pattern_len);
	runPacket();
	for (int i = 0; i < pattern_len; i++) {
		ASSERT_EQ(in_.read8(i+1), i);
	}

	// replace 2nd value with 0xff
	reset();
	out_.append8(HOST_CMD_WRITE_EEPROM);
	out_.append16(12);
	out_.append8(1);
	out_.append8(0xff);
	runPacket();

	// read back and check
	reset();
	out_.append8(HOST_CMD_READ_EEPROM);
	out_.append16(10);
	out_.append8(pattern_len);
	runPacket();
	for (int i = 0; i < pattern_len; i++) {
		if ( i != 2 ) {
			ASSERT_EQ(in_.read8(i+1), i);
		} else {
			ASSERT_EQ(in_.read8(i+1), 255);
		}
	}
}

TEST_F(SerialTest,Reset)
{
  // Reset device
  reset();
  out_.append8(HOST_CMD_RESET);
  runPacket();
}
