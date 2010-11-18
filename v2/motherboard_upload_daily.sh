#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Usage: `basename $0` port=PORTNAME [platform=PLATFORM]"
  echo "(Valid platforms are rrmbv12, rrmbv24)"
  exit 1
fi
scons -c -f SConstruct
VERSION=204 BUILD_NAME=\\\"Beta\ `date '+%Y.%m.%d'`\\\" scons -f SConstruct $@ upload
