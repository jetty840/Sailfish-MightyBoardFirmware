#!/bin/bash

BUILD="$1"
LOCALE="$2"


if [[ $BUILD == *2560* ]]
then
    BOOTLOADER=8192
    MAXSIZE=262144
else
    BOOTLOADER=4096
    MAXSIZE=131072
fi

TMPFILE=/tmp/mightyboard-awk-prog.tmp
echo "{ if ( \$1 != \"text\" ) print \$1+\$2+$BOOTLOADER }" > $TMPFILE
SIZE=`avr-size build/$BUILD/*$LOCALE.elf | awk -f $TMPFILE`
rm $TMPFILE

if [ $SIZE -gt $MAXSIZE ] ; then
    echo "**** size($BUILD)=$SIZE which exceeds $MAXSIZE bytes ****"
    exit 1
fi

echo "*** $BUILD $LOCALE is $SIZE bytes"
exit 0
