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

#include "Command.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "Configuration.hh"
#include "Timeout.hh"
#include "CircularBuffer.hh"
#include <util/atomic.h>
#include <avr/eeprom.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "SDCard.hh"
#include "Pin.hh"
#include <util/delay.h>
#include "Piezo.hh"

#ifdef HAS_RGB_LED
#include "RGB_LED.hh"
#endif

#include "Interface.hh"
#include "UtilityScripts.hh"
#include "stdio.h"
#include "Menu_locales.hh"
#include "Version.hh"
#include <math.h>

#if defined(AUTO_LEVEL)
#include "SkewTilt.hh"
#endif

namespace command {

static bool sdCardError;

#if defined(PSTOP_SUPPORT)
// When non-zero, a P-Stop has been requested
bool pstop_triggered = 0;

// We don't want to execute a Pause until after the coordinate system
// has been established by either recalling home offsets or a G92 X Y Z A B
// command.  Some people use the G92 approach so that RepG will generate
// an accelerated move command for the very first move.  This lets them
// have a fast platform move along the Z axis.  Unfortunately, S3G's
// G92-like command is botched and ALL coordinates are set.  That makes
// it impossible to tell if the gcode actually intended to set all the
// coordinates or if it was simply a G92 Z0.
bool pstop_okay = false;

// One way to tell if it's okay to allow a pstop is to assume it's
// okay after a few G1 commands.
uint8_t pstop_move_count = 0;

#endif

#define COMMAND_BUFFER_SIZE 512
uint8_t buffer_data[COMMAND_BUFFER_SIZE];
CircularBuffer command_buffer(COMMAND_BUFFER_SIZE, buffer_data);
uint8_t currentToolIndex = 0;

#if defined(LINE_NUMBER)
uint32_t line_number;
#define LINE_NUMBER_INCR line_number++
#else
#define LINE_NUMBER_INCR
#endif

#if defined(HEATERS_ON_STEROIDS)
#if !defined(FF_CREATOR) && !defined(WANHAO_DUP4) && !defined(FF_CREATOR_X)
#warning "Building with HEATERS_ON_STEROIDS defined will create firmware which allows ALL heaters to heatup at the same time; this requires a PSU, power connector, and associated electronics capable of handling much higher current loads than the stock Replicators can handle"
#else
#warning "Building with HEATERS_ON_STEROIDS defined will create firmware which allows ALL heaters to heatup at the same time; this requires a PSU, power connector, and associated electronics capable of handling much higher current loads than the stock Replicators can handle" 
#endif
#endif

#if !defined(HEATERS_ON_STEROIDS)
bool check_temp_state = false;
#endif
bool outstanding_tool_command = false;
enum PauseState paused = PAUSE_STATE_NONE;
static const prog_uchar *pauseErrorMessage = 0;
static bool coldPause = false;
bool heat_shutdown = false;

static Point pausedPosition;

volatile int32_t  pauseZPos = 0;
bool pauseAtZPosActivated = false;

int64_t filamentLength[2] = {0, 0};	//This maybe pos or neg, but ABS it and all is good (in steps)
int64_t lastFilamentLength[2] = {0, 0};
static int32_t lastFilamentPosition[2];

bool pauseUnRetract = false;

int16_t pausedPlatformTemp;
int16_t pausedExtruderTemp[2];
uint8_t pausedDigiPots[STEPPER_COUNT] = {0, 0, 0, 0, 0};
bool pausedFanState;

uint8_t buildPercentage = 101;
#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS) || defined(ESTIMATE_TIME)
uint32_t startingBuildTimeSeconds;
uint8_t startingBuildTimePercentage;
uint32_t elapsedSecondsSinceBuildStart;
#endif

#ifdef DITTO_PRINT
bool dittoPrinting = false;
#endif
bool deleteAfterUse = true;
uint16_t altTemp[EXTRUDERS];

#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
static uint8_t  home_command;
static bool     home_again;
static uint32_t home_feedrate;
static uint16_t home_timeout_s;
#endif

#if defined(AUTO_LEVEL)
static uint8_t alevel_state;
#endif

uint16_t getRemainingCapacity() {
	uint16_t sz;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sz = command_buffer.getRemainingCapacity();
	}
	return sz;
}


void displayStatusMessage( const prog_uchar msg1[], const prog_uchar msg2[] ) {
	Motherboard& board = Motherboard::getBoard();
	MessageScreen* scr = board.getMessageScreen();
	scr->clearMessage();
	scr->setXY(0,0);
	scr->addMessage(msg1);
	scr->addMessage(msg2);
	InterfaceBoard& ib = board.getInterfaceBoard();
	if (ib.getCurrentScreen() != scr) {
		ib.pushScreen(scr);
	} else {
		scr->refreshScreen();
	}
	scr->buttonsDisabled = true;
}


void removeStatusMessage(void) {
	Motherboard& board = Motherboard::getBoard();
	MessageScreen* scr = board.getMessageScreen();
	InterfaceBoard& ib = board.getInterfaceBoard();
	scr->buttonsDisabled = false;
	if (ib.getCurrentScreen() == scr) {
		ib.popScreen();
	}
	
	//Reset the underlying screen which is ActiveBuildMenu, so that it adds/removes
	//the filament menu as necessary
	ib.getCurrentScreen()->reset();
}

/// Returns the build percentage (0-100).  This is 101 is the build percentage hasn't been set yet

uint8_t getBuildPercentage(void) {
	return buildPercentage;
}

#if defined(AUTO_LEVEL)

void alevel_update(Point &newPoint) {

     // Return now if no attempt has been made to enable auto-leveling 
     if ( alevel_state == 0 ) return;

     Point currentPoint = steppers::getPlannerPosition();
     int32_t delta[3];

     delta[0] = newPoint[X_AXIS] - currentPoint[X_AXIS];
     delta[1] = newPoint[Y_AXIS] - currentPoint[Y_AXIS];
     delta[2] = newPoint[Z_AXIS] - currentPoint[Z_AXIS];

     // We only need to act if the X, Y, or Z coordinates change
     //   changes of A or B do not matter.  And some gcode likes
     //   to frequently reset the extruder position to 0

     if ( delta[0] != 0 || delta[1] != 0 || delta[2] != 0 ) {

	  if ( alevel_state & 8 )
	       // Auto-leveling was initialized successfully
	       //   update the transform
	       skew_update(delta);
	  else
	       // We were getting ready to initialize auto-leveling,
	       //   but hadn't completed the process AND we just
	       //   translated some combination of X, Y, or Z.
	       //   We need to wipe the auto-leveling state
	       //
	       // In theory, this shouldn't happen: gcode shouldn't
	       //   be doing something like this.  But the code is
	       //   here should it happen.
	       alevel_state = 0;
     }
}

#endif

static void heatersOff() {
     Motherboard::heatersOff(true);
     Motherboard::getBoard().setExtra(false);
}

static void cancelMidBuild() {

#ifdef HAS_FILAMENT_COUNTER
     // Save the used filament info
     addFilamentUsed();
#endif
     // Ensure that the heaters and fan are turned off
     heatersOff();

     // Disable the stepper motors
     steppers::enableAxes(0xff, false);

     // There's likely some command data still in the command buffer
     // If we don't flush it, it'll get executed causing the build
     // platform to "unclear" itself.
     command_buffer.reset();

     // And finally cancel the build
     host::stopBuild();
}

//Called when filament is extracted via the filament menu during a pause.
//It prevents noodle from being primed into the extruder on resume

void pauseUnRetractClear(void) {
	pauseUnRetract = false;
}

void pause(bool pause, bool cold) {
	if ( pause ) paused = (enum PauseState)PAUSE_STATE_ENTER_COMMAND;
	else         paused = (enum PauseState)PAUSE_STATE_EXIT_COMMAND;
	coldPause = cold;
#if defined(PSTOP_SUPPORT)
	pstop_triggered = false;
#endif
}

void heatShutdown(){
	heat_shutdown = true;
}

// Returns the pausing intent
uint8_t isPaused() {
	//If we're not paused, or we in an exiting state, then we are not
	//paused, or we are in the process of unpausing.
	if ( paused == PAUSE_STATE_NONE || paused & PAUSE_STATE_EXIT_COMMAND )
		return 0;
	else
		return coldPause ? 2 : 1;
}

// Returns the paused state
enum PauseState pauseState() {
	return paused;
}

//Returns true if we're transitioning between fully paused, or fully unpaused
bool pauseIntermediateState() {
	if (( paused == PAUSE_STATE_NONE ) || ( paused == PAUSE_STATE_PAUSED )) return false;
	return true;
}

void pauseAtZPos(int32_t zpos) {
        pauseZPos = zpos;

	//If we're already past the pause position, we might be paused, 
	//or homing, so we activate the pauseZPos later, when the Z position drops
	//below pauseZPos
	if ( steppers::getPlannerPosition()[2] >= pauseZPos )
		pauseAtZPosActivated = false;
	else	pauseAtZPosActivated = true;
}

int32_t getPauseAtZPos() {
        return pauseZPos;
}

bool isEmpty() {
	return command_buffer.isEmpty();
}

void push(uint8_t byte) {
	command_buffer.push(byte);
}

uint8_t pop8() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	return command_buffer.pop();
#pragma GCC diagnostic pop
}

int16_t pop16() {
	union {
		// AVR is little-endian
		int16_t a;
		struct {
			uint8_t data[2];
		} b;
	} shared;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
#pragma GCC diagnostic pop
	return shared.a;
}

int32_t pop32() {
	union {
		// AVR is little-endian
		int32_t a;
		struct {
			uint8_t data[4];
		} b;
	} shared;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
	shared.b.data[2] = command_buffer.pop();
	shared.b.data[3] = command_buffer.pop();
#pragma GCC diagnostic pop
	return shared.a;
}

enum ModeState {
	READY=0,
	MOVING,
	DELAY,
	HOMING,
	WAIT_ON_TOOL,
	WAIT_ON_PLATFORM,
	WAIT_ON_BUTTON
};

enum ModeState mode = READY;

Timeout delay_timeout;
Timeout homing_timeout;
Timeout tool_wait_timeout;
Timeout button_wait_timeout;
/// Bitmap of button pushes to wait for
uint8_t button_mask;
enum {
	BUTTON_TIMEOUT_CONTINUE = 0,
	BUTTON_TIMEOUT_ABORT = 1,
	BUTTON_CLEAR_SCREEN = 2
};
/// Action to take when button times out
uint8_t button_timeout_behavior;

void buildDone() {
#if defined(AUTO_LEVEL)
     alevel_state = 0;
     skew_deinit();
#endif
}

void buildReset() {
        pauseAtZPos(0);
	pauseAtZPosActivated = false;
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		pausedDigiPots[i] = 0;
	deleteAfterUse = true;

	sdCardError = false;

#if defined(LINE_NUMBER)
	line_number = 0;
#endif

#if !defined(HEATERS_ON_STEROIDS)
	check_temp_state = false;
#endif
	paused = PAUSE_STATE_NONE;
	pauseErrorMessage = 0;
	coldPause = false;
        filamentLength[0] = filamentLength[1] = 0;
        lastFilamentLength[0] = lastFilamentLength[1] = 0;
	lastFilamentPosition[0] = lastFilamentPosition[1] = 0;

#ifdef DITTO_PRINT
	if (( eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 2 ) && ( eeprom::getEeprom8(eeprom_offsets::DITTO_PRINT_ENABLED, 0) ))
		dittoPrinting = true;
	else	dittoPrinting = false;
#endif

#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS) || defined(ESTIMATE_TIME)
	startingBuildTimeSeconds = 0;
	startingBuildTimePercentage = 0;
	elapsedSecondsSinceBuildStart = 0;
#endif

#if defined(PSTOP_SUPPORT)
	pstop_triggered = false;
	pstop_move_count = 0;
	pstop_okay = false;
#endif

	altTemp[0] = 0;
#if EXTRUDERS > 1
	altTemp[1] = 0;
#endif
	pausedExtruderTemp[0] = 0;
#if EXTRUDERS > 1
	pausedExtruderTemp[1] = 0;
#endif

#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
	home_again = false;
#endif

#if defined(AUTO_LEVEL)
	alevel_state = 0;
	skew_deinit();
#endif
}

void reset() {
        buildReset();
	buildPercentage = 101;
	command_buffer.reset();
	mode = READY;
}

bool isWaiting() {
	return (mode == WAIT_ON_BUTTON);
}

bool isReady() {
    return (mode == READY);
}

#if defined(PSTOP_ZMIN_LEVEL)
void possibleZLevelPStop() {
     // Flag a PStop provided we are not homing and
     // we're far enough into the print that we're not sitting
     // against the Z endstop having just homed.
     if ( ( mode != HOMING ) && ( pstop_okay ) ) pstop_triggered = true;
}
#endif

#if defined(LINE_NUMBER)

uint32_t getLineNumber() {
	return line_number;	
}

void clearLineNumber() {
	line_number = 0;
}

#endif

//If retract is true, the filament is retracted by 1mm,
//if it's false, it's pushed back out by 1mm
void retractFilament(bool retract) {
	//Handle the unretract cancel
	if	( retract )		pauseUnRetract = true;
	else if ( ! pauseUnRetract )	return;

	Point targetPosition = steppers::getPlannerPosition();

	bool extrude_direction[EXTRUDERS] = {ACCELERATION_EXTRUDE_WHEN_NEGATIVE_A, ACCELERATION_EXTRUDE_WHEN_NEGATIVE_B};

	for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {
	    Heater& heater = Motherboard::getBoard().getExtruderBoard(e).getExtruderHeater();
	    if ( heater.get_set_temperature() != 0 && heater.has_reached_target_temperature() )
		targetPosition[A_AXIS + e] += (int32_t)(( extrude_direction[e] ^ retract ) ? -1 : 1) *
		    stepperAxisMMToSteps(PAUSE_RETRACT_FILAMENT_AMOUNT_MM, A_AXIS + e);
	}

	//Calculate the dda interval, we'll calculate for A and assume B is the same
	//Get the feedrate for A, we'll use the max_speed_change feedrate for A
	float retractFeedRateA = (float)eeprom::getEeprom16(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::MAX_SPEED_CHANGE + sizeof(uint16_t) * A_AXIS, DEFAULT_MAX_ACCELERATION_AXIS_A);

	int32_t dda_interval = (int32_t)(1000000.0 / (retractFeedRateA * (float)stepperAxis[A_AXIS].steps_per_mm));

#ifdef DITTO_PRINT
	if ( dittoPrinting ) {
		if ( currentToolIndex == 0 )	targetPosition[B_AXIS] = targetPosition[A_AXIS];
		else				targetPosition[A_AXIS] = targetPosition[B_AXIS];
	}
#endif

	steppers::setTargetNew(targetPosition, dda_interval, 0, 0);

	if (retract)
	     return;

	// Restore A and B position prior to pause.  Important for
	// when using absolute extruder positions in gcode
	Point currentPosition = steppers::getPlannerPosition();
	currentPosition[A_AXIS] = pausedPosition[A_AXIS];
#if EXTRUDERS > 1
	currentPosition[B_AXIS] = pausedPosition[B_AXIS];
#endif
	steppers::definePosition(currentPosition, false);
}

// Moves the Z platform to the bottom
// so that it clears the print if clearPlatform is true,
// otherwise restores the last position before platformAccess(true) was called

void platformAccess(bool clearPlatform) {
   Point currentPosition, targetPosition;

   if ( clearPlatform ) {
	//if we haven't defined a position or we haven't homed, then we
	//don't know our position, so it's unwise to attempt to clear the build
	//so we return doing nothing.
	for ( uint8_t i = 0; i <= Z_AXIS; i ++ ) {
                if (( ! stepperAxis[i].hasDefinePosition ) || ( ! stepperAxis[i].hasHomed ))
			return;
	}

        Point tmpPosition = pausedPosition;
	Point currentPosition = steppers::getPlannerPosition();

	//Position to clear the build area
#ifdef BUILD_CLEAR_X
        tmpPosition[0] = BUILD_CLEAR_X;
#endif

#ifdef BUILD_CLEAR_Y
        tmpPosition[1] = BUILD_CLEAR_Y;
#endif

#ifdef BUILD_CLEAR_Z
        tmpPosition[2] = BUILD_CLEAR_Z;
#endif

	// So as to not undo any retraction done prior
	tmpPosition[A_AXIS] = currentPosition[A_AXIS];
#if EXTRUDERS > 1
	tmpPosition[B_AXIS] = currentPosition[B_AXIS];
#endif

	// Subtract out the offsets
	targetPosition = steppers::removeOffsets(tmpPosition);

   } else {
        targetPosition = pausedPosition;

	//Extruders may have moved, so we use the current position
	//for them and define it 
	Point currentPosition = steppers::getPlannerPosition();

	steppers::definePosition(Point(currentPosition[0], currentPosition[1], currentPosition[2],
				       targetPosition[3], targetPosition[4]), false);
   }

   //Calculate the dda speed.  Somewhat crude but effective.  Use the Z
   //axis, it generally has the slowest feed rate
   int32_t dda_rate = (int32_t)(FPTOF(stepperAxis[Z_AXIS].max_feedrate) * (float)stepperAxis[Z_AXIS].steps_per_mm);

   // Calculate the distance
   currentPosition = steppers::getPlannerPosition();
   float dx = (float)(currentPosition[0] - targetPosition[0]) / (float)stepperAxis[X_AXIS].steps_per_mm;
   float dy = (float)(currentPosition[1] - targetPosition[1]) / (float)stepperAxis[Y_AXIS].steps_per_mm;
   float dz = (float)(currentPosition[2] - targetPosition[2]) / (float)stepperAxis[Z_AXIS].steps_per_mm;
   float distance = sqrtf(dx*dx + dy*dy + dz*dz);

#ifdef DITTO_PRINT
   if ( dittoPrinting ) {
	if ( currentToolIndex == 0 )	targetPosition[B_AXIS] = targetPosition[A_AXIS];
	else				targetPosition[A_AXIS] = targetPosition[B_AXIS];
   }
#endif

   // Don't let the platform clearing be sped up, otherwise Z steps may be skipped
   //   and then the resume after pause will be at the wrong height
   uint8_t as = steppers::alterSpeed;
   steppers::alterSpeed = 0;

   steppers::setTargetNewExt(targetPosition, dda_rate, (uint8_t)0, distance,
			     FPTOI16(stepperAxis[Z_AXIS].max_feedrate << 6));

   // Restore use of speed control
   steppers::alterSpeed = as;
}

//Adds the filament used during this build for a particular extruder
void addFilamentUsedForExtruder(uint8_t extruder) {
        //Need to do this to get the absolute amount
        int64_t fl = getFilamentLength(extruder);

        if ( fl > 0 ) {
		int16_t offset = eeprom_offsets::FILAMENT_LIFETIME + extruder * sizeof(int64_t);
                int64_t filamentUsed = eeprom::getEepromInt64(offset, 0);
                filamentUsed += fl;
                eeprom::setEepromInt64(offset, filamentUsed);

                //We've used it up, so reset it
                lastFilamentLength[extruder] = filamentLength[extruder];
                filamentLength[extruder] = 0;
        }
}

//Adds the filament used during this build
void addFilamentUsed() {
	addFilamentUsedForExtruder(0);	//A
	addFilamentUsedForExtruder(1);	//B
}

int64_t getFilamentLength(uint8_t extruder) {
        if ( filamentLength[extruder] < 0 )       return -filamentLength[extruder];
        return filamentLength[extruder];
}

int64_t getLastFilamentLength(uint8_t extruder) {
        if ( lastFilamentLength[extruder] < 0 )   return -lastFilamentLength[extruder];
        return lastFilamentLength[extruder];

}

float filamentUsed() {
	float filamentUsed = 
		stepperAxisStepsToMM(command::getLastFilamentLength(0), A_AXIS) +
		stepperAxisStepsToMM(command::getLastFilamentLength(1), B_AXIS) +
// When an extruder is turned off during the print, we end up 
// just reporting the lastFilamentLength
//	if ( filamentUsed == 0.0 )
//		filamentUsed =
			stepperAxisStepsToMM(command::getFilamentLength(0), A_AXIS) +
			stepperAxisStepsToMM(command::getFilamentLength(1), B_AXIS); 
	return filamentUsed;
}

// Saves the current digi pot settings, and switches them on high powered
// if power_high is true, otherwise low powered
void saveDigiPotsAndPower(bool power_high) {
	for (uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		if ( pausedDigiPots[i] != 0 )	continue;	//Protection against double saving

		pausedDigiPots[i] = steppers::getAxisPotValue(i);
		if ( power_high )	steppers::resetAxisPot(i);
		else			steppers::setAxisPotValue(i, 20);
	}
}

//Restores previously saved digit pot value.
//Assumes saveDigiPotAndPower has been called recently
void restoreDigiPots(void) {
	for (uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		steppers::setAxisPotValue(i, pausedDigiPots[i]);
		pausedDigiPots[i] = 0;
	}
}

static void pstop_incr() {
	if ( !pstop_okay && ++pstop_move_count > 4 ) pstop_okay = true;
}

// Handle movement comands -- called from a few places
static void handleMovementCommand(const uint8_t &command) {
        // Motherboard::getBoard().resetUserInputTimeout();  // call already made by our caller
	if (command == HOST_CMD_QUEUE_POINT_EXT) {
		// check for completion
		if (command_buffer.getLength() >= 25) {
			pop8(); // remove the command code
			mode = MOVING;

			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t dda = pop32();

#ifdef DITTO_PRINT
   			if ( dittoPrinting ) {
				if ( currentToolIndex == 0 )	b = a;
				else				a = b;
			}
#endif

	                filamentLength[0] += (int64_t)(a - lastFilamentPosition[0]);
			filamentLength[1] += (int64_t)(b - lastFilamentPosition[1]);
			lastFilamentPosition[0] = a;
			lastFilamentPosition[1] = b;
			LINE_NUMBER_INCR;
#if defined(PSTOP_SUPPORT)
			pstop_incr();
#endif
			steppers::setTargetNew(Point(x,y,z,a,b), dda, 0, 0);
		}
	}
	 else if (command == HOST_CMD_QUEUE_POINT_NEW) {
		// check for completion
		if (command_buffer.getLength() >= 26) {
			pop8(); // remove the command code
			mode = MOVING;

			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t us = pop32();
			uint8_t relative = pop8();

#ifdef DITTO_PRINT
   			if ( dittoPrinting ) {
				if ( currentToolIndex == 0 ) {
					b = a;

					//Set B to be the same as A
					relative &= ~(_BV(B_AXIS));
					if ( relative & _BV(A_AXIS) )	relative |= _BV(B_AXIS);
				} else {
					a = b;

					//Set A to be the same as B
					relative &= ~(_BV(A_AXIS));
					if ( relative & _BV(B_AXIS) )	relative |= _BV(A_AXIS);
				}
			}
#endif

			int32_t ab[2] = {a,b};

			for ( int i = 0; i < 2; i ++ ) {
				if ( relative & (1 << (A_AXIS + i))) {
					filamentLength[i] += (int64_t)ab[i];
					lastFilamentPosition[i] += ab[i];
				} else {
					filamentLength[i] += (int64_t)(ab[i] - lastFilamentPosition[i]);
					lastFilamentPosition[i] = ab[i];
				}
			}

			LINE_NUMBER_INCR;
#if defined(PSTOP_SUPPORT)
			pstop_incr();
#endif
			steppers::setTargetNew(Point(x,y,z,a,b), 0, us, relative);
		}
	}
	else if (command == HOST_CMD_QUEUE_POINT_NEW_EXT ) {
		// check for completion
		if (command_buffer.getLength() >= 32) {
			pop8(); // remove the command code
			mode = MOVING;

			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t dda_rate = pop32();
			uint8_t relative = pop8() & 0x7F; // make sure that the high bit is clear
			int32_t distanceInt32 = pop32();
			float *distance = (float *)&distanceInt32;
			int16_t feedrateMult64 = pop16();

#ifdef DITTO_PRINT
   			if ( dittoPrinting ) {
				if ( currentToolIndex == 0 ) {
					b = a;

					//Set B to be the same as A
					relative &= ~(_BV(B_AXIS));
					if ( relative & _BV(A_AXIS) )	relative |= _BV(B_AXIS);
				} else {
					a = b;

					//Set A to be the same as B
					relative &= ~(_BV(A_AXIS));
					if ( relative & _BV(B_AXIS) )	relative |= _BV(A_AXIS);
				}
			}
#endif

			int32_t ab[2] = {a,b};

			for ( int i = 0; i < 2; i ++ ) {
				if ( relative & (1 << (A_AXIS + i))) {
					filamentLength[i] += (int64_t)ab[i];
					lastFilamentPosition[i] += ab[i];
				} else {
					filamentLength[i] += (int64_t)(ab[i] - lastFilamentPosition[i]);
					lastFilamentPosition[i] = ab[i];
				}
			}

			LINE_NUMBER_INCR;
#if defined(PSTOP_SUPPORT)
			// Positions must be known at this point; okay to do a pstop and
			// its attendant platform clearing
			pstop_incr();
#endif
			steppers::setTargetNewExt(Point(x,y,z,a,b), dda_rate,
						  relative | steppers::alterSpeed,
						  *distance, feedrateMult64);
		}
	}
}

//If overrideToolIndex = -1, the toolIndex specified in the packet is used, otherwise
//the toolIndex specified by overrideToolIndex is used

bool processExtruderCommandPacket(int8_t overrideToolIndex) {
	Motherboard& board = Motherboard::getBoard();
		//command_buffer[0] is the command code, i.e. HOST_CMD_TOOL_COMMAND

                //Handle the tool index and override it if we need to
                uint8_t toolIndex = command_buffer[1];
                if ( overrideToolIndex != -1 )  toolIndex = (uint8_t)overrideToolIndex;

		uint8_t command = command_buffer[2];
		//command_buffer[3] - Payload length

		int16_t temp;

		switch (command) {
		case SLAVE_CMD_SET_TEMP:
			temp = (int16_t)command_buffer[4] + (int16_t)( command_buffer[5] << 8 );
			if ( temp == 0 ) addFilamentUsed();

			/// Handle override gcode temp
			if (( temp ) && ( altTemp[toolIndex] ||
					   (eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, DEFAULT_OVERRIDE_GCODE_TEMP)) ))
			    temp = altTemp[toolIndex] ? (int16_t)altTemp[toolIndex] : eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + toolIndex * sizeof(int16_t), DEFAULT_PREHEAT_TEMP);

#ifdef DEBUG_NO_HEAT_NO_WAIT
			temp  = 0;
#endif

			board.getExtruderBoard(toolIndex).getExtruderHeater().set_target_temperature(temp);

#if !defined(HEATERS_ON_STEROIDS)
			/// if platform is actively heating and extruder is not cooling down, pause extruder
			if( board.getPlatformHeater().isHeating() &&
			    !board.getPlatformHeater().isCooling() &&
			    !board.getExtruderBoard(toolIndex).getExtruderHeater().isCooling() ){
				check_temp_state = true;
				board.getExtruderBoard(toolIndex).getExtruderHeater().Pause(true);
			}  /// else ensure extruder is not paused  
			else {
				board.getExtruderBoard(toolIndex).getExtruderHeater().Pause(false);
			}
#endif
			BOARD_STATUS_CLEAR(Motherboard::STATUS_PREHEATING);
			return true;
		// can be removed in process via host query works OK
 		case SLAVE_CMD_PAUSE_UNPAUSE:
			host::pauseBuild(command::isPaused() == 0, false);
			return true;
		case SLAVE_CMD_TOGGLE_FAN:
			board.getExtruderBoard(toolIndex).setFan((command_buffer[4] & 0x01) != 0);
			return true;
		case SLAVE_CMD_TOGGLE_VALVE:
		        board.setExtra((command_buffer[4] & 0x01) != 0);
			return true;
		case SLAVE_CMD_SET_PLATFORM_TEMP:
			if ( !eeprom::hasHBP() ) return true;
			temp = (int16_t)command_buffer[4] + (int16_t)( command_buffer[5] << 8 );
#ifdef DEBUG_NO_HEAT_NO_WAIT
			temp = 0;
#endif
			/// Handle override gcode temp
			if (( temp ) && ( eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0) )) {
				temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_TEMP, 100);
			}

			board.getPlatformHeater().set_target_temperature(temp);

			// If we're setting the platform temp to 0 (off) then it's tempting to
			// just bail here.  However, it may be that the platform was previously
			// turned on and the extruders paused and now it's being turned off....
			// so go ahead and consider the platform as being used and handle the
			// check_temp_state flag.
			board.setUsingPlatform(true);
#if !defined(HEATERS_ON_STEROIDS)
			// pause extruder heaters platform is heating up
			bool pause_state; /// avr-gcc doesn't allow cross-initializtion of variables within a switch statement
			pause_state = !board.getPlatformHeater().isCooling();
			check_temp_state = pause_state;
			Motherboard::pauseHeaters(pause_state);
#else
#warning "Building with HEATERS_ON_STEROIDS defined; all heaters allowed to run concurrently"
#endif
			BOARD_STATUS_CLEAR(Motherboard::STATUS_PREHEATING);			
			return true;
        // not being used with 5D
		case SLAVE_CMD_TOGGLE_MOTOR_1:
		case SLAVE_CMD_TOGGLE_MOTOR_2: 
		case SLAVE_CMD_SET_MOTOR_1_PWM:
		case SLAVE_CMD_SET_MOTOR_2_PWM:
		case SLAVE_CMD_SET_MOTOR_1_DIR:
		case SLAVE_CMD_SET_MOTOR_2_DIR:
		case SLAVE_CMD_SET_MOTOR_1_RPM:
		case SLAVE_CMD_SET_MOTOR_2_RPM:
		case SLAVE_CMD_SET_SERVO_1_POS:
		case SLAVE_CMD_SET_SERVO_2_POS:
			//command_buffer[4]
			return true;
		}
	return false;
}

//Handle the pause state

void handlePauseState(void) {
   switch ( paused ) {
	case PAUSE_STATE_ENTER_START_PIPELINE_DRAIN:
		//We've entered a pause, start draining the pipeline
		if ( host::getBuildState() != host::BUILD_CANCELLING && host::getBuildState() != host::BUILD_CANCELED )
			displayStatusMessage(PAUSE_ENTER_MSG, PAUSE_DRAINING_PIPELINE_MSG);
		paused = PAUSE_STATE_ENTER_WAIT_PIPELINE_DRAIN;

		//If we're heating, the digi pots might be turned down, save them and
		//turn the pots to full on
		saveDigiPotsAndPower(true);
		break;	

	case PAUSE_STATE_ENTER_WAIT_PIPELINE_DRAIN:
		//Wait for the pipeline to drain
		if (movesplanned() == 0) {
			paused = PAUSE_STATE_ENTER_START_RETRACT_FILAMENT;
		}
		break;

	case PAUSE_STATE_ENTER_START_RETRACT_FILAMENT:
		//Store the current position so we can return later
		pausedPosition = steppers::getPlannerPosition();

		//Retract the filament by 1mm to prevent blobbing
		retractFilament(true);
		paused = PAUSE_STATE_ENTER_WAIT_RETRACT_FILAMENT;
		break;

	case PAUSE_STATE_ENTER_WAIT_RETRACT_FILAMENT:
		//Wait for the filament retraction to complete
		if (movesplanned() == 0) {
			paused = PAUSE_STATE_ENTER_START_CLEARING_PLATFORM;
		}
		break;

	case PAUSE_STATE_ENTER_START_CLEARING_PLATFORM:
		//Clear the platform
		platformAccess(true);
		paused = PAUSE_STATE_ENTER_WAIT_CLEARING_PLATFORM;
		{
		    bool cancelling = ( host::getBuildState() == host::BUILD_CANCELLING ) || ( host::getBuildState() == host::BUILD_CANCELED );

		    Motherboard& board = Motherboard::getBoard();

		    // Turn the fan off
		    pausedFanState = EX_FAN.getValue();
		    board.setExtra(false);

		    //Store the current heater temperatures for restoring later
		    pausedExtruderTemp[0] = (int16_t)board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
		    pausedExtruderTemp[1] = (int16_t)board.getExtruderBoard(1).getExtruderHeater().get_set_temperature();
		    pausedPlatformTemp    = (int16_t)board.getPlatformHeater().get_set_temperature();

		    //If we're pausing, and we have HEAT_DURING_PAUSE switched off, switch off the heaters
		    //if (( ! cancelling ) && ( ! (eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE) )))
		    if ( coldPause || !(eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, DEFAULT_HEAT_DURING_PAUSE)) )
			heatersOff();
		    if ( coldPause ) {
#ifdef HAS_RGB_LED
			    RGB_LED::setColor(0, 0, 0, true);
#endif
			    steppers::enableAxes(0xf8, false);
		    }

		    if ( pauseErrorMessage )
			    displayStatusMessage((sdCardError) ? CANCELLING_ENTER_MSG : PAUSE_ENTER_MSG,  pauseErrorMessage );
		    else
			    displayStatusMessage((cancelling) ? CANCELLING_ENTER_MSG : PAUSE_ENTER_MSG, PAUSE_CLEARING_BUILD_MSG);
		}
		break;

	case PAUSE_STATE_ENTER_WAIT_CLEARING_PLATFORM:
		//We finished the last command, now we wait for the platform to reach the bottom
		//before entering the pause
		if (movesplanned() == 0) {
			restoreDigiPots();
			paused = ( sdCardError ) ? PAUSE_STATE_ERROR : PAUSE_STATE_PAUSED;
			removeStatusMessage();
			if ( host::getBuildState() != host::BUILD_CANCELLING && host::getBuildState() != host::BUILD_CANCELED )
			    Piezo::playTune(TUNE_PAUSE);
		}
		break;

	case PAUSE_STATE_EXIT_START_PLATFORM_HEATER:
	{
		//We've begun to exit the pause, instruct the platform heater to resume it's set point

		//We switch digi pots to low during heating
		saveDigiPotsAndPower(false);

		Motherboard& board = Motherboard::getBoard();

		if ( pausedPlatformTemp > 0 ) {
			board.getPlatformHeater().Pause(false);
			board.getPlatformHeater().set_target_temperature(pausedPlatformTemp);
		}

		int16_t temp = altTemp[0] ? (int16_t)altTemp[0] : pausedExtruderTemp[0];
		if ( temp > 0 ) {
			board.getExtruderBoard(0).getExtruderHeater().Pause(false);
			board.getExtruderBoard(0).getExtruderHeater().set_target_temperature(temp);
#if !defined(HEATERS_ON_STEROIDS)
			if ( pausedPlatformTemp > 0 ) 
				board.getExtruderBoard(0).getExtruderHeater().Pause(true);
#endif
		}
		temp = altTemp[1] ? (int16_t)altTemp[1] : pausedExtruderTemp[1];
		if ( temp > 0 ) {
			board.getExtruderBoard(1).getExtruderHeater().Pause(false);
			board.getExtruderBoard(1).getExtruderHeater().set_target_temperature(temp);
#if !defined(HEATERS_ON_STEROIDS)
			if ( pausedPlatformTemp > 0 )
				board.getExtruderBoard(1).getExtruderHeater().Pause(true);
#endif
		}

		paused = PAUSE_STATE_EXIT_WAIT_FOR_PLATFORM_HEATER;
		break;
	}

	case PAUSE_STATE_EXIT_WAIT_FOR_PLATFORM_HEATER:
		//Waiting for the platform heater to reach it's set point
		if (( pausedPlatformTemp > 0 ) && ( ! Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature() ))
			break;
		paused = PAUSE_STATE_EXIT_WAIT_FOR_TOOLHEAD_HEATERS;
		break;

	case PAUSE_STATE_EXIT_WAIT_FOR_TOOLHEAD_HEATERS:
		if (( pausedExtruderTemp[0] > 0 ) && ( ! Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().has_reached_target_temperature() ))
			break;
		if (( pausedExtruderTemp[1] > 0 ) && ( ! Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().has_reached_target_temperature() ))
			break;

		restoreDigiPots();

		paused = PAUSE_STATE_EXIT_START_RETURNING_PLATFORM;
		break;

	case PAUSE_STATE_EXIT_START_RETURNING_PLATFORM:
		//Instruct the platform to return to it's pre pause position

		//If we're heating, the digi pots might be turned down, save them and switch them high
		saveDigiPotsAndPower(true);

		platformAccess(false);
		paused = PAUSE_STATE_EXIT_WAIT_RETURNING_PLATFORM;
		displayStatusMessage(PAUSE_LEAVE_MSG, PAUSE_RESUMING_POSITION_MSG);
		break;

	case PAUSE_STATE_EXIT_WAIT_RETURNING_PLATFORM:
		//Wait for the platform to finish moving to it's prepause position
		if (movesplanned() == 0)
			paused = PAUSE_STATE_EXIT_START_UNRETRACT_FILAMENT;
		break;
	
	case PAUSE_STATE_EXIT_START_UNRETRACT_FILAMENT:
		retractFilament(false);
		paused = PAUSE_STATE_EXIT_WAIT_UNRETRACT_FILAMENT;
		break;

	case PAUSE_STATE_EXIT_WAIT_UNRETRACT_FILAMENT:
		//Wait for the filament unretraction to finish
		//then resume processing commands
		if (movesplanned() == 0) {
		        Motherboard::getBoard().setExtra(pausedFanState);
			restoreDigiPots();
			removeStatusMessage();
			paused = PAUSE_STATE_NONE;
			pauseErrorMessage = 0;
#if defined(PSTOP_SUPPORT)
			pstop_triggered = false;
#endif
		}
		break;

        case PAUSE_STATE_ERROR:
		removeStatusMessage();
		// Need to set incomplete true so that isWaiting() will be true for the
		// message screen.  That, in turn, keeps InterfaceBoard::doUpdate() from
		// clearing the message when it sees that the build has finished.
	        Motherboard::getBoard().errorResponse(pauseErrorMessage, false, true);
		sdcard::finishPlayback();
	        pauseErrorMessage = 0;
		sdCardError = false;
	        paused = PAUSE_STATE_NONE;
	        break;

	default:
		break;
   }
}

// A fast slice for processing commands and refilling the stepper queue, etc.
void runCommandSlice() {

    // get command from SD card if building from SD
    if ( sdcard::isPlaying() ) {
	while (command_buffer.getRemainingCapacity() > 0 && sdcard::playbackHasNext()) {
	    command_buffer.push(sdcard::playbackNext());
	}

	// Deal with any end of file conditions
	if( !sdcard::playbackHasNext() ) {

	    // SD card file is finished.  Was it a normal finish or an error?
	    //  Check the pause state; otherwise, we can hit this code once
	    //  and start a pause with host::stopBuild() and then re-enter
	    //  this code again at which point host::stopBuild() will then
	    //  do an immediate cancel.  Alternatively, call finishPlayback()
	    //  so that sdcard::isPlaying() is then false.

	    if ( sdcard::sdAvailable != sdcard::SD_SUCCESS && paused == PAUSE_STATE_NONE ) {

		// SD card error of some sort
		sdCardError = true;

		// Establish an error message to display while cancelling the build
		if ( sdcard::sdAvailable == sdcard::SD_ERR_NO_CARD_PRESENT ) pauseErrorMessage = NOCARD_MSG;
		else if ( sdcard::sdAvailable == sdcard::SD_ERR_CRC ) pauseErrorMessage = CARDCRC_MSG;
		else pauseErrorMessage = CARDERROR_MSG;

		// Now cancel the build
		cancelMidBuild();
	    }
	    else if ( !pauseErrorMessage ) {
		 sdcard::finishPlayback();
	    }
	}
    }

    // get command from onboard script if building from onboard
	if(utility::isPlaying()){		
		while (command_buffer.getRemainingCapacity() > 0 && utility::playbackHasNext()){
			command_buffer.push(utility::playbackNext());
		}
		if(!utility::playbackHasNext() && command_buffer.isEmpty()){
			utility::finishPlayback();
		}
	}

#if !defined(HEATERS_ON_STEROIDS)
	// if printer is not waiting for tool or platform to heat, we need to make
	// sure the extruders are not in a paused state.  this is relevant when 
	// heating using the control panel in desktop software
	if(check_temp_state){
		if (Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature()){
			// unpause extruder heaters in case they are paused
			Motherboard::pauseHeaters(false);
			check_temp_state = false;
		}
	}
#endif

	//If we were previously past the z pause position (e.g. homing, entering a value during a pause)
	//then we need to activate the z pause when the z position falls below it's value
	if (( pauseZPos ) && ( ! pauseAtZPosActivated ) && ( steppers::getPlannerPosition()[2] < pauseZPos)) {
		pauseAtZPosActivated = true;
	}

        //If we've reached Pause @ ZPos, then pause
        if ((( pauseZPos ) && ( pauseAtZPosActivated ) && ( isPaused() == 0 ) && ( steppers::getPlannerPosition()[2]) >= pauseZPos )) {
		pauseAtZPos(0);		//Clear the pause at zpos
                host::pauseBuild(true, false);
		return;
	}

	if (( paused != PAUSE_STATE_NONE && paused != PAUSE_STATE_PAUSED )) {
		handlePauseState();
		return;	
	}

	// don't execute commands if paused or shutdown because of heater failure
	// If paused and heaters are on, shut them off
	if ( paused == PAUSE_STATE_PAUSED ) {
	    if ( coldPause || heat_shutdown || !Motherboard::getBoard().user_input_timeout.hasElapsed() ) return;
	    coldPause = true;
	    heatersOff();
#ifdef HAS_RGB_LED
	    RGB_LED::setColor(0, 0, 0, true);
#endif
	    steppers::enableAxes(0xf8, false);
	    return;
	}

	if ( mode == HOMING ) {
	     if ( !steppers::isRunning() ) {
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
		  if ( home_again ) {
		       home_again = false;
		       homing_timeout.start(home_timeout_s * 1000L * 1000L);
		       steppers::startHoming(home_command==HOST_CMD_FIND_AXES_MAXIMUM,
					     (1 << Y_AXIS),
					     home_feedrate);
		  }
		  else
#endif
		       mode = READY;
	     }
	     else if ( homing_timeout.hasElapsed() ) {
		  steppers::abort();
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
		  home_again = false;
#endif
		  mode = READY;
	     }
	}

#if defined(PSTOP_SUPPORT)
	// We don't act on the PSTOP when we are homing or are paused
	if ( pstop_triggered && pstop_okay && mode != HOMING && paused == PAUSE_STATE_NONE ) {
		if ( !isPaused() )
		{
			const static PROGMEM prog_uchar pstop_msg[] = "P-Stop triggered";
			pauseErrorMessage = pstop_msg;
			host::pauseBuild(true, true);
		}
		pstop_triggered = false;
	}
#endif

	if ( mode == MOVING ) {
		if ( !steppers::isRunning() )
			mode = READY;
	}

	if ( mode == DELAY ) {
		// check timers
		if ( delay_timeout.hasElapsed() )
			mode = READY;
	}

	if ( mode == WAIT_ON_TOOL ) {
		if ( tool_wait_timeout.hasElapsed() ) {
			Motherboard::getBoard().errorResponse(EXTRUDER_TIMEOUT_MSG);
			mode = READY;		
		}
		else if ( !Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().isHeating() ) {
#ifdef DITTO_PRINT
			if ( dittoPrinting ) {
				if ( !Motherboard::getBoard().getExtruderBoard((currentToolIndex == 0 ) ? 1 : 0).getExtruderHeater().isHeating() )
					mode = READY;
			}
			else
#endif
				mode = READY;
		}
		else if ( Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().has_reached_target_temperature() ) {
#ifdef DITTO_PRINT
			if ( dittoPrinting ) {
				if ( Motherboard::getBoard().getExtruderBoard((currentToolIndex == 0) ? 1 : 0).getExtruderHeater().has_reached_target_temperature() )
            				mode = READY;
			}
			else 
#endif
				mode = READY;
		}
	}

	if ( mode == WAIT_ON_PLATFORM ) {
		if ( tool_wait_timeout.hasElapsed() ) {
			Motherboard::getBoard().errorResponse(PLATFORM_TIMEOUT_MSG);
			mode = READY;		
		}
		else if ( !Motherboard::getBoard().getPlatformHeater().isHeating() ||
			  Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature() )
			mode = READY;
	}

	if ( mode == WAIT_ON_BUTTON ) {
		if ( button_wait_timeout.hasElapsed() ) {
			if ( button_timeout_behavior & (1 << BUTTON_TIMEOUT_ABORT) )
				// Abort build!
				// We'll interpret this as a catastrophic situation
				// and do a full reset of the machine.
				Motherboard::getBoard().reset(false);
			else {
				mode = READY;
			//	Motherboard::getBoard().interfaceBlink(0,0);
				BOARD_STATUS_CLEAR(Motherboard::STATUS_WAITING_FOR_BUTTON);
			}
		}
		else {
			// Check buttons
			InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
			if ( ib.buttonPushed( )) {			
				if ( button_timeout_behavior & (1 << BUTTON_CLEAR_SCREEN) )
					ib.popScreen();
				Motherboard::interfaceBlinkOff();
				BOARD_STATUS_CLEAR(Motherboard::STATUS_WAITING_FOR_BUTTON);
#ifdef HAS_RGB_LED
				RGB_LED::setDefaultColor();
#endif
				mode = READY;
			}
		}
	}

	if ( mode == READY ) {
		//
		// process next command on the queue.
		//
		if ((command_buffer.getLength() > 0)){
			Motherboard::getBoard().resetUserInputTimeout();
			
			uint8_t command = command_buffer[0];

			//If we're running acceleration, we want to populate the pipeline buffer,
			//but we also need to sync (wait for the pipeline buffer to clear) on certain
			//commands, we do that here
			//If we're not pipeline'able command, then we sync here,
			//by waiting for the pipeline buffer to empty before continuing
			if ((command != HOST_CMD_QUEUE_POINT_EXT) &&
 			    (command != HOST_CMD_QUEUE_POINT_NEW) &&
			    (command != HOST_CMD_QUEUE_POINT_NEW_EXT ) &&
			    (command != HOST_CMD_ENABLE_AXES ) &&
			    (command != HOST_CMD_CHANGE_TOOL ) &&
			    (command != HOST_CMD_SET_POSITION_EXT) &&
			    (command != HOST_CMD_SET_ACCELERATION_TOGGLE) &&
			    (command != HOST_CMD_RECALL_HOME_POSITION) &&
			    (command != HOST_CMD_FIND_AXES_MINIMUM) &&
			    (command != HOST_CMD_FIND_AXES_MAXIMUM) &&
			    (command != HOST_CMD_TOOL_COMMAND) && 
			    (command != HOST_CMD_PAUSE_FOR_BUTTON )) {
       	                         if ( ! st_empty() )     return;
       	                 }

		if (command == HOST_CMD_QUEUE_POINT_EXT || command == HOST_CMD_QUEUE_POINT_NEW ||
		     command == HOST_CMD_QUEUE_POINT_NEW_EXT ) {
					handleMovementCommand(command);
			}  else if (command == HOST_CMD_CHANGE_TOOL) {
				if (command_buffer.getLength() >= 2) {
					pop8(); // remove the command code
                    currentToolIndex = pop8();
                    LINE_NUMBER_INCR;
                    
                    steppers::changeToolIndex(currentToolIndex);
				}
			} else if (command == HOST_CMD_ENABLE_AXES) {
				if (command_buffer.getLength() >= 2) {
					pop8(); // remove the command code
					uint8_t axes = pop8();
					LINE_NUMBER_INCR;

#ifdef DITTO_PRINT
					if ( dittoPrinting ) {
						if ( currentToolIndex == 0 ) {
							//Set B to be the same as A
							axes &= ~(_BV(B_AXIS));
							if ( axes & _BV(A_AXIS) )	axes |= _BV(B_AXIS);
						} else {
							//Set A to be the same as B
							axes &= ~(_BV(A_AXIS));
							if ( axes & _BV(B_AXIS) )	axes |= _BV(A_AXIS);
						}
					}
#endif
					steppers::enableAxes(axes, (axes & 0x80) != 0);
				}
			} else if (command == HOST_CMD_SET_POSITION_EXT) {
				// check for completion
				if (command_buffer.getLength() >= 21) {
					pop8(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					int32_t a = pop32();
					int32_t b = pop32();

#ifdef DITTO_PRINT
					if ( dittoPrinting ) {
						if ( currentToolIndex == 0 )	b = a;
						else				a = b;
					}
#endif

					lastFilamentPosition[0] = a;
					lastFilamentPosition[1] = b;
					LINE_NUMBER_INCR;

					Point newPoint = Point(x,y,z,a,b);
#if defined(AUTO_LEVEL)
					alevel_update(newPoint);
#endif	
					steppers::definePosition(newPoint, false);
				}
			} else if (command == HOST_CMD_DELAY) {
				if (command_buffer.getLength() >= 5) {
					mode = DELAY;
					pop8(); // remove the command code
					// parameter is in milliseconds; timeouts need microseconds
					uint32_t microseconds = pop32() * 1000L;
					LINE_NUMBER_INCR;
					
					delay_timeout.start(microseconds);
				}
			} else if (command == HOST_CMD_PAUSE_FOR_BUTTON) {
				if (command_buffer.getLength() >= 5) {
					pop8(); // remove the command code
					button_mask = pop8();
					uint16_t timeout_seconds = pop16();
					button_timeout_behavior = pop8();
					LINE_NUMBER_INCR;
					
					if (timeout_seconds != 0) {
						button_wait_timeout.start(timeout_seconds * 1000L * 1000L);
					} else {
						button_wait_timeout = Timeout();
					}
                    // set button wait via interface board
					Motherboard::interfaceBlinkOn();
					InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
					ib.waitForButton(button_mask);
					BOARD_STATUS_SET(Motherboard::STATUS_WAITING_FOR_BUTTON);
					mode = WAIT_ON_BUTTON;
				}
			} else if (command == HOST_CMD_DISPLAY_MESSAGE) {
				MessageScreen* scr = Motherboard::getBoard().getMessageScreen();
				if (command_buffer.getLength() >= 6) {
					pop8(); // remove the command code
					uint8_t options = pop8();
					uint8_t xpos = pop8();
					uint8_t ypos = pop8();
					uint8_t timeout_seconds = pop8();
					LINE_NUMBER_INCR;
					
                    // check message clear bit
					if ( (options & (1 << 0)) == 0 ) { scr->clearMessage(); }
					// set position and add message
					scr->setXY(xpos,ypos);
					scr->addMessage(command_buffer); 
					
					// push message screen if the full message has been recieved
					if((options & (1 << 1))){          
						InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
						if (ib.getCurrentScreen() != scr) {
							ib.pushScreen(scr);
						} else {
							scr->refreshScreen();
						}
						// set message timeout if not a buttonWait call
						if ((timeout_seconds != 0) && (!(options & (1 <<2)))) {
								scr->setTimeout(timeout_seconds);//, true);
						}
						
						if (options & (1 << 2)) { // button wait bit --> start button wait
							if (timeout_seconds != 0) {
								button_wait_timeout.start(timeout_seconds * 1000L * 1000L);
							} else {
								button_wait_timeout = Timeout();
							}
							button_mask = (1 << ButtonArray::CENTER);  // center button
							button_timeout_behavior &= (1 << BUTTON_CLEAR_SCREEN);
							Motherboard::interfaceBlinkOn();
							InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
							ib.waitForButton(button_mask);
							BOARD_STATUS_SET(Motherboard::STATUS_WAITING_FOR_BUTTON);
							mode = WAIT_ON_BUTTON;
						}
					}
				}
			} else if (command == HOST_CMD_FIND_AXES_MINIMUM ||
				   command == HOST_CMD_FIND_AXES_MAXIMUM) {
				if (command_buffer.getLength() >= 8) {
					pop8(); // remove the command
					uint8_t flags = pop8();
					uint32_t feedrate = pop32(); // feedrate in us per step
					uint16_t timeout_s = pop16();
					LINE_NUMBER_INCR;

#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
					if (((1 << X_AXIS) | (1 << Y_AXIS)) == (flags & ((1 << X_AXIS) | (1 << Y_AXIS)))) {
					     flags &= ~(1 << Y_AXIS);
					     home_again     = true;
					     home_command   = command;
					     home_feedrate  = feedrate;
					     home_timeout_s = timeout_s;
					}
					else
					     home_again = false;
#endif
					//bool direction = command == HOST_CMD_FIND_AXES_MAXIMUM;
#if defined(PSTOP_OKAY)
					// Helpful at end of print
					// Especially with filament detectors
					// with timeouts that are fooled by
					// end gcode which homes, sends Z
					// to bottom, then plays a song.
					pstop_okay = false;
#endif
					mode = HOMING;
					homing_timeout.start(timeout_s * 1000L * 1000L);
					steppers::startHoming(command==HOST_CMD_FIND_AXES_MAXIMUM,
							flags,
							feedrate);
				}
			} else if (command == HOST_CMD_WAIT_FOR_TOOL) {
				if (command_buffer.getLength() >= 6) {
#ifdef DEBUG_NO_HEAT_NO_WAIT
					mode = READY;
#else
					mode = WAIT_ON_TOOL;
#endif
#if defined(PSTOP_SUPPORT)
					// Assume that by now coordinates are set
					pstop_okay = true;
#endif
					pop8();
					currentToolIndex = pop8();
					pop16();	//uint16_t toolPingDelay
					uint16_t toolTimeout = (uint16_t)pop16();
					LINE_NUMBER_INCR;
					// if we re-add handling of toolTimeout, we need to make sure
					// that values that overflow our counter will not be passed)
					tool_wait_timeout.start(toolTimeout*1000000L);
				}
			} else if (command == HOST_CMD_WAIT_FOR_PLATFORM) {
        // FIXME: Almost equivalent to WAIT_FOR_TOOL
				if (command_buffer.getLength() >= 6) {
#ifdef DEBUG_NO_HEAT_NO_WAIT
					mode = READY;
#else
					mode = WAIT_ON_PLATFORM;
#endif
#if defined(PSTOP_SUPPORT)
					// Assume that by now coordinates are set
					pstop_okay = true;
#endif
					pop8();
					pop8();	//uint8_t currentToolIndex
					pop16(); //uint16_t toolPingDelay
					uint16_t toolTimeout = (uint16_t)pop16();
					LINE_NUMBER_INCR;
					// if we re-add handling of toolTimeout, we need to make sure
					// that values that overflow our counter will not be passed)
					tool_wait_timeout.start(toolTimeout*1000000L);
				}
			} else if (command == HOST_CMD_STORE_HOME_POSITION) {

				// check for completion
				if (command_buffer.getLength() >= 2) {
					pop8();
					uint8_t axes = pop8();
					LINE_NUMBER_INCR;
					
					// Go through each axis, and if that axis is specified, read it's value,
					// then record it to the eeprom.
					Point currentPoint = steppers::getPlannerPosition();
#if !defined(AUTO_LEVEL)
					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
#else
					for (uint8_t i = 0; i <= Z_AXIS; i++) {
#endif
						if ( axes & (1 << i) ) {
						     uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + i * 4;
							uint32_t position = currentPoint[i];
							cli();
							eeprom_write_block(&position, (void*) offset, 4);
							sei();
						}
					}
#if defined(AUTO_LEVEL)
					// Trigger only for A, B, or A & B
					// Do not trigger if any of X, Y, or Z was specified
					if ((axes) && (axes == (axes & ((1 << A_AXIS) | (1 << B_AXIS))))) {
					     uint8_t idx;
					     switch(axes) {
					     case (1 << A_AXIS) : idx = 0; break;
					     case (1 << B_AXIS) : idx = 1; break;
					     default: idx = 2; break;
					     }
					     alevel_state |= 1 << idx;
					     int32_t position[3];
					     position[0] = currentPoint[X_AXIS];
					     position[1] = currentPoint[Y_AXIS];
					     position[2] = currentPoint[Z_AXIS];
					     cli();
					     eeprom_write_block(
						  position,
						  (char *)eeprom_offsets::ALEVEL_P1 + idx * 3 * sizeof(int32_t),
						  3 * sizeof(int32_t));
					     sei();
					}
#endif
				}
			} else if (command == HOST_CMD_RECALL_HOME_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 2) {
					pop8();
					uint8_t axes = pop8();
					LINE_NUMBER_INCR;

					Point newPoint = steppers::getPlannerPosition();
#if defined(AUTO_LEVEL)
					// Trigger only for A & B
					// Do not trigger if any of X, Y, or Z was specified
					if ( axes == ((1 << A_AXIS) | (1 << B_AXIS)) ) {
					     // M131 AB -- initialize and enable skew
					     // alevel_state must have bits 0, 1, and 2 set
					     uint8_t alevel_valid = 1;
					     if ( 7 == (alevel_state & 7) ) {
						  // Attempt to enable auto-level
						  auto_level_t alevel_data;
						  cli();
						  eeprom_read_block(&alevel_data, (void *)eeprom_offsets::ALEVEL_FLAGS,
								    sizeof(alevel_data));
						  sei();
						  if ( alevel_data.max_zdelta <= 0 )
						       alevel_data.max_zdelta = ALEVEL_MAX_ZDELTA_DEFAULT;
						  if ( skew_init(alevel_data.max_zdelta,
								 alevel_data.p1, alevel_data.p2,
								 alevel_data.p3) ) {
						       alevel_valid = 0;
						       alevel_state |= 8;
						  }
						  else
						       alevel_valid = 2;
					     }
					     cli();
					     eeprom_write_byte((uint8_t *)eeprom_offsets::ALEVEL_FLAGS,
							       alevel_valid ? 1 : 0);
					     sei();
					     if ( alevel_valid ) {
						  // Cancel the build!
						  if ( alevel_valid == 1 )
						       pauseErrorMessage = ALEVEL_INCOMPLETE_MSG;
						  else
						       pauseErrorMessage = ( skew_status() == ALEVEL_COLINEAR )
							    ? ALEVEL_COLINEAR_MSG : ALEVEL_BADLEVEL_MSG;

						  // Now cancel the build
						  cancelMidBuild();
					     }
					}
					else {
#endif
					for (uint8_t i = 0; i <= Z_AXIS; i++) {
						if ( axes & (1 << i) ) {
							uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + 4*i;
							cli();
							eeprom_read_block(&(newPoint[i]), (void*) offset, 4);
							sei();
						}
					}

					lastFilamentPosition[0] = newPoint[A_AXIS];
					lastFilamentPosition[1] = newPoint[B_AXIS];
#if defined(AUTO_LEVEL)
					// If we've jumped through all the hoops and successfully initialized
					// auto-leveling, then we need to update the skew transform as we may be
					// translating our X, Y, or Z coordinates.
					alevel_update(newPoint);
#endif
					steppers::definePosition(newPoint, true);
#if defined(AUTO_LEVEL)
					}
#endif
				}


			}else if (command == HOST_CMD_SET_POT_VALUE){
				if (command_buffer.getLength() >= 3) {
					pop8(); // remove the command code
					uint8_t axis = pop8();
					uint8_t value = pop8();
					LINE_NUMBER_INCR;
                    steppers::setAxisPotValue(axis, value);
				}
			}else if (command == HOST_CMD_SET_RGB_LED){
				if (command_buffer.getLength() >= 6) {
					pop8(); // remove the command code

#ifdef HAS_RGB_LED
					uint8_t red = pop8();
					uint8_t green = pop8();
					uint8_t blue = pop8();
#else
					pop8();
					pop8();
					pop8();
#endif
					pop8(); // uint8_t blink_rate = pop8();
					pop8();	//uint8_t effect
					LINE_NUMBER_INCR;
					// RGB_LED::setLEDBlink(blink_rate);
#ifdef HAS_RGB_LED
					RGB_LED::setCustomColor(red, green, blue);
#endif

				}
			}else if (command == HOST_CMD_SET_BEEP){
				if (command_buffer.getLength() >= 6) {
					pop8(); // remove the command code
					uint16_t frequency= pop16();
					uint16_t beep_length = pop16();
					pop8();	//uint8_t effect
					LINE_NUMBER_INCR;
                    Piezo::setTone(frequency, beep_length);

				}			
			}else if (command == HOST_CMD_TOOL_COMMAND) {
				if (command_buffer.getLength() >= 4) { // needs a payload
					uint8_t payload_length = command_buffer[3];
					if (command_buffer.getLength() >= 4U+payload_length) {
#ifdef DITTO_PRINT
							if ( dittoPrinting ) {
								//Delete after use toggles, so that
								//when deleteAfterUse = false, it's the 1st call of the extruder command
								//and we copy to the other extruder.  When true, it's the 2nd call if the
								//extruder command, and we use the tool index specified in the command
								if ( deleteAfterUse )	deleteAfterUse = false;
								else			deleteAfterUse = true;
							} else
#endif
								deleteAfterUse = true;  //ELSE


							//If we're not setting a temperature, or toggling a fan, then we don't
							//"ditto print" the command, so we delete after use
							if (( command_buffer[2] != SLAVE_CMD_SET_TEMP ) &&
							    ( command_buffer[2] != SLAVE_CMD_TOGGLE_FAN ))
								deleteAfterUse = true;

							//If we're copying this command due to ditto printing, then we need to switch
							//the extruder controller by switching toolindex to the other extruder
							int8_t overrideToolIndex = -1;
							if ( ! deleteAfterUse ) {
								if ( command_buffer[1] == 0 )	overrideToolIndex = 1;
								else				overrideToolIndex = 0;
							}

							processExtruderCommandPacket(overrideToolIndex);

							//Delete the packet from the buffer
							if ( deleteAfterUse ) {
								//We start from 1 not 0, because byte 0 was already removed in the pop8
								//above
								for ( uint8_t i = 0; i < (4U + payload_length); i ++ )
									pop8();

								LINE_NUMBER_INCR;
							}
				}
			}
			} else if (command == HOST_CMD_SET_BUILD_PERCENT){
				if (command_buffer.getLength() >= 3){
					pop8(); // remove the command code
					buildPercentage = pop8();
					pop8();	// uint8_t ignore; // remove the reserved byte
					LINE_NUMBER_INCR;
#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS) || defined(ESTIMATE_TIME)
					//Set the starting time / percent on the first HOST_CMD_SET_BUILD_PERCENT
					//with a non zero value sent near the start of the build
					//We use this to calculate the build time
					if (( buildPercentage > 0 ) && ( startingBuildTimeSeconds == 0) && ( startingBuildTimePercentage == 0 )) {
						startingBuildTimeSeconds = host::getPrintSeconds();
						startingBuildTimePercentage = buildPercentage;
					}
					if ( buildPercentage > 0 ) {
						elapsedSecondsSinceBuildStart = host::getPrintSeconds();
					}
#endif
				}
			} else if (command == HOST_CMD_QUEUE_SONG ) //queue a song for playing
 			{
				/// Error tone is 0,
				/// End tone is 1,
				/// all other tones user-defined (defaults to end-tone)
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					uint8_t songId = pop8();
					LINE_NUMBER_INCR;
					if(songId == 0)
						Piezo::errorTone(4);
					else if (songId == 1 )
						Piezo::playTune(TUNE_PRINT_DONE);
					else
						Piezo::errorTone(2);
				}
#if defined(PSTOP_SUPPORT)
				// Helpful at end of print
				// Especially with filament detectors
				// with timeouts that are fooled by
				// end gcode which homes, sends Z
				// to bottom, then plays a song.
				pstop_okay = false;
#endif

			} else if ( command == HOST_CMD_RESET_TO_FACTORY) {
				/// reset EEPROM settings to the factory value. Reboot bot.
				if (command_buffer.getLength() >= 2){
				pop8(); // remove the command code
				pop8();	//uint8_t options
				LINE_NUMBER_INCR;
				eeprom::factoryResetEEPROM();
				Motherboard::getBoard().reset(false);
				}
			} else if ( command == HOST_CMD_BUILD_START_NOTIFICATION) {
				if (command_buffer.getLength() >= 5){
					pop8(); // remove the command code
					pop32();	//int buildSteps
					LINE_NUMBER_INCR;
					host::handleBuildStartNotification(command_buffer);
#if defined(PSTOP_SUPPORT)
					pstop_okay = false;
#endif
				}
			} else if ( command == HOST_CMD_BUILD_END_NOTIFICATION) {
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					uint8_t flags = pop8();
					LINE_NUMBER_INCR;
					host::handleBuildStopNotification(flags);
#if defined(PSTOP_SUPPORT)
					pstop_okay = false;
#endif
				}
			} else if ( command == HOST_CMD_SET_ACCELERATION_TOGGLE) {
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					LINE_NUMBER_INCR;
					uint8_t status = pop8();
					steppers::setSegmentAccelState(status == 1);
				}
			} else if ( command == HOST_CMD_STREAM_VERSION ) {
			        if ( command_buffer.getLength() >= 21 ) {
      
					pop8();// remove the command code
					// stream number
					uint8_t version_high = pop8();
					uint8_t version_low = pop8();

					if ( (version_high *100 + version_low) != stream_version ) {
						Motherboard::getBoard().errorResponse(ERROR_STREAM_VERSION);
					}
					// extra version
					pop8();
					// checksum (currently not implemented)
					pop32();
					uint16_t bot_type = pop16();
					// extra bytes
					if ( bot_type != BOT_TYPE ) Motherboard::getBoard().errorResponse(ERROR_BOT_TYPE);

					// eleven extra bytes
					pop16();
					pop32();
					pop32();
					pop8();
					LINE_NUMBER_INCR;    
				}
			} else if ( command == HOST_CMD_PAUSE_AT_ZPOS ) {
				if (command_buffer.getLength() >= 5){
					pop8(); // remove the command code
					LINE_NUMBER_INCR;
					int32_t zPosInt32 = pop32();
					float *zPos = (float *)&zPosInt32;

					//Exclude negative zpos's and nearly zero zpos's
					if ( *zPos < 0.001 )	*zPos = 0.0;
					pauseAtZPos(stepperAxisMMToSteps(*zPos, Z_AXIS));
				}
		        } else {
		        }
		}
	}

#if defined(LINE_NUMBER)
	/// we're not handling overflows in the line counter.  Possibly implement this later.
       if ( line_number > MAX_LINE_COUNT )
		line_number = MAX_LINE_COUNT + 1;
#endif
}

#if defined(MODEL_REPLICATOR2) || defined(BUILD_STATS) || defined(ESTIMATE_TIME)

//Returns the estimated time left for the build in seconds
//If we can't complete the calculation due to a lack of information, then we return 0

int32_t estimatedTimeLeftInSeconds(void) {
	//Safety guard against insufficient information, we return 0 if this is the case
	if (( buildPercentage == 101 ) || ( buildPercentage == 0 ) ||
	    ( buildPercentage == startingBuildTimePercentage ) ||
	    (startingBuildTimePercentage == 0 ) || (elapsedSecondsSinceBuildStart == 0))
		return 0;

	//The build time is not calculated from the start of the build, it's calculated from the first non zero build
	//percentage update sent in the .s3g or from the host
	float timeLeft = ((float)elapsedSecondsSinceBuildStart / (float)(buildPercentage - startingBuildTimePercentage)) * (100.0 - (float)buildPercentage); 
	
	//Safe guard against negative results
	if ( timeLeft < 0.0 )	timeLeft = 0.0;

	return (int32_t)timeLeft;
}

#endif

}
