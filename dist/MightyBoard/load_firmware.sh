#!/bin/bash

FAIL8U2="8U2 Bootloader PASS"
FAIL1280="1280 Bootloader PASS"
FAILUSB="USB Program PASS"

    # Upload bootloader via isp
    avrdude -p at90usb82 -F -P usb -c usbtiny -U flash:w:Makerbot-usbserial.hex -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

    if [ $? -ne 0 ]
    then
     FAIL8U2="8U2 Bootloader FAIL"
    fi

    # Upload bootloader via isp
    avrdude -p m1280 -F -P usb -c avrispmkii -U flash:w:motor_test.hex -U lfuse:w:0xFF:m -U hfuse:w:0xDA:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

    if [ $? -ne 0 ]
     then
      FAIL1280="1280 Bootloader FAIL"
    fi
  
   sleep 10   
 
   # Upload bootloader via isp
   avrdude -p m1280 -P usb -c avrispmkii
   avrdude -F -V -p m1280 -P /dev/ttyACM0 -c stk500v1 -b 57600 -U flash:w:MightyBoard.hex

   if [ $? -ne 0 ]
    then
       FAILUSB="USB Program FAIL"
    fi

	echo $FAIL8U2
	echo $FAIL1280
	echo $FAILUSB

#!/bin/bash



