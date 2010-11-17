scons -c -f SConstruct.extruder
VERSION=206 BUILD_NAME=\\\"Beta\ `date '+%Y.%m.%d'`\\\" scons -f SConstruct.extruder port=$1 upload
