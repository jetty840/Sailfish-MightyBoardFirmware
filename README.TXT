*** NOTE ***
This document is mostly out-of-date.  It still should work to build the
firmware through the Arduino environment, but further doc is needed regarding
how to build the firmware without Arduino.
*** NOTE ***


This is the v1.3 release of the RepRap Generation 3 firmware.  It contains
firmware for the following boards:
* RepRap v1.1 Motherboard
* RepRap v1.2 Motherboard
* Extruder Controller v2.2

** INSTALLATION **

1) Install the Arduino software

If you haven't already, you should start by installing the Arduino software.
You can find the latest version for Windows, Mac, and Linux here:
http://arduino.cc/en/Main/Software

Follow the instructions on the Arduino site to install the software.

2) Install Sanguino support

The RepRep motherboard uses the Sanguino, an Arduino derivative.  You'll
need to add some Sanguino-specific bits to your Arduino installation
before you can program the motherboard.  Follow the directions for your
platform:

For Mac OS X:
http://sanguino.cc/softwareformac
For Windows:
http://sanguino.cc/softwareforwindows
For Linux:
http://sanguino.cc/softwareforlinux

3) Install the bootloaders

If you have pre-assembled RepRap motherboards or extruder controllers, or
are updating firmware you've installed before, you can skip these steps.  If
you've assembled your own board and are installing firmware for the first
time, you'll need an in-system AVR programmer, like the USBTinyISP.

3a) Install the bootloader on the RepRap Motherboard

* Use the 6-pin connector to attach the USBTinyISP to the header on the 
  motherboard.  
* Plug the USBTinyISP into your computer.
* Start the Arduino software.
* Select "Sanguino" from the Tools > Board menu.  (If "Sanguino" doesn't
  appear as an option, check that you've installed the Sanguino software
  correctly.)
* From the menu, select Tools > Burn Bootloader > w/USBTinyISP.  This
  will start the bootloader installation.  When it's complete, you can
  quit the Arduino software.

3b) Install the bootloader on the Extruder Controller

This step is identical to step 3a), with one important difference: the
Extruder Controller board is Arduino-based, not Sanguino-based, so you
will need to select "Arduino Duemilanove w/ATmega168" from the
Tools > Board menu instead of "Sanguino".

4) Copy the libraries

The firmware update requires you to install a couple of libraries.  Copy
all the folders in reprap-gen3-firmware-v1.x/libraries to 
arduino-00XX/hardware/libraries.  If you have older versions of the
libraries installed, replace them with the new version.

5) Install the RepRap Motherboard firmware

* Connect your USB-TTY cable to the RepRap motherboard and your computer's
  USB port.  Make sure that the colors on the cable match up with the
  ones marked on the board.
* Start the Arduino software.  Open the file:
  reprap-gen3-firmware-v1.x/SanguinoMaster/SanguinoMaster.pde
* Select "Sanguino" from the Tools > Board menu.
* Select your USB cable's serial port fronm the Tools > Serial Port menu.
* Click the Upload button. This is the trickiest step. The v1.1 and
  v1.2 motherboards require a manual reset when being
  reprogrammed. There's a small reset button on the motherboard; you
  should see some LEDs flicker when you press it. The trick is to get
  the timing right. If you can, start the arduino software from a
  terminal program and keep a close eye on the output. A few seconds
  after you hit the upload button on the arduino software, you will
  see a message in the terminal like this:

"Binary sketch size: NNNN bytes (of a NNNN byte maximum)"

  When you see that message, hit the reset button immediately.

  If you can't or don't know how to start arduino from a terminal, try
  hitting the reset button four seconds or so after you press the
  upload button in the arduino software. 

  Regardless of which method you use, you may need a few tries before
  you get it right. Don't panic if it doesn't work the first time! 

6) Install the Extruder Controller firmware

This step is exactly the same as Step 5, with two important
differences:
* Select "Arduino Duemilanove w/ATmega168" from the 
  Tools > Board menu instead of "Sanguino"
* Instead of SanguinoMaster, open this file:
  reprap-gen3-firmware-v1.x/ArduinoSlaveExtruder/ArduinoSlaveExtruder.pde 

7) Try it out!

For further information, check out the online documentation:
http://wiki.makerbot.com/cupcake-motherboard-firmware-update
http://wiki.makerbot.com/plastruder-firmware-update