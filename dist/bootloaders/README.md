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
