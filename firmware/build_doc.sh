#! /bin/sh

# you need to install doxygen and Graphviz


# Common color helpers
RED='\033[01;31m'   # bold red
GREEN='\033[01;32m' # green
RESET='\033[0m'     # Text reset


LOG_FILE="./build_doc.log"
DOXYGEN_CONF="./MightyBoardFirmware.doxyfile"

 
cd ..

rm -f ${LOG_FILE}
echo -n " - Running Doxygen... "

echo "Running Doxygen" >> ${LOG_FILE}
echo -e "\n\n\n\n" >> ${LOG_FILE}
doxygen ${DOXYGEN_CONF} >> ${LOG_FILE} 2>&1

if [ "$?" -ne "0" ]; then
	echo -e ${RED}"Failure"${RESET}
else
	echo -e ${GREEN}"Success"${RESET}
fi
