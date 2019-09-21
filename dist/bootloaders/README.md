### ATmega bootloaders for use with MightyBoard firmwares.

ATmega 1280 -- `ATmegaBOOT_168_atmega1280.hex`

* This is the standard ATmega 1280 bootloader.

ATmega 2560 -- `stk500boot_v2_mega2560.hex`

* This is a special build of the standard ATmega 2560 bootloader intended to
  accept firmware downloads at 57.6 KBaud.  The USB firmware, when it sees a
  speed of 57.6 KBaud asserted, will attempt to put the ATmega firmware download
  mode.  Thus, the bootloader must be built to accept that speed.  By default
  the standard 1280 bootloader does, but the standard 2560 bootloader does not.

* See the [MightyBoardFirmware-2560-bootloader github repository for further
  details](https://github.com/dcnewman/MightyBoardFirmware-2560-bootloader).

Optiboot

* The Optiboot bootloader is an alternate extra small bootloader.

* This Bootloader is only 602 Bytes on 1280 and 664 Bytes on 2560 long and so
  it is extremly limited. But it allows for reading and writing EEPROM and
  reading and burning the Flash except for the Bootloader itself. But absolutly
  nothing more is needed in a 3D Printer. So we have full 3KB more space for
  the Main Programm.

* It is MANDATORY to set the H-Fuse to 0xDE for this bootloader to
  operate correctly.

* As an extra this Bootloader has an extra long (20sec) Bootloader
  wait on restart using the RESET Button to make it easier to get
  updates flashed. This extra delay is NOT applied on PowerOn.

ATmega 1280 -- `optiboot_atmega1280.hex`

* This is the Optiboot ATmega 1280 bootloader.

ATmega 2560 -- `optiboot_atmega2560.hex`

* This is a special build of the Optiboot bootloader to support ATmega 2560
