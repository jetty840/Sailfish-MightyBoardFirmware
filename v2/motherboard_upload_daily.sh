scons -c -f SConstruct
VERSION=204 BUILD_NAME=\\\"Beta\ `date '+%Y.%m.%d'`\\\" scons -f SConstruct port=$1 upload
