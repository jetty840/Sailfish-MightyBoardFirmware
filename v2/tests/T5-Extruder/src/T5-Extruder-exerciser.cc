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

#include <vector>

#include "Packet.hh"
#include "Commands.hh"

using namespace std;

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

	void makeToolQuery(uint8_t code, vector<uint8_t>& payload);
	void makeToolCommand(uint8_t code, vector<uint8_t>& payload);

	void writePacket();
	void readPacketWithTimeout(uint16_t timeout);

	void runPacket();
	void runCommandPacket();
	void reset();

	void setTemperature(uint16_t celsius);
	uint16_t getTemperature();
	void waitForTool();
};

void SerialTest::makeToolQuery(uint8_t code, vector<uint8_t>& payload) {
	reset();
	out_.append8(HOST_CMD_TOOL_QUERY);
	out_.append8(0);
	out_.append8(code);
	for (int i = 0; i < payload.size(); i++) {
		out_.append8(payload[i]);
	}
}

void SerialTest::makeToolCommand(uint8_t code, vector<uint8_t>& payload) {
	reset();
	out_.append8(HOST_CMD_TOOL_COMMAND);
	out_.append8(0);
	out_.append8(code);
	out_.append8(payload.size());
	for (int i = 0; i < payload.size(); i++) {
		out_.append8(payload[i]);
	}
}

uint16_t SerialTest::getTemperature() {
	vector<uint8_t> payload;
	makeToolQuery(SLAVE_CMD_GET_TEMP,payload);
	runPacket();
	EXPECT_EQ(3,in_.getLength());
	return in_.read16(1);
}

void SerialTest::setTemperature(uint16_t celsius) {
	vector<uint8_t> payload;
	payload.push_back(celsius&0xff);
	payload.push_back((celsius>>8)&0xff);
	makeToolCommand(SLAVE_CMD_SET_TEMP,payload);
	runCommandPacket();
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

void SerialTest::runCommandPacket() {
	bool done = false;
	while (!done) {
		writePacket();
		readPacketWithTimeout(50);
		ASSERT_TRUE(in_.isFinished());
		ASSERT_FALSE(in_.hasError());
		ASSERT_EQ(in_.read8(0), RC_OK);
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

void SerialTest::reset() {
	in_.reset();
	out_.reset();
}

TEST_F(SerialTest,CheckTemperature)
{
	uint16_t temp = getTemperature();
	ASSERT_GE(temp,5);
	ASSERT_LT(temp,250);
	cout << "Temperature: " << temp << endl;
}

TEST_F(SerialTest,SetTemperature)
{
	setTemperature(0);
	uint16_t start_temp = getTemperature();
	while (start_temp > 90) {
		cout << "Cooldown in progress." << endl;
		sleep(3);
		start_temp = getTemperature();
	}
	cout << "Temperature at start: " << start_temp << endl;
	bool done = false;
	setTemperature(120);
	int waited = 0;
	while (!done) {
		sleep(3); waited += 3;
		uint16_t temp = getTemperature();
		cout << "Temperature: " << temp << endl;
		if (waited > 20) ASSERT_GE(temp,start_temp);
		if (temp > 120) done = true;
	}
	setTemperature(0);
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
/*
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
*/

