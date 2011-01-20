#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Usage: `basename $0` port=PORTNAME [platform=PLATFORM]"
  echo "(Valid platforms are ecv22, ecv34, ec36)"
  exit 1
fi
scons -c -f SConstruct.extruder
BUILD_NAME=\\\"`date '+%Y%m%d'`\ `git show-ref --hash=20 HEAD`\\\" scons -f SConstruct.extruder $@ upload
