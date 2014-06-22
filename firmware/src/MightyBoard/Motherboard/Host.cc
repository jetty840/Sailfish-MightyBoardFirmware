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
#include "Configuration.hh"
#if HONOR_DEBUG_PACKETS
#include "DebugPacketProcessor.hh"
#endif
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
#include "stdio.h"

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
Timeout do_host_reset_timeout;
bool buildWasCancelled;

#define HOST_PACKET_TIMEOUT_MS 200
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

//#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
//#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

char machineName[eeprom_info::MAX_MACHINE_NAME_LEN + 1];

char buildName[MAX_FILE_LEN];

uint32_t buildSteps;

/// Used to indicate what the UI should do, and used by
/// host process to know what state it's in for error/command allowed.
/// doesn't change state machine per-se, but sets context for other cmds.
HostState currentState = HOST_STATE_READY;

/// Used to indicate the status of the current or last finished print
/// is queryable by repG and by the stats screen during builds
BuildState buildState = BUILD_NONE;

/// counter for current print time
enum {
     PRINT_TIME_OTHER = 0,
     PRINT_TIME_ACTIVE,
     PRINT_TIME_PAUSED
};

static uint8_t  print_time_state = PRINT_TIME_OTHER;
static micros_t print_time_start;
static micros_t print_time_accum;

/// queryable time for last print
static uint16_t last_print_hours = 0;
static uint8_t  last_print_minutes = 0;

#if defined(LINE_NUMBER)
static uint32_t last_print_line = 0;
#endif

bool do_host_reset = false;
bool hard_reset = false;
bool cancelBuild = false;

void runHostSlice() {
	// If we're cancelling the build, and we have completed pausing,
	// then we cancel the build
	if (( buildState == BUILD_CANCELLING ) && ( command::pauseState() == PAUSE_STATE_PAUSED )) {
		stopBuildNow();
	}

        InPacket& in = UART::getHostUART().in;
        OutPacket& out = UART::getHostUART().out;
	if (out.isSending() &&
	    (( ! do_host_reset) || (do_host_reset && (! do_host_reset_timeout.hasElapsed())))) {
		return;
	}

    // soft reset the machine unless waiting to notify repG that a cancel has occured
	if (do_host_reset && (!cancelBuild || cancel_timeout.hasElapsed())){

		if((buildState == BUILD_RUNNING) || (buildState == BUILD_PAUSED)){
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
		packet_in_timeout.abort();
		out.reset();

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
				//PacketError::NOISE_BYTE and PacketError::APPEND_BUFFER_OVERFLOW
				out.append8(RC_PACKET_ERROR);
				break;
		}

		in.reset();
		UART::getHostUART().beginSend();
#if HONOR_DEBUG_PACKETS
		Motherboard::getBoard().indicateError(ERR_HOST_PACKET_MISC);
#endif

	}
	else if (in.isFinished() == 1) {
		//DEBUG_PIN1.setValue(false);
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
#if HONOR_DEBUG_PACKETS
			Motherboard::getBoard().indicateError(ERR_CANCEL_BUILD);
#endif
		} else
#if HONOR_DEBUG_PACKETS
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
	else if((currentState==HOST_STATE_BUILDING_ONBOARD))
	{
		if(!utility::isPlaying()) {
			currentState = HOST_STATE_READY;
			// Home Axes script is just the first ~100 bytes of the levelling
			//   script and thus ends without disabling the steppers
			steppers::enableAxes(0xff, false);
			BOARD_STATUS_CLEAR(Motherboard::STATUS_ONBOARD_SCRIPT);
		}
	}
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
#ifdef S3G_CAPTURE_2_SD
			// If we're capturing a file to an SD card, we send it to the sdcard module
			// for processing.
			if (sdcard::isCapturing()) {
				sdcard::capturePacket(from_host);
				to_host.append8(RC_OK);
				return true;
			}
#endif
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

    // Case to give an error on Replicator G versions older than 0038.
    if(from_host.read16(1) < 39) {
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
	from_host.read16(1);	//uint16_t host_version

	to_host.append8(RC_OK);
	to_host.append16(firmware_version);
	to_host.append16((uint16_t)0);
	to_host.append8(SOFTWARE_VARIANT_ID);
	to_host.append8(0);
	to_host.append16(0);

}

    // return build name
inline void handleGetBuildName(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (uint8_t idx = 0; idx < sizeof(buildName); idx++) {
	  to_host.append8(buildName[idx]);
	  if (buildName[idx] == '\0') break;
	}
}

inline void handleGetBufferSize(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(command::getRemainingCapacity());
}

inline void handleGetPosition(const InPacket& from_host, OutPacket& to_host) {
	uint8_t toolIndex;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		const Point p = steppers::getStepperPosition(&toolIndex);
		to_host.append8(RC_OK);
		to_host.append32(p[0]);
		to_host.append32(p[1]);
		to_host.append32(p[2]);
		// From spec:
		// endstop status bits: (7-0) : | N/A | N/A | z max | z min | y max | y min | x max | x min |
		uint8_t endstop_status = steppers::getEndstopStatus();
		to_host.append8(endstop_status);
	}
}

inline void handleGetPositionExt(const InPacket& from_host, OutPacket& to_host) {
	uint8_t toolIndex;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		const Point p = steppers::getStepperPosition(&toolIndex);
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
		uint8_t endstop_status = steppers::getEndstopStatus();

		to_host.append16((uint16_t)endstop_status);
	}
}

#ifdef S3G_CAPTURE_2_SD

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

#endif // S3G_CAPTURE_2_SD

    // playback from SD
inline void handlePlayback(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (uint8_t idx = 1; (idx < from_host.getLength()) && (idx < sizeof(buildName)); idx++)
		buildName[idx-1] = from_host.read8(idx);
	buildName[sizeof(buildName)-1] = '\0';
	to_host.append8(startBuildFromSD(buildName,0));
}

    // retrieve SD file names
void handleNextFilename(const InPacket& from_host, OutPacket& to_host) {
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
	bool isdir;
	char fnbuf[MAX_FILE_LEN];
	// Ignore dot-files
	do {
		sdcard::directoryNextEntry(fnbuf,sizeof(fnbuf),0,&isdir);
		if (fnbuf[0] == '\0') break;
		else if ( (fnbuf[0] != '.') ||
			  ( isdir && fnbuf[1] == '.' && fnbuf[2] == 0) ) break;
	} while (true);
	// Note that the old directoryNextEntry() always returned SD_SUCCESS
	to_host.append8(sdcard::SD_SUCCESS);
	uint8_t idx;
	for (idx = 0; (idx < sizeof(fnbuf)) && (fnbuf[idx] != 0); idx++)
		to_host.append8(fnbuf[idx]);
	to_host.append8(0);
}

    // pause command response
inline void handlePause(const InPacket& from_host, OutPacket& to_host) {
	//If we're either pausing or unpausing, but we haven't completed
	//the operation yet, we ignore this request
	if (!command::pauseIntermediateState()) {
		/// this command also calls the host::pauseBuild() command
		pauseBuild(command::isPaused() == 0, false);
	}

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
void handleReadEeprom(const InPacket& from_host, OutPacket& to_host) {

    uint16_t offset = from_host.read16(1);
    uint8_t length = from_host.read8(3);
    uint8_t data[length];
    eeprom_read_block(data, (const void*) offset, length);
    to_host.append8(RC_OK);
    for (int i = 0; i < length; i++) {
        to_host.append8(data[i]);
    }
}

/**
 * writes a chunk of data from a input packet to eeprom
 */
void handleWriteEeprom(const InPacket& from_host, OutPacket& to_host) {
    uint16_t offset = from_host.read16(1);
    uint8_t length = from_host.read8(3);
    uint8_t data[length];
    eeprom_read_block(data, (const void*) offset, length);
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
		steppers::abort();
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
	switch (currentState){
		case HOST_STATE_BUILDING_FROM_SD:
			// MBI copies the build name to the buildName[] buffer
			//   if it doesn't match 'RepG build'.  But why waste
			//   time if (1) it will always be 'RepG build', and (2)
			//   we'd rather see the SD card file name anyway?
			while ( buf.pop() ) ;
			break;
	        case HOST_STATE_READY:
			currentState = HOST_STATE_BUILDING;
			// Fallthrough
		case HOST_STATE_BUILDING_ONBOARD:
		case HOST_STATE_BUILDING:
		        command::buildReset();
		        // Setting lastFileIndex here is an attempt to make a non SD
		        // card build ensure that the print completion screen doesn't
		        // permit a "Print Again?" of the last printed SD card file.
		        // However, this logic WILL BE tripped up by a very, very small
		        // SD card file as the entire file will be read in one swoop
		        // into the command buffer and sdcard::isPlaying() will then
		        // return false causing currentState to change to HOST_STATE_READY
		        // before the start build notification command is processed from
		        // the command buffer.  We'll live with this for now.
		        lastFileIndex = 255;
			do {
				buildName[idx++] = buf.pop();
                        } while ((buildName[idx-1] != '\0') && (idx < sizeof(buildName)));
			buildName[sizeof(buildName)-1] = '\0';
			break;
		default:
		        lastFileIndex = 255;
			break;
	}
	startPrintTime();
#if defined(LINE_NUMBER)
	command::clearLineNumber();
#endif
	buildState = BUILD_RUNNING;
	buildWasCancelled = false;
}

    // set build state to ready
void handleBuildStopNotification(uint8_t stopFlags) {
	stopPrintTime();
#if defined(LINE_NUMBER)
	last_print_line = command::getLineNumber();
#endif

 	// ensure filament axes are disabled on stop build to prevent drool
	steppers::enableAxes(0xf8, false);

	// Turn all heaters off
	Motherboard::heatersOff(true);

 	// turn off the cooling fan
	EX_FAN.setValue(false);

	buildState = BUILD_FINISHED_NORMALLY;
	currentState = HOST_STATE_READY;
}

/// get current print stats if printing, or last print stats if not printing
inline void handleGetBuildStats(OutPacket& to_host) {
        to_host.append8(RC_OK);
	
	uint16_t hours;
	uint8_t minutes;

	getPrintTime(hours, minutes);

        to_host.append8(buildState);
        to_host.append8((uint8_t)(0xff & hours));
        to_host.append8(minutes);
#if defined(LINE_NUMBER)
        if( (buildState == BUILD_RUNNING) || (buildState == BUILD_PAUSED) ) {
	     to_host.append32(command::getLineNumber());
	} else {
	     to_host.append32(last_print_line);
	}
#else
	to_host.append32(0); // line number reporting not supported
#endif
        to_host.append32(0); // open spot for filament detect info
}
/// get current print stats if printing, or last print stats if not printing
inline void handleGetBoardStatus(OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append8(board_status);
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
			{
			        bool resetMe = true;
				command::addFilamentUsed();
				if (currentState == HOST_STATE_BUILDING ||
				    currentState == HOST_STATE_BUILDING_FROM_SD ||
				    currentState == HOST_STATE_BUILDING_ONBOARD) {
				     if (1 == eeprom::getEeprom8(eeprom_offsets::CLEAR_FOR_ESTOP, 0)) {
					  buildState = BUILD_CANCELED;
					  stopBuild();
					  resetMe = false;
				     }
				     // Motherboard::getBoard().indicateError(ERR_RESET_DURING_BUILD);
				}
				if ( resetMe ) {
				     do_host_reset = true; // indicate reset after response has been sent
				     do_host_reset_timeout.start(200000);	//Protection against the firmware sending to a down host
				}
				to_host.append8(RC_OK);
				return true;
			}
			case HOST_CMD_GET_BUFFER_SIZE:
				handleGetBufferSize(from_host,to_host);
				return true;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
			case HOST_CMD_GET_POSITION:
				handleGetPosition(from_host,to_host);
				return true;
			case HOST_CMD_GET_POSITION_EXT:
				handleGetPositionExt(from_host,to_host);
				return true;
#pragma GCC diagnostic pop
#ifdef S3G_CAPTURE_2_SD
			case HOST_CMD_CAPTURE_TO_FILE:
				handleCaptureToFile(from_host,to_host);
				return true;
			case HOST_CMD_END_CAPTURE:
				handleEndCapture(from_host,to_host);
				return true;
#endif
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
			case HOST_CMD_GET_BUILD_STATS:
				handleGetBuildStats(to_host);
				return true;
#pragma GCC diagnostic pop
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
		// WARNING: Owing to a bug in SanguinoDriver.java and
		//    MightyBoard.java, all versions of RepG up to and
		//    including RepG 0040 would NOT NUL terminate the
		//    string they sent to the bot's EEPROM if it had
		//    length >= 16.  As such this string can NOT be assumed
		//    to be NUL terminated.
		//
		//  This was fixed in RepG 40r4 Sailfish on 1 Feb 2013
		for(uint8_t i = 0; i < eeprom_info::MAX_MACHINE_NAME_LEN; i++) {
			machineName[i] = eeprom::getEeprom8(eeprom_offsets::MACHINE_NAME+i, 0);
		}
		machineName[eeprom_info::MAX_MACHINE_NAME_LEN] = 0;
	}

	// If EEPROM is zero, load in a default. The 0 is there on purpose
	// since this fallback should only happen on EEPROM total failure
#ifdef MODEL_REPLICATOR2
	const static PROGMEM prog_uchar defaultMachineName[] = "Replicat0r 2";
#else
	const static PROGMEM prog_uchar defaultMachineName[] = "Replicat0r 1";
#endif

	if (machineName[0] == 0) {
	        for(uint8_t i = 0; i < 12; i++) {
			machineName[i] = pgm_read_byte_near(defaultMachineName+i);
		}
		machineName[12] = '\0';
	}
	return machineName;
}

HostState getHostState() {
	return currentState;
}

BuildState getBuildState() {
	return buildState;
}

void resumePrintTime() {
     print_time_start = Motherboard::getBoard().getCurrentSeconds();
     print_time_state = PRINT_TIME_ACTIVE;
}

void startPrintTime() {
     print_time_accum = 0;
     resumePrintTime();
}

micros_t deltaPrintTime() {
     return ( print_time_state != PRINT_TIME_ACTIVE ) ? (micros_t)0 :
	  Motherboard::getBoard().getCurrentSeconds() - print_time_start;
}

void pausePrintTime() {
     print_time_accum += deltaPrintTime();
     print_time_state = PRINT_TIME_PAUSED;
}

void stopPrintTime() {
    // Set last_print_hours & last_print_minutes
    // We do this call so that the global variables are set and
    //   can be returned by getPrintTime() when no print is active
    getPrintTime(last_print_hours, last_print_minutes);

    print_time_state = PRINT_TIME_OTHER;

    // Save the information
    eeprom::updateBuildTime(last_print_hours, last_print_minutes);
}

uint32_t getPrintSeconds(void) {
     return (uint32_t)(deltaPrintTime() + print_time_accum);
}

/// returns time hours and minutes since the start of the print
void getPrintTime(uint16_t& hours, uint8_t& minutes) {
    if ( print_time_state == PRINT_TIME_OTHER ) {
	 hours   = last_print_hours;
	 minutes = last_print_minutes;
    }
    else {
	 uint32_t so_far = getPrintSeconds();
	 hours   = (uint16_t)(so_far / (60 * 60));
	 minutes = (uint16_t)(( so_far / 60 ) - hours * 60);
    }
    return;
}

sdcard::SdErrorCode startBuildFromSD(char *fname, uint8_t flen) {
	sdcard::SdErrorCode e;

	// Attempt to start build
	if ( !fname ) fname = buildName;
	else if ( fname != buildName ) {
		if ( flen > sizeof(buildName) ) flen = sizeof(buildName) - 1;
		for (uint8_t i = 0; i < flen; i++) buildName[i] = fname[i];
		buildName[flen] = 0;
	}
	e = sdcard::startPlayback(fname);
	if (e == sdcard::SD_CWD) return sdcard::SD_SUCCESS;
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return e;
	}

	// clear heater temps
	Motherboard::heatersOff(true);

	command::reset();
	steppers::reset();
	steppers::abort();
	buildWasCancelled = false;
	currentState = HOST_STATE_BUILDING_FROM_SD;

	return e;
}
// start build from utility script
void startOnboardBuild(uint8_t  build){
    buildWasCancelled = false;
    if ( utility::startPlayback(build) )
	currentState = HOST_STATE_BUILDING_ONBOARD;
    BOARD_STATUS_SET(Motherboard::STATUS_ONBOARD_SCRIPT);
    command::reset();
    steppers::abort();
    // steppers::reset();
    lastFileIndex = 255;
}

// Stop the current build, if any
void stopBuildNow() {
    // if building from repG, try to send a cancel msg to repG before reseting
    if ( currentState == HOST_STATE_BUILDING )
    {
	currentState = HOST_STATE_CANCEL_BUILD;
	cancel_timeout.start(1000000); //look for commands from repG for one second before resetting
	cancelBuild = true;
    }
#if defined(LINE_NUMBER)
    last_print_line = command::getLineNumber();
#endif
    stopPrintTime();
    do_host_reset = true; // indicate reset after response has been sent
    do_host_reset_timeout.start(200000);	//Protection against the firmware sending to a down host
    buildState = BUILD_CANCELED;
}

// Stop the current build, if any via an intermediate state (BUILD_CANCELLING),
// where we pause first and when that's complete we call stopBuildNow to cancel the
// print.  The purpose of the pause is to move the build away from the tool head.
void stopBuild() {
    buildState = BUILD_CANCELLING;
    buildWasCancelled = true;
    steppers::abort();

    BOARD_STATUS_SET(Motherboard::STATUS_CANCELLING);
    BOARD_STATUS_CLEAR(Motherboard::STATUS_ONBOARD_SCRIPT);

    //If we're already paused, we stop the print now, otherwise we pause
    //The runSlice picks up this pause later when completed, then calls stopBuildNow
    if ( (command::isPaused()) || (command::pauseIntermediateState()) )
	stopBuildNow();
    else
	command::pause(true);
}

/// update state variables if print is paused
void pauseBuild(bool pause, bool cold) {
	/// don't update time or state if we are already in the desired state
	bool isPaused = 0 != command::isPaused();
	if ( !(pause == isPaused) ) {
		//If we're either pausing or unpausing, but we haven't completed
		//the operation yet, we ignore this request
		if (command::pauseIntermediateState())
			return;

		command::pause(pause, cold);
		if ( pause ) {
		     buildState = BUILD_PAUSED;
		     pausePrintTime();
		}
		else {
		     buildState = BUILD_RUNNING;
		     resumePrintTime();
		}
	}
}

    // legacy tool / motherboard breakout of query commands
bool processExtruderQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	Motherboard& board = Motherboard::getBoard();
	if (from_host.getLength() >= 1) {

        uint8_t	id = from_host.read8(1);
		uint8_t command = from_host.read8(2);
		// All commands are query commands.
		to_host.append8(RC_OK);
		switch (command) {
		case SLAVE_CMD_VERSION:
			to_host.append16(firmware_version);
			return true;
		case SLAVE_CMD_GET_TEMP:
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_IS_TOOL_READY:
			to_host.append8(board.getExtruderBoard(id).getExtruderHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_PLATFORM_TEMP:
			to_host.append16(board.getPlatformHeater().get_current_temperature());
			return true;
		case SLAVE_CMD_GET_SP:
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_GET_PLATFORM_SP:
			to_host.append16(board.getPlatformHeater().get_set_temperature());
			return true;
		case SLAVE_CMD_IS_PLATFORM_READY:
			to_host.append8(board.getPlatformHeater().has_reached_target_temperature()?1:0);
			return true;
		case SLAVE_CMD_GET_TOOL_STATUS:
			to_host.append8((board.getExtruderBoard(id).getExtruderHeater().has_failed()?128:0)
							| (board.getPlatformHeater().has_failed()?64:0)
							| (board.getExtruderBoard(id).getExtruderHeater().GetFailMode())
							| (board.getExtruderBoard(id).getExtruderHeater().has_reached_target_temperature()?1:0));
			return true;
		case SLAVE_CMD_GET_PID_STATE:
#if defined(SUPPORT_GET_PID_STATE)
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDErrorTerm());
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDDeltaTerm());
			to_host.append16(board.getExtruderBoard(id).getExtruderHeater().getPIDLastOutput());
			to_host.append16(board.getPlatformHeater().getPIDErrorTerm());
			to_host.append16(board.getPlatformHeater().getPIDDeltaTerm());
			to_host.append16(board.getPlatformHeater().getPIDLastOutput());
#else
			to_host.append32(0);
			to_host.append32(0);
			to_host.append32(0);
#endif
			return true;
		}
	}
	return false;
}

#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS)

bool isBuildComplete() {
	if (( command::isEmpty() ) && ( ! sdcard::playbackHasNext() ))	return true;
	return false;
}

#endif

}
/* footnote 1: due to a protocol change, replicatiorG 0026 and newer can ONLY work with
 * firmware 3.00 and newer. Because replicatorG handles version mismatches poorly,
 * if our firmware is 3.0 or newer, *AND* the connecting replicatorG is 25 or older, we
 * lie, and reply with firmware 0.00 to case ReplicatorG to display a 'null version' error
 * so users will know to upgrade.
 */
