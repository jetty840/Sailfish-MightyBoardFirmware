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
	
	enum {
	FILAMENT_RIGHT_FOR = 0,
	FILAMENT_LEFT_FOR = 1,
    FILAMENT_RIGHT_REV = 2,
    FILAMENT_LEFT_REV = 3,
	HOME_AXES = 4,
	LEVEL_PLATE_STARTUP = 5,
	FILAMENT_STARTUP_SINGLE = 6,
	FILAMENT_STARTUP_DUAL = 7
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
  void setScriptOffset(int offset);

};

#endif
