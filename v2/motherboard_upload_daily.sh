#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Usage: `basename $0` port=PORTNAME [platform=PLATFORM]"
  echo "(Valid platforms are rrmbv12, mb24)"
  exit 1
fi
scons -c -f SConstruct
BUILD_NAME=\\\"`date '+%Y.%m.%d'`\ `git show-ref --hash=20 HEAD`\\\" scons -f SConstruct $@ upload
