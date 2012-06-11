#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <termios.h>

void setDTR(int fd, int high) {
  unsigned int result = 0;
  ioctl( fd, TIOCMGET, &result );
  if (high) result |= TIOCM_DTR;
  else result &= ~TIOCM_DTR;
  ioctl( fd, TIOCMSET, &result );
  printf( "setDTR( %i )\n", high );
}

void main(int argc, char** argv) {
  unsigned long i = 0;
  int fd;
  if (argv[1] != 0) {
    fd = open(argv[1], O_RDWR || O_NONBLOCK);
  } else {
    fd = open("/dev/ttyUSB0", O_RDWR || O_NONBLOCK);
  }
  printf("opened: %d\n",fd);
  // adjust serial communuication parameters
  struct termios ComParams;
  tcgetattr(fd, &ComParams);
  ComParams.c_cflag &= ~CBAUD; // baud rate = 9600 bd
  ComParams.c_cflag |= B57600;
  tcsetattr( fd, TCSANOW, &ComParams );
  
  // play with RTS & DTR
  int iFlags;


  
  // turn on RTS
  iFlags = TIOCM_RTS;
  ioctl(fd, TIOCMBIS, &iFlags);
  // turn off RTS
  iFlags = TIOCM_RTS;
  ioctl(fd, TIOCMBIC, &iFlags);

  // turn on DTR
  iFlags = TIOCM_DTR;
  ioctl(fd, TIOCMBIS, &iFlags);
    
  
	sleep(1);
// turn off DTR
  iFlags = TIOCM_DTR;
  ioctl(fd, TIOCMBIC, &iFlags);

  close(fd);
}
