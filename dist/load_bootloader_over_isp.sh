#!/bin/bash
#
# Load the proper ATmega 1280 or 2560 bootloader to the ATmega
# using the MightyBoard's ISP programming port.  The port is usually
# labelled as "1280" regardless of whether a 1280 or 2560 chip is
# placed on the board.
#
# Usage: load_bootloader_over_isp -c programmer -p device -P port
#
#   -c -- ISP programmer type
#   -p -- AVR device type, m2560 or m1280
#   -P -- device port (e.g., /dev/ttyACM0)
#
# Option/switch names as per avrdude
#
# Notes
#
#   1. avrdude must be found in PATH.
#
#   2. since ATmega 1280 and 2560s have been around for a while, it is
#        assumed that the default avrdude.conf file is sufficient.  Thus
#        avrdude's -C switch is not used here.

# Change as necessary
AVRDUDE=avrdude
AVRFLAGS="-v"
SPEED=57600

# Our name for when reporting usage()
NAME=$(basename "$0")
DIRPATH=$(dirname "$0")

# Defaults for command line options
PORT=
PROG=
ARCH=

# Display usage and exit with an error
function usage {
	echo "Usage: $NAME -P port -p device"
	exit 1
}

# Check to make sure that avrdude is in path
# We assume that if it is there then it is executable

CHECK=$(which "$AVRDUDE")
if [ ! -x "$CHECK" ] ;
then
	>&2 echo "An executable avrdude was not found in PATH"
	exit 1
fi

# Parse command line switches.  Switch names follow those used by avrdude
while getopts "c:p:P:" opt
do
	case $opt in
		c) PROG="$OPTARG" ;;
		p) ARCH="$OPTARG" ;;
		P) PORT="$OPTARG" ;;
		\?) usage ;;
	esac
done

# Check that a programmer was specified
if [ -z "$PROG" ] ;
then
	>&2 echo "ISP progammer type not specified with -c"
	usage
fi

# Check that a port was specified
if [ -z "$PORT" ] ;
then
	>&2 echo "USB device/port name must be specified with -P"
	usage
fi

if [ "$ARCH" = "m1280" ] ;
then
	SRC="$DIRPATH/bootloaders/ATmegaBOOT_168_atmega1280.hex"
elif [ "$ARCH" = "m2560" ] ;
then
	SRC="$DIRPATH/bootloaders/stk500boot_v2_mega2560.hex"
else
	>&2 echo "Unsupported device type, \"$ARCH\" specified; must be either m1280 or m2560"
	usage
fi

if [ ! -f "$SRC" ] ;
then
	>&2 echo "Bootloader file not found, \"$SRC\""
	exit 1
fi

set -x

$AVRDUDE -c "$PROG" -P "$PORT" -p "$ARCH" -b "$SPEED" $AVRFLAGS \
	"-Uflash:w:$SRC:i"
