#!/bin/bash

while true; do
    echo "Press ENTER to upload"
    read
    # Burn lock bits and fuses
    avrdude -v -pm644p -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0xFD:m -Uhfuse:w:0xDC:m -Ulfuse:w:0xFF:m
    avrdude -v -pm644p -cusbtiny -Uflash:w:./SanguinoMaster-v1.6+BL.hex:i -Ulock:w:0x0F:m 
done

