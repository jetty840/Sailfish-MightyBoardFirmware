
This is the firmware for the MightyBoard 8U2 Chip. to flash this onto an 8u2, please use:

sudo avrdude -p at90usb82 -F -P usb -c avrispmkii -U flash:w:Makerbot-usbserial.hex -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m


