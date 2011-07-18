#!/bin/bash

MOTHERBOARD_SCONS=SConstruct
EXTRUDER_SCONS=SConstruct.extruder

MOTHERBOARD_PLATFORMS=( rrmbv12 mcv10 mb24 mb24-2560 )
EXTRUDER_PLATFORMS=( ecv22 ecv34 )

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




echo Building all firmware

echo "Building all firmware" > ${LOG_FILE}

build_firmware MOTHERBOARD_PLATFORMS ${MOTHERBOARD_SCONS}
build_firmware EXTRUDER_PLATFORMS ${EXTRUDER_SCONS}
