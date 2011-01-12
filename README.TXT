This is the firmware source for the RepRap Generation 3 and MakerBot 
Generation 4 board sets.

If you're getting started, please begin by reading the documentation for
the v2 firmware:
http://wiki.makerbot.com/v2-firmware

This project contains the following subdirectories:
* v2 - this is the v2 firmware, where active development takes place.
       See http://wiki.makerbot.com/v2-firmware for details on how to
       build and install the code.
* v1 - this is the old v1 firmware, which requires the Arduino libraries.
       For more information about building and installing this firmware,
       see the document v1/README.TXT.
* bootloader - this contains a slightly modified version of the Arduino
       bootloader for installing on older extruder controller boards.  It
       includes code to quickly shut down all mosfets to avoid possible
       damage during bootloader startup.
* dist - this directory contains tools and scripts for quickly installing
       bootloaders and board images on Piles O' Boards.
