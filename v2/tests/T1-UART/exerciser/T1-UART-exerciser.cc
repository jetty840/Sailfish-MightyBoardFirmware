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
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (fd <0) {perror(MODEMDEVICE); return -1;}

	tcgetattr(fd,&oldtio); /* save current port settings */
	/* set new port settings for canonical input processing */
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
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
	// can't do 32, since we drop the debug command byte
	uint8_t length_in_bytes = random()%31;
	while(1) {
		sequence_number++;
		// write a packet
		in.reset();
		out.reset();
		out.append8(0x70); // echo test
		bool low = true;
		for (int i =0; i < length_in_bytes; i++) {
			out.append8(low?(sequence_number&0xff):(sequence_number>>8));
		}
		while (!out.isFinished()) {
			uint8_t out_byte = out.getNextByteToSend();
			write(fd,&out_byte,1);
		}
		printf("Finished sending packet SN %d, length %d\n", sequence_number,length_in_bytes);
		while (!in.isFinished()) {
			uint8_t in_byte;
			int count = read(fd,&in_byte,1);
			if (count == 1) {
				in.processByte(in_byte);
			}
		}
		printf("Received packet SN %d\n", sequence_number);

	}

	/* restore old port settings */
	tcsetattr(fd,TCSANOW,&oldtio);
}
