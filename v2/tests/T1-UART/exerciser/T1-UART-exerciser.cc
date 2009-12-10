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

#include "util/Packet.hh"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int main()
{
	int fd,c, res;
	struct termios oldtio,newtio;
	struct sigaction saio; /* definition of signal action */
	char buf[255];

	/* open the device to be non-blocking (read will return immediately) */
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd <0) {perror(MODEMDEVICE); return -1;}

	tcgetattr(fd,&oldtio); /* save current port settings */
	/* set new port settings for canonical input processing */
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;

	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	InPacket in;
	OutPacket out;

	uint16_t sequence_number = 0;
	long long bytes_txd = 0;
	// can't do 32, since we drop the debug command byte
	while(1) {
		uint8_t length_in_bytes = random()%31;
		sequence_number++;
		// write a packet
		in.reset();
		out.reset();
		out.append8(0x70); // echo test
		for (int i =0; i < length_in_bytes; i++) {
			uint8_t val = (i%2 == 0)?(sequence_number & 0xff):(sequence_number>>8);
			out.append8(val);
			bytes_txd++;
		}
		bytes_txd += 4;
		while (!out.isFinished()) {
			uint8_t out_byte = out.getNextByteToSend();
			write(fd,&out_byte,1);
		}
//		printf("Finished sending packet SN %d, length %d\n", sequence_number,length_in_bytes);
		while (!in.isFinished()) {
			uint8_t in_byte;
			int count = read(fd,&in_byte,1);
			if (count == 1) {
				in.processByte(in_byte);
			}
		}
//		printf("Received packet SN %d\n", sequence_number);
		// Verify
		if (in.hasError()) {
			fprintf(stderr,"%d - PACKET ERROR (error code %d)\n",sequence_number,in.getErrorCode());
		}
		if (!in.getLength() == length_in_bytes) {
			fprintf(stderr,"%d - LENGTH MISMATCH (%d, expected %d)\n",sequence_number, in.getLength(), length_in_bytes);
		}
		for (int i =0; i < length_in_bytes; i++) {
			uint8_t val = (i%2 == 0)?(sequence_number & 0xff):(sequence_number>>8);
			if (in.read8(i) != val) {
				fprintf(stderr,"%d - DATA MISMATCH on index %d (%d, expected %d)\n",sequence_number, i, in.read8(i), val);
			}
		}
		if (sequence_number % 100 == 0) {
			printf("Packet sequence: %u (%llu bytes transmitted)\n", sequence_number, bytes_txd);
			fflush(stdout);
		}
	}

	/* restore old port settings */
	tcsetattr(fd,TCSANOW,&oldtio);
}
