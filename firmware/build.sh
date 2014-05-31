#!/bin/sh

FWDIR=`pwd`
DISTDIR=
REPGDIR=$HOME/.replicatorg/firmware

if [ -f $HOME/.makerbot_config_build.sh ] ; then
    . $HOME/.makerbot_config_build.sh
    if [ $FWDIR_MB ] ; then
	FWDIR="$FWDIR_MB"
    fi
fi

if [ -d ../../../../branches ] ; then
  SVN=`svnversion ../../../.. | awk -f $FWDIR/svnversion.awk`
else
  SVN=`svnversion $FWDIR | awk -f $FWDIR/svnversion.awk`
fi

VER=`awk -F'.' '{printf("%d.%d.%d",$1,$2,$3); exit}' $FWDIR/current_version.txt`

for BUILD in "mighty_one" "mighty_one-corexy" "mighty_one-2560" "mighty_one-2560-max31855" "mighty_one-2560-corexy" "mighty_two" "mighty_two-2560" "mighty_twox" "mighty_twox-2560" "ff_creator" "ff_creator-2560" "ff_creatorx-2560" "wanhao_dup4"
do

    MAX31855=""
    BMAX31855=""
    COREXY=""
    BCOREXY=""

    if [ "$BUILD" = "mighty_one-2560-max31855" ] ; then
	BUILD="mighty_one-2560"
	MAX31855="max31855=1"
	BMAX31855="-max31855"
    fi

    if [ "$BUILD" = "mighty_one-2560-corexy" ] ; then
	BUILD="mighty_one-2560"
	COREXY="core_xy=1"
	BCOREXY="-corexy"
    fi

    if [ "$BUILD" = "mighty_one-corexy" ] ; then
	BUILD="mighty_one"
	COREXY="core_xy=1"
	BCOREXY="-corexy"
    fi

    rm -rf build/$BUILD

    for LOCALE in "en" "de" "fr"
    do
        scons platform=$BUILD $MAX31855 $COREXY locale=$LOCALE
        ./checksize.sh $BUILD .$LOCALE
        if [ $? -ne 0 ]; then
	        exit 1
        fi

        if [ $DISTDIR ] ; then
            mkdir -p $DISTDIR/$LOCALE
            cp build/$BUILD/*.$LOCALE.hex  $DISTDIR/$LOCALE/$BUILD$BMAX31855$BCOREXY-Sailfish-v${VER}-r${SVN}.hex
        fi
    done

    # only english to repgdir
    if [ $REPGDIR ] ; then
	    cp build/$BUILD/*.en.hex  $REPGDIR/$BUILD$BMAX31855$BCOREXY-Sailfish-v${VER}-r${SVN}.hex
    fi

    # show sizes of each build
    avr-size build/$BUILD/*.elf


    rm -rf build/$BUILD/MightyBoard

    for LOCALE in "en" "de" "fr"
    do
        scons platform=$BUILD broken_sd=1 $MAX31855 $COREXY locale=$LOCALE
        ./checksize.sh $BUILD b.$LOCALE
        if [ $? -ne 0 ]; then
	        exit 1
        fi

        if [ $DISTDIR ] ; then
            mkdir -p $DISTDIR/$LOCALE
            cp build/$BUILD/*b.$LOCALE.hex  $DISTDIR/$LOCALE/$BUILD$BMAX31855$BCOREXY-Sailfish-v${VER}-r${SVN}b.hex
        fi
    done

    if [ $REPGDIR ] ; then
	    cp build/$BUILD/*b.en.hex  $REPGDIR/$BUILD$BMAX31855$BCOREXY-Sailfish-v${VER}-r${SVN}b.hex
    fi

    # show sizes of each build
    avr-size build/$BUILD/*.elf

done
