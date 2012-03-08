#!/bin/bash

cd $( dirname $( readlink -f "${BASH_SOURCE[0]}" ))

while true; do

FAIL8U2="8U2 Bootloader PASS"
FAIL1280="1280 Bootloader PASS"
FAILUSB="USB Program PASS"

    echo "Press Enter upload 8U2 firmware"
    read

    # Upload bootloader via isp
    avrdude -p at90usb82 -F -P usb -c avrispmkii -U flash:w:Makerbot-usbserial.hex -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

    if [ $? -ne 0 ]
    then
     FAIL8U2="8U2 Bootloader FAIL"
    fi

   echo "Press Enter upload 1280 firmware"
    read

    # Upload bootloader and motor test via isp
    avrdude -p m1280 -F -P usb -c avrispmkii -U flash:w:motor_test.hex -U lfuse:w:0xFF:m -U hfuse:w:0xDA:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

    if [ $? -ne 0 ]
     then
      FAIL1280="1280 Bootloader FAIL"
    # else
    #  sleep 10
    fi
 
   # Upload firmware via usb
   avrdude -p m1280 -P usb -c avrispmkii
   avrdude -F -V -p m1280 -P /dev/ttyACM0 -c stk500v1 -b 57600 -U flash:w:Mighty-mb40-v5.2.hex

   if [ $? -ne 0 ]
    then
       FAILUSB="USB Program FAIL"
    fi

	echo $FAIL8U2
	echo $FAIL1280
	echo $FAILUSB
done

#!/bin/bash



