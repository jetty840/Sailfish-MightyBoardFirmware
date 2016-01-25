#!/bin/bash

BUILD="$1"
LOCALE="$2"

OS=`uname -s`

if [[ $OS == Darwin ]]
then
	AWK_PROG=
else
	AWK_PROG="-e"
fi

if [[ $BUILD == *2560* ]] || [[ $BUILD == *azteeg* ]]
then
    BOOTLOADER=8192
    MAXSIZE=262144
else
    BOOTLOADER=4096
    MAXSIZE=131072
fi

for FILE in build/${BUILD}/*${LOCALE}.elf
do
	SIZE=$(avr-size ${FILE} | awk $AWK_PROG "{ if ( \$1 != \"text\" ) print \$1+\$2+${BOOTLOADER} }")

	if [ $SIZE -gt $MAXSIZE ]
	then
	    echo "**** size($BUILD)=$SIZE which exceeds $MAXSIZE bytes ****"
	    exit 1
	fi

	echo "*** $FILE is $SIZE / $MAXSIZE bytes"
done
exit 0
