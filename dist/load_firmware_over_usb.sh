#!/bin/bash
#
# Flash a firmware .hex file over the printer's USB connection.
#
# Usage: load_bootloader_over_usb [-P port] [-p "m2560" | "m1280"] hex-file
#
#   -P -- device port (e.g., /dev/ttyACM0)
#   -p -- AVR device type, m2560 or m1280
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
AVRFLAGS="-v -D"
SPEED=57600

# Our name for when reporting usage()
NAME=$(basename "$0")

# Defaults for command line options
SRC=
PORT=
ARCH="m1280"

# Display usage and exit with an error
function usage {
	echo "Usage: $NAME [-p \"m2560\" | \"m1280\"] -P port hex-file"
	echo "  -p m1280 is assumed when -p is not specified"
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
while getopts "p:P:" opt
do
	case $opt in
		p) ARCH="$OPTARG" ;;
		P) PORT="$OPTARG" ;;
		\?) usage ;;
	esac
done

# The hex file
shift $((OPTIND-1))
SRC="$1"

# Check the port
if [ -z "$PORT" ] ;
then
	>&2 echo "USB device/port name must be specified with -P"
	usage
fi

if [ -z "$SRC" ] ;
then
	>&2 echo "Firmware file name must be specified"
	usage
fi

set -x

$AVRDUDE -c stk500v2 -P "$PORT" -p "$ARCH" -b "$SPEED" $AVRFLAGS \
	"-Uflash:w:$SRC:i"
