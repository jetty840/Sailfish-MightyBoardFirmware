#!/bin/bash

MAC_TOOLS_HOME=/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr

# Look for avrdude
if [ ! $AVRDUDE ]; then
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

FIRMWARE=MB-mb24-v2.9
FWDIR=`dirname $0`

while true; do
    echo "Press ENTER to upload $FIRMWARE"
    read
    if [ $AD_CONF ]; then
	CONF_FLAGS="-C $AD_CONF "
    fi
    # Upload firmware
    $AVRDUDE $CONF_FLAGS -cstk500v1 -P/dev/ttyUSB0 -b57600 -v -v -pm1280 -P/dev/ttyUSB0 -Uflash:w:${FWDIR}/${FIRMWARE}.hex:i 
done

#!/bin/bash

while true; do
    echo "Press ENTER to upload"
    read
    # Burn lock bits and fuses
done


