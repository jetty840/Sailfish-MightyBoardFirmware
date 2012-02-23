#!/bin/bash


# read firmware + bootloader from atmega1280
avrdude  -v -p m1280 -c avrispmkii -b57600 -U flash:r:Mighty_ATMega1280.hex:i -P usb
