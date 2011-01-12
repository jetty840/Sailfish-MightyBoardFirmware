HOW TO USE THE SD_SLAMMER.

1) Build and upload.  I use the included makefile for this, but
   you should be able to use athe arduino environment if you
   swing that way.

2) Bring up a serial terminal and connect to the board at
   38400 bps.

3) Reset the board and insert an SD card.

4) Enter the size of the test in KB.  Please note that the
   sketch is going to wait for a newline, and by default arduino's
   serial monitor doesn't seem to send one... suck.  I just
   used minicom.

5) ???

6) Science.
