#!/bin/sh

FWDIR=`pwd`
DISTDIR=./dist
REPGDIR=$HOME/.replicatorg/firmware

if [ -f $HOME/.makerbot_config_build.sh ] ; then
    . $HOME/.makerbot_config_build.sh
    if [ $FWDIR_MB ] ; then
	FWDIR="$FWDIR_MB"
    fi
fi

SVN=`git log --oneline | wc -l`

VER=`awk -F'.' '{printf("%d.%d.%d",$1,$2,$3); exit}' $FWDIR/current_version.txt`
SCONS="scons -j${JOBS:-4}"

for BUILD in `python src/platforms.py`
do

    rm -rf build/$BUILD

    for LOCALE in "en"
    do
        $SCONS platform=$BUILD locale=$LOCALE
        ./checksize.sh $BUILD .$LOCALE
        if [ $? -ne 0 ]; then
	        exit 1
        fi

        if [ $DISTDIR ] ; then
            mkdir -p $DISTDIR/$LOCALE
            cp build/$BUILD/*.$LOCALE.hex  $DISTDIR/$LOCALE/$BUILD-Sailfish-v${VER}-r${SVN}.hex
        fi
    done

    # show sizes of each build
    avr-size build/$BUILD/*.elf

    rm -rf build/$BUILD/MightyBoard

    for LOCALE in "en"
    do
        $SCONS platform=$BUILD broken_sd=1 locale=$LOCALE
        ./checksize.sh $BUILD b.$LOCALE
        if [ $? -ne 0 ]; then
	        exit 1
        fi

        if [ $DISTDIR ] ; then
            mkdir -p $DISTDIR/$LOCALE
            cp build/$BUILD/*b.$LOCALE.hex  $DISTDIR/$LOCALE/$BUILD-Sailfish-v${VER}-r${SVN}b.hex
        fi
    done

    # show sizes of each build
    avr-size build/$BUILD/*.elf

done
