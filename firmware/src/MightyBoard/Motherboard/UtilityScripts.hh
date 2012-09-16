/*
 * Copyright 2012 by Alison Leonard <alison@makerbot.com>
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

#ifndef UTILITY_SCRIPTS
#define UTILITY_SCRIPTS

#include "Types.hh"

namespace utility {
	
	enum ScriptName{
		HOME_AXES = 0,
	LEVEL_PLATE_STARTUP = 1,
	TOOLHEAD_CALIBRATE = 2,
	LEVEL_PLATE_SECOND
	};
	
 /// returns true if script is running
 bool isPlaying();
 
 /// returns true if more bytes are available in the script
 bool playbackHasNext();
 
 /// gets next byte in script
 uint8_t playbackNext();
 
 /// begin buffer playback
 bool startPlayback(uint8_t build);
 
 /// updates state to finished playback
 void finishPlayback();
 
 /// reset pointers
 void reset();

 /// change the build index of the script
 void getSecondLevelOffset(void);

 /// return true if we should show the monitor screen during the utility script
 bool showMonitor();
};

#endif
