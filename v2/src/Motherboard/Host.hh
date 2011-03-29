/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef HOST_HH_
#define HOST_HH_

#include "Packet.hh"
#include "SDCard.hh"

namespace host {

const int MAX_MACHINE_NAME_LEN = 32;
const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;

void runHostSlice();

enum HostState {
	HOST_STATE_READY = 0,
	HOST_STATE_BUILDING = 1,
	HOST_STATE_BUILDING_FROM_SD = 2,
	HOST_STATE_ERROR = 3,
};

// Returns the name of the current machine
char* getMachineName();

//Returns the name of the current build, if any.
char* getBuildName();

// Returns the current host state
HostState getHostState();

// Start a build from SD, if possible.
// Returns true if build started successfully, false otherwise
sdcard::SdErrorCode startBuildFromSD();

// Stop the current build, if any
void stopBuild();

}

#endif // HOST_HH_
