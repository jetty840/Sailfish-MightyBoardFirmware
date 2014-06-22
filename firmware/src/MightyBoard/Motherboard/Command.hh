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

#ifndef COMMAND_HH_
#define COMMAND_HH_

#include <stdint.h>
#include "Configuration.hh"


//Pause states are used internally to determine various scenarios, so the 
//numbers here are important.
//The high 2 bits denote if it's a pause entry or pause exit command
//The first state that is execeuted on entering or exiting pause is 
//PAUSE_STATE_ENTER_COMMAND / PAUSE_STATE_EXIT_COMMAND
#define PAUSE_STATE_OTHER_COMMAND	0x00
#define PAUSE_STATE_ENTER_COMMAND	0x40
#define PAUSE_STATE_EXIT_COMMAND	0x80
#define PAUSE_STATE_ERROR_COMMAND       0x20

enum PauseState {
	//Other states
	PAUSE_STATE_NONE				= PAUSE_STATE_OTHER_COMMAND,
	PAUSE_STATE_PAUSED				= PAUSE_STATE_OTHER_COMMAND + 1,

	//Pause enter states
	PAUSE_STATE_ENTER_START_PIPELINE_DRAIN		= PAUSE_STATE_ENTER_COMMAND,
	PAUSE_STATE_ENTER_WAIT_PIPELINE_DRAIN		= PAUSE_STATE_ENTER_COMMAND + 1,
	PAUSE_STATE_ENTER_START_RETRACT_FILAMENT	= PAUSE_STATE_ENTER_COMMAND + 2,
	PAUSE_STATE_ENTER_WAIT_RETRACT_FILAMENT		= PAUSE_STATE_ENTER_COMMAND + 3,
	PAUSE_STATE_ENTER_START_CLEARING_PLATFORM	= PAUSE_STATE_ENTER_COMMAND + 4,
	PAUSE_STATE_ENTER_WAIT_CLEARING_PLATFORM	= PAUSE_STATE_ENTER_COMMAND + 5,
	
	//Pause exit states
	PAUSE_STATE_EXIT_START_PLATFORM_HEATER		= PAUSE_STATE_EXIT_COMMAND,
	PAUSE_STATE_EXIT_WAIT_FOR_PLATFORM_HEATER	= PAUSE_STATE_EXIT_COMMAND  + 1,
	PAUSE_STATE_EXIT_START_TOOLHEAD_HEATERS		= PAUSE_STATE_EXIT_COMMAND  + 2,
	PAUSE_STATE_EXIT_WAIT_FOR_TOOLHEAD_HEATERS	= PAUSE_STATE_EXIT_COMMAND  + 3,
	PAUSE_STATE_EXIT_START_RETURNING_PLATFORM	= PAUSE_STATE_EXIT_COMMAND  + 4,
	PAUSE_STATE_EXIT_WAIT_RETURNING_PLATFORM	= PAUSE_STATE_EXIT_COMMAND  + 5,
	PAUSE_STATE_EXIT_START_UNRETRACT_FILAMENT	= PAUSE_STATE_EXIT_COMMAND  + 6,
	PAUSE_STATE_EXIT_WAIT_UNRETRACT_FILAMENT	= PAUSE_STATE_EXIT_COMMAND  + 7,

	//Error display state
	PAUSE_STATE_ERROR                               = PAUSE_STATE_ERROR_COMMAND
};


/// The` command namespace contains functions that handle the incoming command
/// queue, for both SD and serial jobs.
namespace command {

#ifdef PSTOP_SUPPORT
extern bool pstop_triggered;
extern bool pstop_okay;
#if defined(PSTOP_ZMIN_LEVEL)
void possibleZLevelPStop();
#endif
#endif

#ifdef DITTO_PRINT
extern bool dittoPrinting;
#endif

extern uint16_t altTemp[EXTRUDERS];
extern int16_t pausedExtruderTemp[2];

/// Reset the entire command queue.  Clears out any remaining queued
/// commands.
void reset();

/// Reset core build state when building over USB and a build start
/// notification is encountered
void buildReset();

/// State which needs to be reset at the end of a build
void buildDone();

/// Adds the filament used in this build to eeprom
void addFilamentUsed();

/// Run the command thread slice.
void runCommandSlice();

/// Returns the build percentage (0-100).  This is 101 is the build percentage hasn't been set yet
uint8_t getBuildPercentage(void);

/// Called when filament is extracted via the filament menu during a pause.
/// It prevents noodle from being primed into the extruder on resume
void pauseUnRetractClear(void);

/// Pause the command processor
/// \param[in] pause If true, disable the command processor. If false, enable it.
void pause(bool pause, bool cold=false);

/// Returns the pausing intent
/// \return true if we've previously called a pause, and false if we've previously called unpause
/// \This denotes the intent, not that we've actually pasued, check pauseState for that
uint8_t isPaused();

/// \Pause at >= a Z Position provded in steps
/// 0 cancels pauseAtZPos
void pauseAtZPos(int32_t zpos);

/// Get the current pauseAtZPos position
/// \return the z position set for pausing (in steps), otherwise 0
int32_t getPauseAtZPos();

/// Returns the paused state
enum PauseState pauseState();

/// Returns true if we're transitioning between fully paused, or fully unpaused
bool pauseIntermediateState();

/// Check the state of the command processor
/// \return True if it is waiting for a button press, false if not
bool isWaiting();
    
/// Check the state of the command proccesor
/// \return True if it is in ready mode, false if not in ready mode
bool isReady();

/// Returns the length of filament extruded (in steps)
int64_t getFilamentLength(uint8_t extruder);

/// Returns the length of filament extruded (in steps) prior to the
/// last time the filament was added to the filament count
int64_t getLastFilamentLength(uint8_t extruder);

/// Returns the length of filament extruded (in mm) so far for the current print
float filamentUsed();

/// Check the remaining capacity of the command buffer
/// \return Amount of space left in the buffer, in bytes
uint16_t getRemainingCapacity();

/// Check if the command buffer is empty
/// \return true if is empty
bool isEmpty();

/// Push a byte onto the command buffer. This is used by the host to add commands
/// to the buffer.
/// \param[in] byte Byte to add to the buffer.
void push(uint8_t byte);

/// commands are no longer executed when the heat shutdown is activated
void heatShutdown();

#if defined(LINE_NUMBER)

/// return line number of current build
uint32_t getLineNumber();

/// clear line number count
void clearLineNumber();

#endif

/// if we update the line_counter  to allow overflow, we'll need to update the BuildStats Screen implementation
const static uint32_t MAX_LINE_COUNT = 1000000000;

#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS) || defined(ESTIMATE_TIME)

int32_t estimatedTimeLeftInSeconds();

float filamentUsed();

#endif

}

#endif // COMMAND_HH_
