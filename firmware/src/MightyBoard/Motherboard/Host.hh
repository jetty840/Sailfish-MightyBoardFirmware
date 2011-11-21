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

// TODO: Make this a class.
/// Functions in the host namespace deal with communications to the host
/// computer. The host also implents a simple state machine, because it is
/// also responsable for handling prints from SD card.
namespace host {

const int MAX_MACHINE_NAME_LEN = 32;
const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;

/// The host can be in any of these four states.
enum HostState {
        HOST_STATE_READY            = 0,
        HOST_STATE_BUILDING         = 1,
	HOST_STATE_BUILDING_FROM_SD = 2,
        HOST_STATE_ERROR            = 3
};

/// Run the host slice. This function handles incoming packets and host resets.
void runHostSlice();

/// Returns the name of the current machine
/// \return Pointer to a character string containing the machine name.
char* getMachineName();

/// Returns the name of the current build, if any.
/// \return Pointer to a character string containing the build name.
char* getBuildName();

/// Returns the current host state. The host implements a very simple
/// state machine, which is used to determine what information should be
/// displayed on the interface board.
/// \return Current host state.
HostState getHostState();

/// Start a build from SD card. The build name should be set by overwriting
/// the value of buildName, provided by #getBuildName().
/// \return True if build started successfully.
sdcard::SdErrorCode startBuildFromSD();

/// Stop the current build
void stopBuild();

}

#endif // HOST_HH_
