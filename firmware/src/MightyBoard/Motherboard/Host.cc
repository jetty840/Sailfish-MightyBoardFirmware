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
 
#include "Host.hh"
#include "Command.hh"
#include <string.h>
#include "Commands.hh"
#include "Steppers.hh"
#include "DebugPacketProcessor.hh"
#include "Timeout.hh"
#include "Version.hh"
#include <util/atomic.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "Main.hh"
#include "Errors.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "UtilityScripts.hh"
#include "Planner.hh"
#include "stdio.h"
#include "Menu_locales.hh"

namespace host {

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host);
bool processQueryPacket(const InPacket& from_host, OutPacket& to_host);
bool processExtruderQueryPacket(const InPacket& from_host, OutPacket& to_host);

// Timeout from time first bit recieved until we abort packet reception
Timeout packet_in_timeout;
Timeout cancel_timeout;
Timeout z_stage_timeout;

#define HOST_PACKET_TIMEOUT_MS 200
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

//#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
//#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

char machineName[eeprom_info::MAX_MACHINE_NAME_LEN];

char buildName[MAX_FILE_LEN];

uint32_t buildSteps;

/// Used to indicate what the UI should do, and used by
/// host process to know what state it's in for error/command allowed.
/// doesn't change state machine per-se, but sets context for other cmds.
HostState currentState;

/// Used to indicate the status of the current or last finished print
/// is queryable by repG and by the stats screen during builds
BuildState buildState = BUILD_NONE;

/// queryable time for last print
uint8_t last_print_hours = 0;
uint8_t last_print_minutes = 0;

uint32_t last_print_line = 0;

/// counter for current print time
uint8_t print_time_hours = 0;
Timeout print_time;

const static uint32_t ONE_HOUR = 3600000000;


bool do_host_reset = false;
bool hard_reset = false;
bool cancelBuild = false;

void runHostSlice() {
		
        InPacket& in = UART::getHostUART().in;
        OutPacket& out = UART::getHostUART().out;
	if (out.isSending()) {
		// still sending; wait until send is complete before reading new host packets.
		return;
	}
    // soft reset the machine unless waiting to notify repG that a cancel has occured
	if (do_host_reset && (!cancelBuild || cancel_timeout.hasElapsed()) && (!z_stage_timeout.isActive() || z_stage_timeout.hasElapsed() || !steppers::isRunning())){
			
		if((buildState == BUILD_RUNNING) || (buildState == BUILD_PAUSED) || (buildState == BUILD_SLEEP)){
			stopBuild();
		}
		do_host_reset = false;

		// reset local board
		reset(hard_reset);
		
        // hard_reset can be called, but is not called by any
        // a hard reset calls the start up sound and resets heater errors
		hard_reset = false;
		packet_in_timeout.abort();

		// Clear the machine and build names
		machineName[0] = 0;
		buildName[0] = 0;
		currentState = HOST_STATE_READY;
			
		return;
	}
    // new packet coming in
	if (in.isStarted() && !in.isFinished()) {
		if (!packet_in_timeout.isActive()) {
			// initiate timeout
			packet_in_timeout.start(HOST_PACKET_TIMEOUT_MICROS);
		} else if (packet_in_timeout.hasElapsed()) {
			in.timeout();
		}

	}
	if (in.hasError()) {
		// Reset packet quickly and start handling the next packet.
		
	/*	out.reset();
			
		// Report error code.
		switch (in.getErrorCode()){
			case PacketError::PACKET_TIMEOUT:
				out.append8(RC_PACKET_TIMEOUT);
				break;
			case PacketError::BAD_CRC:
				out.append8(RC_CRC_MISMATCH);
				break;
			case PacketError::EXCEEDED_MAX_LENGTH:
				out.append8(RC_PACKET_LENGTH);
				break;
			default:
				out.append8(RC_PACKET_ERROR);
				break;
		}
		*/  	
		in.reset();
		//UART::getHostUART().beginSend();
		//Motherboard::getBoard().indicateError(ERR_HOST_PACKET_MISC);
		
	}
	if (in.isFinished()) {
		packet_in_timeout.abort();
		out.reset();
	  // do not respond to commands if the bot has had a heater failure
		if(currentState == HOST_STATE_HEAT_SHUTDOWN){
			if(cancelBuild){
				out.append8(RC_CANCEL_BUILD);
				cancelBuild= false;
			}else{
				out.append8(RC_BOT_OVERHEAT);
			}
		}else if(cancelBuild){
			out.append8(RC_CANCEL_BUILD);
			cancelBuild = false;
			Motherboard::getBoard().indicateError(6);
		} else
#if defined(HONOR_DEBUG_PACKETS) && (HONOR_DEBUG_PACKETS == 1)
		if (processDebugPacket(in, out)) {
			// okay, processed
		} else
#endif
		if (processCommandPacket(in, out)) {
			// okay, processed
		} else if (processQueryPacket(in, out)) {
			// okay, processed
		} else {
			// Unrecognized command
			out.append8(RC_CMD_UNSUPPORTED);
		}
		in.reset();
                UART::getHostUART().beginSend();
	}
    /// mark new state as ready if done building from SD
	if(currentState==HOST_STATE_BUILDING_FROM_SD)
	{
		if(!sdcard::isPlaying())
			currentState = HOST_STATE_READY;
	}
    // mark new state as ready if done buiding onboard script
	if((currentState==HOST_STATE_BUILDING_ONBOARD))
	{
		if(!utility::isPlaying()){
			currentState = HOST_STATE_READY;
		}
		Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_SCRIPT, false);
	}
	managePrintTime();
}

/** Identify a command packet, and process it.  If the packet is a command
 * packet, return true, indicating that the packet has been queued and no
 * other processing needs to be done. Otherwise, processing of this packet
 * should drop through to the next processing level.
 */
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) != 0) {
			// If we're capturing a file to an SD card, we send it to the sdcard module
			// for processing.
			if (sdcard::isCapturing()) {
				sdcard::capturePacket(from_host);
				to_host.append8(RC_OK);
				return true;
			}
			if(sdcard::isPlaying() || utility::isPlaying()){
				// ignore action commands if SD card build is playing
				// or if ONBOARD script is playing
				to_host.append8(RC_BOT_BUILDING);
				return true;
			}
			
			// Queue command, if there's room.
			// Turn off interrupts while querying or manipulating the queue!
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				const uint8_t command_length = from_host.getLength();
				if (command::getRemainingCapacity() >= command_length) {
					// Append command to buffer
					for (int i = 0; i < command_length; i++) {
						command::push(from_host.read8(i));
					}
					to_host.append8(RC_OK);
				} else {
					to_host.append8(RC_BUFFER_OVERFLOW);
				}
			}
			return true;
		}
	}
	return false;
}

    // alert the host that the bot has had a heat failure
void heatShutdown(){
	currentState = HOST_STATE_HEAT_SHUTDOWN;
	cancelBuild = true;
}


// Received driver version info, and request for fw version info.
// puts fw version into a reply packet, and send it back
inline void handleVersion(const InPacket& from_host, OutPacket& to_host) {

    // Case to give an error on Replicator G versions older than 0025. See footnote 1
    if(from_host.read16(1)  <=  25   ) {
        to_host.append8(RC_OK);
        to_host.append16(0x0000);
    }
    else  {
        to_host.append8(RC_OK);
        to_host.append16(firmware_version);
    }

}

// Received driver version info, and request for fw version info.
// puts fw version into a reply packet, and send it back
inline void handleGetAdvancedVersion(const InPacket& from_host, OutPacket& to_host) {

	// we're not doing anything with the host version at the moment
	uint16_t host_version = from_host.read16(1);
	
	to_host.append8(RC_OK);
	to_host.append16(firmware_version);
	to_host.append16(internal_version);
	to_host.append16(0);
	to_host.append16(0);

}

    // return build name
inline void handleGetBuildName(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (uint8_t idx = 0; idx < MAX_FILE_LEN; idx++) {
	  to_host.append8(buildName[idx]);
	  if (buildName[idx] == '\0') { break; }
	}
}

inline void handleGetBufferSize(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(command::getRemainingCapacity());
}

inline void handleGetPosition(const InPacket& from_host, OutPacket& to_host) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		const Point p = planner::getPosition();
		to_host.append8(RC_OK);
		to_host.append32(p[0]);
		to_host.append32(p[1]);
		to_host.append32(p[2]);
		// From spec:
		// endstop status bits: (7-0) : | N/A | N/A | z max | z min | y max | y min | x max | x min |
		Motherboard& board = Motherboard::getBoard();
		uint8_t endstop_status = steppers::getEndstopStatus();
		to_host.append8(endstop_status);
	}
}

inline void handleGetPositionExt(const InPacket& from_host, OutPacket& to_host) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		const Point p = planner::getPosition();
		to_host.append8(RC_OK);
		to_host.append32(p[0]);
		to_host.append32(p[1]);
		to_host.append32(p[2]);
#if STEPPER_COUNT > 3
		to_host.append32(p[3]);
		to_host.append32(p[4]);
#else
		to_host.append32(0);
		to_host.append32(0);
#endif
		// From spec:
		// endstop status bits: (15-0) : | b max | b min | a max | a min | z max | z min | y max | y min | x max | x min |
		Motherboard& board = Motherboard::getBoard();
		uint8_t endstop_status = steppers::getEndstopStatus();
		
		to_host.append16((uint16_t)endstop_status);
	}
}

    // capture to SD
inline void handleCaptureToFile(const InPacket& from_host, OutPacket& to_host) {
	char *p = (char*)from_host.getData() + 1;
	to_host.append8(RC_OK);
	to_host.append8(sdcard::startCapture(p));
}
    // stop capture to SD
inline void handleEndCapture(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(sdcard::finishCapture());
	sdcard::reset();
}

    // playback from SD
inline void handlePlayback(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (int idx = 1; idx < from_host.getLength(); idx++) {
		buildName[idx-1] = from_host.read8(idx);
	}
	buildName[MAX_FILE_LEN-1] = '\0';

	uint8_t response = startBuildFromSD();
	to_host.append8(response);
}

    // retrive SD file names
inline void handleNextFilename(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	uint8_t resetFlag = from_host.read8(1);
	if (resetFlag != 0) {
		sdcard::SdErrorCode e = sdcard::directoryReset();
		if (e != sdcard::SD_SUCCESS) {
			to_host.append8(e);
			to_host.append8(0);
			return;
		}
	}
	int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;
	char fnbuf[MAX_FILE_LEN];
	sdcard::SdErrorCode e;
	// Ignore dot-files
	do {
		e = sdcard::directoryNextEntry(fnbuf,MAX_FILE_LEN);
		if (fnbuf[0] == '\0') break;
	} while (e == sdcard::SD_SUCCESS && fnbuf[0] == '.');
	to_host.append8(e);
	uint8_t idx;
	for (idx = 0; (idx < MAX_FILE_LEN) && (fnbuf[idx] != 0); idx++) {
		to_host.append8(fnbuf[idx]);
	}
	to_host.append8(0);
}

    // pause command response
inline void handlePause(const InPacket& from_host, OutPacket& to_host) {
	/// this command also calls the host::pauseBuild() command
	pauseBuild(!command::isPaused());
	to_host.append8(RC_OK);
}

inline void handleSleep(const InPacket& from_host, OutPacket& to_host) {
	/// this command also calls the host::pauseBuild() command
	activePauseBuild(!command::isActivePaused(), command::SLEEP_TYPE_COLD);
	to_host.append8(RC_OK);
}

    // check if steppers are still executing a command
inline void handleIsFinished(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		bool done = !steppers::isRunning() && command::isEmpty();
		to_host.append8(done?1:0);
	}
}
    // read value from eeprom
inline void handleReadEeprom(const InPacket& from_host, OutPacket& to_host) {
    
    uint16_t offset = from_host.read16(1);
    uint8_t length = from_host.read8(3);
    uint8_t data[length];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    eeprom_read_block(data, (const void*) offset, length);
	}
    to_host.append8(RC_OK);
    for (int i = 0; i < length; i++) {
        to_host.append8(data[i]);
    }
}

/**
 * writes a chunk of data from a input packet to eeprom
 */
inline void handleWriteEeprom(const InPacket& from_host, OutPacket& to_host) {
    uint16_t offset = from_host.read16(1);
    uint8_t length = from_host.read8(3);
    uint8_t data[length];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    eeprom_read_block(data, (const void*) offset, length);
	}
    for (int i = 0; i < length; i++) {
        data[i] = from_host.read8(i + 4);
    }
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		eeprom_write_block(data, (void*) offset, length);
	}
    to_host.append8(RC_OK);
    to_host.append8(length);
}

enum { // bit assignments
	ES_STEPPERS = 0, // stop steppers
	ES_COMMANDS = 1  // clean queue
};

    // stop steppers and command execution
inline void handleExtendedStop(const InPacket& from_host, OutPacket& to_host) {
	uint8_t flags = from_host.read8(1);
	if (flags & _BV(ES_STEPPERS)) {
		planner::abort();
	}
	if (flags & _BV(ES_COMMANDS)) {
		command::reset();
	}

	to_host.append8(RC_OK);
	to_host.append8(0);
}

    //set build name and build state
void handleBuildStartNotification(CircularBuffer& buf) {
	
	uint8_t idx = 0;
	char newName[MAX_FILE_LEN];
	switch (currentState){
		case HOST_STATE_BUILDING_FROM_SD:
			do {
				newName[idx++] = buf.pop();		
			} while ((newName[idx-1] != '\0') && (idx < MAX_FILE_LEN));
			if(strcmp(newName, "RepG Build"))
				strcpy(buildName, newName);
			break;
		case HOST_STATE_READY:
			currentState = HOST_STATE_BUILDING;
		case HOST_STATE_BUILDING_ONBOARD:
		case HOST_STATE_BUILDING:
			do {
				buildName[idx++] = buf.pop();		
			} while ((buildName[idx-1] != '\0') && (idx < MAX_FILE_LEN));
			break;
	}
	//interface::BuildStart();
	startPrintTime();
	command::clearLineNumber();
	buildState = BUILD_RUNNING;
}

    // set build state to ready
void handleBuildStopNotification(uint8_t stopFlags) {
	uint8_t flags = stopFlags;
	
	
	Motherboard::getBoard().getInterfaceBoard().queueScreen(InterfaceBoard::BUILD_FINISHED);
	//interface::BuildFinished();
	stopPrintTime();
	last_print_line = command::getLineNumber();
	buildState = BUILD_FINISHED_NORMALLY;
	currentState = HOST_STATE_READY;

  // turn off the cooling fan
  EX_FAN.setValue(false);
}

/// get current print stats if printing, or last print stats if not printing
inline void handleGetBuildStats(OutPacket& to_host) {
        to_host.append8(RC_OK);
 
		uint8_t hours;
		uint8_t minutes;
		
		getPrintTime(hours, minutes);
		
        to_host.append8(buildState);
        to_host.append8(hours);
        to_host.append8(minutes);
        if((buildState == BUILD_RUNNING) || (buildState == BUILD_PAUSED) || (buildState == BUILD_SLEEP)){
			to_host.append32(command::getLineNumber());
		} else {
			to_host.append32(last_print_line);
		}
        to_host.append32(0);// open spot for filament detect info
}
/// get current print stats if printing, or last print stats if not printing
/// for documentation of these status bytes, see docs/MotherboardStatusBytes.md
inline void handleGetBoardStatus(OutPacket& to_host) {
	Motherboard& board = Motherboard::getBoard();
	to_host.append8(RC_OK);
	to_host.append8(board.GetBoardStatus());
}

// query packets (non action, not queued)
bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) == 0) {
			// Is query command.
			switch (command) {
			case HOST_CMD_VERSION:
				handleVersion(from_host,to_host);
				return true;
			case HOST_CMD_GET_BUILD_NAME:
				handleGetBuildName(from_host,to_host);
				return true;
			case HOST_CMD_INIT:
				// There's really nothing we want to do here; we don't want to
				// interrupt a running build, for example.
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_CLEAR_BUFFER: // equivalent at current time
			case HOST_CMD_ABORT: // equivalent at current time
			case HOST_CMD_RESET:
				if (currentState == HOST_STATE_BUILDING
						|| currentState == HOST_STATE_BUILDING_FROM_SD
						|| currentState == HOST_STATE_BUILDING_ONBOARD) {
          host::stopBuild();
					Motherboard::getBoard().indicateError(ERR_RESET_DURING_BUILD);
				}else{
          do_host_reset = true; // indicate reset after response has been sent
        }
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_GET_BUFFER_SIZE:
				handleGetBufferSize(from_host,to_host);
				return true;
			case HOST_CMD_GET_POSITION:
				handleGetPosition(from_host,to_host);
				return true;
			case HOST_CMD_GET_POSITION_EXT:
				handleGetPositionExt(from_host,to_host);
				return true;
			case HOST_CMD_CAPTURE_TO_FILE:
				handleCaptureToFile(from_host,to_host);
				return true;
			case HOST_CMD_END_CAPTURE:
				handleEndCapture(from_host,to_host);
				return true;
			case HOST_CMD_PLAYBACK_CAPTURE:
				handlePlayback(from_host,to_host);
				return true;
			case HOST_CMD_NEXT_FILENAME:
				handleNextFilename(from_host,to_host);
				return true;
			case HOST_CMD_PAUSE:
				handlePause(from_host,to_host);
				return true;
			case HOST_CMD_TOOL_QUERY:
				if(processExtruderQueryPacket(from_host,to_host)){
					return true;}
				break;
			case HOST_CMD_IS_FINISHED:
				handleIsFinished(from_host,to_host);
				return true;
			case HOST_CMD_READ_EEPROM:
				handleReadEeprom(from_host,to_host);
				return true;
			case HOST_CMD_WRITE_EEPROM:
				handleWriteEeprom(from_host,to_host);
				return true;
			case HOST_CMD_EXTENDED_STOP:
				handleExtendedStop(from_host,to_host);
				return true;
			case HOST_CMD_BOARD_STATUS:
				handleGetBoardStatus(to_host);
				return true;
			case HOST_CMD_GET_BUILD_STATS:
				handleGetBuildStats(to_host);
				return true;
			case HOST_CMD_ADVANCED_VERSION:
				handleGetAdvancedVersion(from_host, to_host);
				return true;
			}
		}
	}
	return false;
}

char* getMachineName() {
	// If the machine name hasn't been loaded, load it
	if (machineName[0] == 0) {
		for(uint8_t i = 0; i < eeprom_info::MAX_MACHINE_NAME_LEN; i++) {
			machineName[i] = eeprom::getEeprom8(eeprom_offsets::MACHINE_NAME+i, 0);
		}
	}

	// If EEPROM is zero, load in a default. The 0 is there on purpose
	// since this fallback should only happen on EEPROM total failure
	static PROGMEM prog_uchar defaultMachineName[] =  "The Replicat0r";

	if (machineName[0] == 0) {
		for(uint8_t i = 0; i < 14; i++) {
			machineName[i] = pgm_read_byte_near(defaultMachineName+i);
		}
	}

	return machineName;
}

char* getBuildName() {
	return buildName;
}

HostState getHostState() {
	return currentState;
}

sdcard::SdErrorCode startBuildFromSD() {
	sdcard::SdErrorCode e;
	
	
	// Attempt to start build
	e = sdcard::startPlayback(buildName);
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return e;
	}
	
	// clear heater temps
//	Motherboard::getBoard().getPlatformHeater().set_target_temperature(0);
//	Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
//	Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
	
	command::reset();
	steppers::reset();
	planner::abort();
	Motherboard::getBoard().reset(false);

	currentState = HOST_STATE_BUILDING_FROM_SD;

	return e;
}
    // start build from utility script
void startOnboardBuild(uint8_t  build){
	
	if(utility::startPlayback(build)){
		currentState = HOST_STATE_BUILDING_ONBOARD;
    Motherboard::getBoard().getInterfaceBoard().RecordOnboardStartIdx();
    Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_SCRIPT, true);
    command::reset();
    planner::abort();
  }
}

// Stop the current build, if any
void stopBuild() {
	
	if((currentState == host::HOST_STATE_BUILDING) ||
		(currentState == host::HOST_STATE_BUILDING_FROM_SD)){
    	
    planner::abort();

		// record print statistics
		last_print_line = command::getLineNumber();
		stopPrintTime();
		buildState = BUILD_CANCELED;
		command::ActivePause(true, command::SLEEP_TYPE_NONE);
		
		uint8_t hours;
		uint8_t minutes;
		getPrintTime(hours, minutes);
		/// lower the z stage if a build is canceled
		/// ensure that we have homed all axes before attempting this
    uint8_t z_home = steppers::isZHomed();   
    if(z_home){
      Point target = planner::getPosition();
      if(z_home == 1) {target[2] = 58000;}
      else {target[2] = 60000;}
      command::pause(false);
      planner::addMoveToBuffer(target, 150);
			InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
      ib.errorMessage(CANCEL_PLATE_MSG);
      ib.lock();
      z_stage_timeout.start(5000000);  //5 seconds
    }
	}
	
    // if building from repG, try to send a cancel msg to repG before reseting 
	if(currentState == HOST_STATE_BUILDING)
	{	
		currentState = HOST_STATE_CANCEL_BUILD;
		cancelBuild = true;
		cancel_timeout.start(1000000);  //1 seconds
	}
	
	Motherboard::getBoard().setBoardStatus(Motherboard::STATUS_ONBOARD_SCRIPT, false);
	do_host_reset = true; // indicate reset after response has been sent
	buildState = BUILD_CANCELED;
}

/// update state variables if print is paused
void pauseBuild(bool pause){
	
	/// don't update time or state if we are already in the desired state
	if (!(pause == command::isPaused())){
		
		command::pause(pause);
		if(pause){
			buildState = BUILD_PAUSED;
			print_time.pause(true);
		}else{
			buildState = BUILD_RUNNING;
			print_time.pause(false);
		}
	}
}

void activePauseBuild(bool pause, command::SleepType type){

	/// don't update time or state if we are already in the desired state
	if (!(pause == command::isActivePaused())){
		
		command::ActivePause(pause, type);
		if(pause){
			buildState = BUILD_SLEEP;
			print_time.pause(true);
		}else{
			buildState = BUILD_RUNNING;
			print_time.pause(false);
		}
	}
}

void startPrintTime(){
	print_time.start(ONE_HOUR);
	print_time_hours = 0;
}

void stopPrintTime(){
	
	getPrintTime(last_print_hours, last_print_minutes);
	eeprom::updateBuildTime(last_print_hours, last_print_minutes);
	print_time = Timeout();
	print_time_hours = 0;
}

void managePrintTime(){

	/// print time is precise to the host loop frequency 
	if (print_time.hasElapsed()){
		print_time.start(ONE_HOUR);
		print_time_hours++;
	}
}

/// returns time hours and minutes since the start of the print
void getPrintTime(uint8_t& hours, uint8_t& minutes){
	
	if(!print_time.isActive()){
		hours = last_print_hours;
		minutes = last_print_minutes;
	} else{
		hours = print_time_hours;
		minutes = print_time.getCurrentElapsed() / 60000000;
	}
	return;
}

    // legacy tool / motherboard breakout of query commands
bool processExtruderQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	Motherboard& board = Motherboard::getBoard();
	if (from_host.getLength() >= 1) {
			
        uint8_t	id = from_host.read8(1);
		uint8_t command = from_host.read8(2);
		// All commands are query commands.	
		switch (command) {
		case SLAVE_CMD_VERSION:
			to_host.append8(RC_OK);
			to_host.append16(firmware_version);
			return true;
		case SLAVE_CMD_GET_TEMP:
			to_host.append8(RC_OK);
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_IS_TOOL_READY:
			to_host.append8(RC_OK);
			to_host.append8(board.getExtruderBoard(id).getExtruderHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_PLATFORM_TEMP:
			to_host.append8(RC_OK);
			to_host.append16(board.getPlatformHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_GET_SP:
			to_host.append8(RC_OK);
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_GET_PLATFORM_SP:
			to_host.append8(RC_OK);
			to_host.append16(board.getPlatformHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_IS_PLATFORM_READY:
			to_host.append8(RC_OK);
			to_host.append8(board.getPlatformHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_TOOL_STATUS:
			to_host.append8(RC_OK);
			to_host.append8((board.getExtruderBoard(id).getExtruderHeater().has_failed()?128:0)
							| (board.getPlatformHeater().has_failed()?64:0)
							| (board.getExtruderBoard(id).getExtruderHeater().GetFailMode())
							| (board.getExtruderBoard(id).getExtruderHeater().has_reached_target_temperature()?1:0));
			return true;
		case SLAVE_CMD_GET_PID_STATE:
			to_host.append8(RC_OK);
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDErrorTerm());
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDDeltaTerm());
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDLastOutput());
			to_host.append16(board.getPlatformHeater().getPIDErrorTerm());
			to_host.append16(board.getPlatformHeater().getPIDDeltaTerm());
			to_host.append16(board.getPlatformHeater().getPIDLastOutput());
			return true;
		}
	}
	return false;
}

}
/* footnote 1: due to a protocol change, replicatiorG 0026 and newer can ONLY work with
 * firmware 3.00 and newer. Because replicatorG handles version mismatches poorly,
 * if our firmware is 3.0 or newer, *AND* the connecting replicatorG is 25 or older, we
 * lie, and reply with firmware 0.00 to case ReplicatorG to display a 'null version' error
 * so users will know to upgrade.
 */
