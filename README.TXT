This is the firmware source for Makerbot MightyBoard

If you're getting started, please begin by reading the documentation for
the firmware:
http://wiki.makerbot.com/v2-firmware

This project contains the following subdirectories:
* firmware - this is the firmware, where active development takes place.
       See http://wiki.makerbot.com/v2-firmware for details on how to
       build and install the code.
* bootloader - this contains the bootloaders for the MCUs on the Mightyboard
* dist - this directory contains tools and scripts for quickly installing
       bootloaders and board images for the MightyBoard
* tests - this directory contains tests using the s3g driver and development testing data
* docs - info about command handling and firmware details

N.B. to generate the french version of the firmware, add the LOCALE=fr env
variable before compiling

To use the tests in s3g_tests, you must initialize the s3g submodule by running
git submodule init
git submodule update

