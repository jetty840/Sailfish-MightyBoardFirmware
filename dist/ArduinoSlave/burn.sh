#!/bin/bash

while true; do
    echo "Press ENTER to upload"
    read
    # Burn lock bits and fuses
    avrdude -v -pm168 -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xdd:m -Ulfuse:w:0xff:m 
    avrdude -v -pm168 -cusbtiny -Uflash:w:./ArduinoSlaveExtruder-v1.6+BL.hex:i -Ulock:w:0x0F:m
done


