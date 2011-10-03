#!/bin/bash

SCONS=SConstruct

PLATFORMS=( rrmbv12 mb24 mb24-2560 ecv22 ecv34 )

LOG_FILE=build_all_output


function build_firmware {
    platform_list_name="$1[*]"
    platform_list=(${!platform_list_name})
    scons_file=$2

    for platform in ${platform_list[@]}
    do
        echo -n "Building firmware for ${platform}... "

        echo -e "\n\n\n\n" >> ${LOG_FILE}
    echo Building firmware for ${platform} >> ${LOG_FILE}

    scons -f ${scons_file} platform=${platform} >> ${LOG_FILE} 2>&1

    if [ "$?" -ne "0" ]; then
        echo Failure
    else
	echo Success
    fi
done


}


function build_documentation {
    echo -n "Building documentation..."

    echo Building documentation >> ${LOG_FILE}
    echo -e "\n\n\n\n" >> ${LOG_FILE}
    doxygen G3Firmware.doxyfile >> ${LOG_FILE} 2>&1
    
    if [ "$?" -ne "0" ]; then
        echo Failure
    else
	echo Success
    fi
}



echo Building all firmware
echo "Building all firmware" > ${LOG_FILE}

build_firmware PLATFORMS ${SCONS}



build_documentation
