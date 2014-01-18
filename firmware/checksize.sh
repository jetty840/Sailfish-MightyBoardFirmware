#!/bin/sh

BUILD="$1"

BOOTLOADER=4096

if [ "$BUILD" = "mighty_one" -o "$BUILD" = "mighty_two" -o \
     "$BUILD" = "mighty_twox" -o "$BUILD" = "ff_creator" -o \
     "$BUILD" = "wanhao_dup4" ] ; then
    MAXSIZE=131072
elif [ "$BUILD" = "mighty_one-2560" -o "$BUILD" = "mighty_two-2560" ] ; then
    MAXSIZE=262144
else
    echo "**** Invalid board $BUILD specified; must be mighty_one, mighty_one-2560 ****"
    echo "**** mighty_two, or mighty_twox ****"
    exit 1
fi

TMPFILE=/tmp/mightyboard-awk-prog.tmp
echo "{ if ( \$1 != \"text\" ) print \$1+\$2+$BOOTLOADER }" > $TMPFILE
SIZE=`avr-size build/$BUILD/*.elf | awk -f $TMPFILE`
rm $TMPFILE

if [ $SIZE -gt $MAXSIZE ] ; then
    echo "**** size($BUILD)=$SIZE which exceeds $MAXSIZE bytes ****"
    exit 1
fi

echo "*** $BUILD is $SIZE bytes"
exit 0
