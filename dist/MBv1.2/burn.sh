#!/bin/bash

MAC_TOOLS_HOME=/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr

# Look for avrdude
if [ ! $AVRDUDE]; then
    if [ `which avrdude` ]; then
	echo "Using default avrdude installed on system."
	AVRDUDE=`which avrdude`
    elif [ -a ${MAC_TOOLS_HOME}/bin/avrdude ]; then
	echo "Found an avrdude installation in the default Mac Arduino location."
	AVRDUDE=${MAC_TOOLS_HOME}/bin/avrdude
	AD_CONF=${MAC_TOOLS_HOME}/etc/avrdude.conf
    else
	echo "Couldn't find a valid AVRDUDE installation.  Try setting the"
	echo "AVRDUDE environment variable to the location of your AVRDUDE"
	echo "installation.  You may also need to set the AD_CONF variable"
	echo "to the location of your avrdude.conf file, if your installation"
	echo "of AVRDUDE doesn't support USBTinyISP out of the box."
	exit 1
    fi
fi

FIRMWARE=PRODUCTION


while true; do
    echo "Press ENTER to upload $FIRMWARE"
    read
    if [ $AD_CONF ]; then
	CONF_FLAGS="-C $AD_CONF "
    fi
    # Burn lock bits and fuses
    $AVRDUDE $CONF_FLAGS -v -b38400 -pm644p -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0xFD:m -Uhfuse:w:0xDC:m -Ulfuse:w:0xFF:m
    # Burn firmware
    $AVRDUDE $CONF_FLAGS -v -b38400 -B1.0 -pm644p -cusbtiny -Uflash:w:./${FIRMWARE}.hex:i -Ulock:w:0x0F:m
done

