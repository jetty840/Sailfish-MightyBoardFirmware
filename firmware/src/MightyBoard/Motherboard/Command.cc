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
#include "RGB_LED.hh"
#include "Interface.hh"
#include "UtilityScripts.hh"
#include "stdio.h"
#include "Menu_locales.hh"

namespace command {

#define COMMAND_BUFFER_SIZE 512
uint8_t buffer_data[COMMAND_BUFFER_SIZE];
CircularBuffer command_buffer(COMMAND_BUFFER_SIZE, buffer_data);
uint8_t currentToolIndex = 0;

uint32_t line_number;

bool outstanding_tool_command = false;
bool check_temp_state = false;
enum PauseState paused = PAUSE_STATE_NONE;
bool heat_shutdown = false;
uint32_t sd_count = 0; static Point pausedPosition;

volatile int32_t  pauseZPos = 0;
bool pauseAtZPosActivated = false;

int64_t filamentLength[2] = {0, 0};	//This maybe pos or neg, but ABS it and all is good (in steps)
int64_t lastFilamentLength[2] = {0, 0};
static int32_t lastFilamentPosition[2];

bool pauseUnRetract = false;

int16_t pausedPlatformTemp;
int16_t pausedExtruderTemp[2];
uint8_t pausedDigiPots[STEPPER_COUNT] = {0, 0, 0, 0, 0};

uint8_t buildPercentage = 101;

#ifdef DITTO_PRINT
	bool dittoPrinting = false;
	bool deleteAfterUse = true;
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

//Called when filament is extracted via the filament menu during a pause.
//It prevents noodle from being primed into the extruder on resume

void pauseUnRetractClear(void) {
	pauseUnRetract = false;
}

void pause(bool pause) {
	if ( pause )	paused = (enum PauseState)PAUSE_STATE_ENTER_COMMAND;
	else		paused = (enum PauseState)PAUSE_STATE_EXIT_COMMAND;
}

void heatShutdown(){
	heat_shutdown = true;
}

// Returns the pausing intent
bool isPaused() {
	//If we're not paused, or we in an exiting state, then we are not
	//paused, or we are in the process of unpausing.
	if ( paused == PAUSE_STATE_NONE || paused & PAUSE_STATE_EXIT_COMMAND )
		return false;
	return true;
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
//	sd_count ++;
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
//	sd_count+=2;
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
//	sd_count+=4;
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


bool sdcard_reset = false;

void reset() {
	buildPercentage = 101;
        pauseAtZPos(0);
	pauseAtZPosActivated = false;
	deleteAfterUse = true;
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		pausedDigiPots[i] = 0;

	command_buffer.reset();
	line_number = 0;
	check_temp_state = false;
	paused = PAUSE_STATE_NONE;
	sd_count = 0;
	sdcard_reset = false;
        filamentLength[0] = filamentLength[1] = 0;
        lastFilamentLength[0] = lastFilamentLength[1] = 0;
	lastFilamentPosition[0] = lastFilamentPosition[1] = 0;

#ifdef DITTO_PRINT
	if (( eeprom::getEeprom8(eeprom_offsets::TOOL_COUNT, 1) == 2 ) && ( eeprom::getEeprom8(eeprom_offsets::DITTO_PRINT_ENABLED, 0) ))
		dittoPrinting = true;
	else	dittoPrinting = false;
#endif

	mode = READY;
}

bool isWaiting(){
	return (mode == WAIT_ON_BUTTON);
}
    
bool isReady() {
    return (mode == READY);
}

uint32_t getLineNumber() {
	return line_number;	
}

void clearLineNumber() {
	line_number = 0;
}

//If retract is true, the filament is retracted by 1mm,
//if it's false, it's pushed back out by 1mm
void retractFilament(bool retract) {
	//Handle the unretract cancel
	if	( retract )		pauseUnRetract = true;
	else if ( ! pauseUnRetract )	return;

	Point targetPosition = steppers::getPlannerPosition();

	bool extrude_direction[EXTRUDERS] = {ACCELERATION_EXTRUDE_WHEN_NEGATIVE_A, ACCELERATION_EXTRUDE_WHEN_NEGATIVE_B};

	for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {
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

	steppers::setTarget(targetPosition, dda_interval);
}

// Moves the Z platform to the bottom
// so that it clears the print if clearPlatform is true,
// otherwise restores the last position before platformAccess(true) was called

void platformAccess(bool clearPlatform) {
   Point targetPosition;

   if ( clearPlatform ) {
	//if we haven't defined a position or we haven't homed, then we
	//don't know our position, so it's unwise to attempt to clear the build
	//so we return doing nothing.
	for ( uint8_t i = 0; i <= Z_AXIS; i ++ ) {
                if (( ! stepperAxis[i].hasDefinePosition ) || ( ! stepperAxis[i].hasHomed ))
			return;
	}

        targetPosition = pausedPosition;

	//Position to clear the build area
#ifdef BUILD_CLEAR_X
        targetPosition[0] = BUILD_CLEAR_X;
#endif

#ifdef BUILD_CLEAR_Y
        targetPosition[1] = BUILD_CLEAR_Y;
#endif

#ifdef BUILD_CLEAR_Z
        targetPosition[2] = BUILD_CLEAR_Z;
#endif

   } else {
        targetPosition = pausedPosition;

	//Extruders may have moved, so we use the current position
	//for them and define it 
	Point currentPosition = steppers::getPlannerPosition();

	steppers::definePosition(Point(currentPosition[0], currentPosition[1], currentPosition[2],
					targetPosition[3], targetPosition[4]));
   }

   //Calculate the dda speed.  Somewhat crude but effective.  Use the Z
   //axis, it generally has the slowest feed rate
   int32_t dda_interval = (int32_t)(1000000.0 / (stepperAxis[Z_AXIS].max_feedrate * (float)stepperAxis[Z_AXIS].steps_per_mm));

#ifdef DITTO_PRINT
   if ( dittoPrinting ) {
	if ( currentToolIndex == 0 )	targetPosition[B_AXIS] = targetPosition[A_AXIS];
	else				targetPosition[A_AXIS] = targetPosition[B_AXIS];
   }
#endif

   steppers::setTarget(targetPosition, dda_interval);
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

// Handle movement comands -- called from a few places
static void handleMovementCommand(const uint8_t &command) {
	if (command == HOST_CMD_QUEUE_POINT_EXT) {
		// check for completion
		if (command_buffer.getLength() >= 25) {
			Motherboard::getBoard().resetUserInputTimeout();
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

			line_number++;

			steppers::setTarget(Point(x,y,z,a,b), dda);
		}
	}
	 else if (command == HOST_CMD_QUEUE_POINT_NEW) {
		// check for completion
		if (command_buffer.getLength() >= 26) {
			Motherboard::getBoard().resetUserInputTimeout();
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

			line_number++;
			
			steppers::setTargetNew(Point(x,y,z,a,b), us, relative);
		}
	}
	else if (command == HOST_CMD_QUEUE_POINT_NEW_EXT ) {
		// check for completion
		if (command_buffer.getLength() >= 32) {
			Motherboard::getBoard().resetUserInputTimeout();
			pop8(); // remove the command code
			mode = MOVING;

			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t dda_rate = pop32();
			uint8_t relative = pop8();
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

			line_number++;
			
			steppers::setTargetNewExt(Point(x,y,z,a,b), dda_rate, relative, *distance, feedrateMult64);
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

		int16_t *temp;

		switch (command) {
		case SLAVE_CMD_SET_TEMP:
			temp = (int16_t *)&command_buffer[4];
			if ( *temp == 0 )	addFilamentUsed();

			/// Handle override gcode temp
			if (( *temp ) && ( eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0) )) {
				*temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + toolIndex * sizeof(int16_t), 220);
			}

#ifdef DEBUG_NO_HEAT_NO_WAIT
			*temp  = 0;
#endif

			board.getExtruderBoard(toolIndex).getExtruderHeater().set_target_temperature(*temp);
			/// if platform is actively heating and extruder is not cooling down, pause extruder
			if(board.getPlatformHeater().isHeating() && !board.getPlatformHeater().isCooling() && !board.getExtruderBoard(toolIndex).getExtruderHeater().isCooling()){
				check_temp_state = true;
				board.getExtruderBoard(toolIndex).getExtruderHeater().Pause(true);
			}  /// else ensure extruder is not paused  
			else {
				board.getExtruderBoard(toolIndex).getExtruderHeater().Pause(false);
			}

			return true;
		// can be removed in process via host query works OK
 		case SLAVE_CMD_PAUSE_UNPAUSE:
			host::pauseBuild(!command::isPaused());
			return true;
		case SLAVE_CMD_TOGGLE_FAN:
			{
			uint8_t fanCmd = command_buffer[4];
			board.getExtruderBoard(toolIndex).setFan((fanCmd & 0x01) != 0);
			}
			return true;
		case SLAVE_CMD_TOGGLE_VALVE:
			board.setValve((command_buffer[4] & 0x01) != 0);
			return true;
		case SLAVE_CMD_SET_PLATFORM_TEMP:
			board.setUsingPlatform(true);

			temp = (int16_t *)&command_buffer[4];

#ifdef DEBUG_NO_HEAT_NO_WAIT
			*temp = 0;
#endif

			/// Handle override gcode temp
			if (( *temp ) && ( eeprom::getEeprom8(eeprom_offsets::OVERRIDE_GCODE_TEMP, 0) )) {
				*temp = eeprom::getEeprom16(eeprom_offsets::PREHEAT_SETTINGS + preheat_eeprom_offsets::PREHEAT_PLATFORM_OFFSET, 100);
			}

			board.getPlatformHeater().set_target_temperature(*temp);
			// pause extruder heaters platform is heating up
			bool pause_state; /// avr-gcc doesn't allow cross-initializtion of variables within a switch statement
			pause_state = false;
			if(!board.getPlatformHeater().isCooling()){
				pause_state = true;
			}
			check_temp_state = pause_state;
			board.getExtruderBoard(0).getExtruderHeater().Pause(pause_state);
			board.getExtruderBoard(1).getExtruderHeater().Pause(pause_state);
			
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
		{	//Bracket to stop compiler complaining
		//Clear the platform
		platformAccess(true);
		paused = PAUSE_STATE_ENTER_WAIT_CLEARING_PLATFORM;
		bool cancelling = false;
		if ( host::getBuildState() == host::BUILD_CANCELLING || host::getBuildState() == host::BUILD_CANCELED )
			cancelling = true;

		Motherboard& board = Motherboard::getBoard();

		//Store the current heater temperatures for restoring later
		pausedExtruderTemp[0] = (int16_t)board.getExtruderBoard(0).getExtruderHeater().get_set_temperature();
		pausedExtruderTemp[1] = (int16_t)board.getExtruderBoard(1).getExtruderHeater().get_set_temperature();
		pausedPlatformTemp    = (int16_t)board.getPlatformHeater().get_set_temperature();

		//If we're pausing, and we have HEAT_DURING_PAUSE switched off, switch off the heaters
		if (( ! cancelling ) && ( ! (eeprom::getEeprom8(eeprom_offsets::HEAT_DURING_PAUSE, 1) ))) {
			board.getExtruderBoard(0).getExtruderHeater().set_target_temperature(0);
			board.getExtruderBoard(1).getExtruderHeater().set_target_temperature(0);
			board.getPlatformHeater().set_target_temperature(0);
		}

		displayStatusMessage((cancelling) ? CANCELLING_ENTER_MSG : PAUSE_ENTER_MSG, PAUSE_CLEARING_BUILD_MSG);
		}
		break;

	case PAUSE_STATE_ENTER_WAIT_CLEARING_PLATFORM:
		//We finished the last command, now we wait for the platform to reach the bottom
		//before entering the pause
		if (movesplanned() == 0) {
			restoreDigiPots();
			removeStatusMessage();
			if ( host::getBuildState() != host::BUILD_CANCELLING && host::getBuildState() != host::BUILD_CANCELED )
				Piezo::playTune(TUNE_PAUSE);
			paused = PAUSE_STATE_PAUSED;
		}
		break;

	case PAUSE_STATE_EXIT_START_PLATFORM_HEATER:
		//We've begun to exit the pause, instruct the platform heater to resume it's set point

		//We switch digi pots to low during heating
		saveDigiPotsAndPower(false);

		if ( pausedPlatformTemp > 0 ) {
			Motherboard& board = Motherboard::getBoard();
			board.getPlatformHeater().Pause(false);
			board.getPlatformHeater().set_target_temperature(pausedPlatformTemp);
		}
		paused = PAUSE_STATE_EXIT_WAIT_FOR_PLATFORM_HEATER;
		break;

	case PAUSE_STATE_EXIT_WAIT_FOR_PLATFORM_HEATER:
		//Waiting for the platform heater to reach it's set point
		if (( pausedPlatformTemp > 0 ) && ( ! Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature() ))
			break;
		paused = PAUSE_STATE_EXIT_START_TOOLHEAD_HEATERS;
		break;

	case PAUSE_STATE_EXIT_START_TOOLHEAD_HEATERS:
		//Instruct the toolhead heaters to resume their set points
		if ( pausedExtruderTemp[0] > 0 ) {
			Motherboard& board = Motherboard::getBoard();
			board.getExtruderBoard(0).getExtruderHeater().Pause(false);
			board.getExtruderBoard(0).getExtruderHeater().set_target_temperature(pausedExtruderTemp[0]);
		}
		if ( pausedExtruderTemp[1] > 0 ) {
			Motherboard& board = Motherboard::getBoard();
			board.getExtruderBoard(1).getExtruderHeater().Pause(false);
			board.getExtruderBoard(1).getExtruderHeater().set_target_temperature(pausedExtruderTemp[1]);
		}
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
		if (movesplanned() == 0) {
			paused = PAUSE_STATE_EXIT_START_UNRETRACT_FILAMENT;
		}
		break;
	
	case PAUSE_STATE_EXIT_START_UNRETRACT_FILAMENT:
		retractFilament(false);
		paused = PAUSE_STATE_EXIT_WAIT_UNRETRACT_FILAMENT;
		break;

	case PAUSE_STATE_EXIT_WAIT_UNRETRACT_FILAMENT:
		//Wait for the filament unretraction to finish
		//then resume processing commands
		if (movesplanned() == 0) {
			restoreDigiPots();
			removeStatusMessage();
			paused = PAUSE_STATE_NONE;
		}
		break;

	default:
		break;
   }
}

// A fast slice for processing commands and refilling the stepper queue, etc.
void runCommandSlice() {

    // get command from SD card if building from SD
	if (sdcard::isPlaying()) {
		while (command_buffer.getRemainingCapacity() > 0 && sdcard::playbackHasNext()) {
			sd_count++;
			command_buffer.push(sdcard::playbackNext());
		}
		if(!sdcard::playbackHasNext() && (sd_count < sdcard::getFileSize()) && !sdcard_reset){
			
			Motherboard::getBoard().getInterfaceBoard().resetLCD();
			Motherboard::getBoard().errorResponse(STATICFAIL_MSG);
			sdcard_reset = true;
			/// do the sd card initialization files
			//command_buffer.reset();
			//sdcard::startPlayback(host::getBuildName());
			//uint32_t count;
			//while(count < sd_count){
			//	sdcard::playbackNext();
			//}
		}else if(!sdcard::playbackHasNext() && command_buffer.isEmpty()){
			sdcard::finishPlayback();
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
	
	// if printer is not waiting for tool or platform to heat, we need to make
	// sure the extruders are not in a paused state.  this is relevant when 
	// heating using the control panel in desktop software
	if(check_temp_state){
		if (Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature()){
			// unpause extruder heaters in case they are paused
			Motherboard::getBoard().getExtruderBoard(0).getExtruderHeater().Pause(false);
			Motherboard::getBoard().getExtruderBoard(1).getExtruderHeater().Pause(false);
			check_temp_state = false;
		}
	}

	//If we were previously past the z pause position (e.g. homing, entering a value during a pause)
	//then we need to activate the z pause when the z position falls below it's value
	if (( pauseZPos ) && ( ! pauseAtZPosActivated ) && ( steppers::getPlannerPosition()[2] < pauseZPos)) {
		pauseAtZPosActivated = true;
	}

        //If we've reached Pause @ ZPos, then pause
        if ((( pauseZPos ) && ( pauseAtZPosActivated ) && ( ! isPaused() ) && ( steppers::getPlannerPosition()[2]) >= pauseZPos )) {
		pauseAtZPos(0);		//Clear the pause at zpos
                host::pauseBuild(true);
		return;
	}

	if (( paused != PAUSE_STATE_NONE && paused != PAUSE_STATE_PAUSED )) {
		handlePauseState();
		return;	
	}

	// don't execute commands if paused or shutdown because of heater failure
	if ((paused == PAUSE_STATE_PAUSED) || heat_shutdown) {	return; }

	if (mode == HOMING) {
		if (!steppers::isRunning()) {
			mode = READY;
		} else if (homing_timeout.hasElapsed()) {
			steppers::abort();
			mode = READY;
		}
	}
	if (mode == MOVING) {
		if (!steppers::isRunning()) {
			mode = READY;
		}
	}
	if (mode == DELAY) {
		// check timers
		if (delay_timeout.hasElapsed()) {
			mode = READY;
		}
	}

	if (mode == WAIT_ON_TOOL) {
		if(tool_wait_timeout.hasElapsed()){
		     Motherboard::getBoard().errorResponse(EXTRUDER_TIMEOUT_MSG);
			mode = READY;		
		}
		else if(!Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().isHeating()){
#ifdef DITTO_PRINT
			if ( dittoPrinting ) {
				if (!Motherboard::getBoard().getExtruderBoard((currentToolIndex == 0 ) ? 1 : 0).getExtruderHeater().isHeating())
					mode = READY;
			}
			else 
#endif
				mode = READY;
		}else if( Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().has_reached_target_temperature()){
#ifdef DITTO_PRINT
			if ( dittoPrinting ) {
				if (Motherboard::getBoard().getExtruderBoard((currentToolIndex == 0) ? 1 : 0).getExtruderHeater().has_reached_target_temperature())
            				mode = READY;
			}
			else 
#endif
				mode = READY;
		}
	}
	if (mode == WAIT_ON_PLATFORM) {
		if(tool_wait_timeout.hasElapsed()){
		     Motherboard::getBoard().errorResponse(PLATFORM_TIMEOUT_MSG);
			mode = READY;		
		} else if (!Motherboard::getBoard().getPlatformHeater().isHeating()){
			mode = READY;
		}
		else if(Motherboard::getBoard().getPlatformHeater().has_reached_target_temperature()){
            mode = READY;
		}
	}
	if (mode == WAIT_ON_BUTTON) {
		if (button_wait_timeout.hasElapsed()) {
			if (button_timeout_behavior & (1 << BUTTON_TIMEOUT_ABORT)) {
				// Abort build!
				// We'll interpret this as a catastrophic situation
				// and do a full reset of the machine.
				Motherboard::getBoard().reset(false);

			} else {
				mode = READY;
			//	Motherboard::getBoard().interfaceBlink(0,0);
			}
		} else {
			// Check buttons
			InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
			if (ib.buttonPushed()) {			
				if(button_timeout_behavior & (1 << BUTTON_CLEAR_SCREEN))
					ib.popScreen();
				Motherboard::getBoard().interfaceBlink(0,0);
				RGB_LED::setDefaultColor();
				mode = READY;
			}
		}
	}

	if (mode == READY) {
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
                    line_number++;
                    
                    steppers::changeToolIndex(currentToolIndex);
				}
			} else if (command == HOST_CMD_ENABLE_AXES) {
				if (command_buffer.getLength() >= 2) {
					pop8(); // remove the command code
					uint8_t axes = pop8();
					line_number++;

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

					bool enable = (axes & 0x80) != 0;
					for (int i = 0; i < STEPPER_COUNT; i++) {
						if ((axes & _BV(i)) != 0) {
							steppers::enableAxis(i, enable);
						}
					}
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
					line_number++;
					
					steppers::definePosition(Point(x,y,z,a,b));
				}
			} else if (command == HOST_CMD_DELAY) {
				if (command_buffer.getLength() >= 5) {
					mode = DELAY;
					pop8(); // remove the command code
					// parameter is in milliseconds; timeouts need microseconds
					uint32_t microseconds = pop32() * 1000L;
					line_number++;
					
					delay_timeout.start(microseconds);
				}
			} else if (command == HOST_CMD_PAUSE_FOR_BUTTON) {
				if (command_buffer.getLength() >= 5) {
					pop8(); // remove the command code
					button_mask = pop8();
					uint16_t timeout_seconds = pop16();
					button_timeout_behavior = pop8();
					line_number++;
					
					if (timeout_seconds != 0) {
						button_wait_timeout.start(timeout_seconds * 1000L * 1000L);
					} else {
						button_wait_timeout = Timeout();
					}
                    // set button wait via interface board
					Motherboard::getBoard().interfaceBlink(25,15);
					InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
					ib.waitForButton(button_mask);
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
					line_number++;
					
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
							Motherboard::getBoard().interfaceBlink(25,15);
							InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
							ib.waitForButton(button_mask);
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
					line_number++;
					
					//bool direction = command == HOST_CMD_FIND_AXES_MAXIMUM;
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
					pop8();
					currentToolIndex = pop8();
					pop16();	//uint16_t toolPingDelay
					uint16_t toolTimeout = (uint16_t)pop16();
					line_number++;
					
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
					pop8();
					pop8();	//uint8_t currentToolIndex
					pop16(); //uint16_t toolPingDelay
					uint16_t toolTimeout = (uint16_t)pop16();
					line_number++;
					
					// if we re-add handling of toolTimeout, we need to make sure
					// that values that overflow our counter will not be passed)
					tool_wait_timeout.start(toolTimeout*1000000L);
				}
			} else if (command == HOST_CMD_STORE_HOME_POSITION) {

				// check for completion
				if (command_buffer.getLength() >= 2) {
					pop8();
					uint8_t axes = pop8();
					line_number++;
					
					// Go through each axis, and if that axis is specified, read it's value,
					// then record it to the eeprom.
					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
						if ( axes & (1 << i) ) {
							uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + 4*i;
							uint32_t position = steppers::getPlannerPosition()[i];
							cli();
							eeprom_write_block(&position, (void*) offset, 4);
							sei();
						}
					}
				}
			} else if (command == HOST_CMD_RECALL_HOME_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 2) {
					pop8();
					uint8_t axes = pop8();
					line_number++;

					Point newPoint = steppers::getPlannerPosition();

					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
						if ( axes & (1 << i) ) {
							uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + 4*i;
							cli();
							eeprom_read_block(&(newPoint[i]), (void*) offset, 4);
							sei();
						}
					}

					lastFilamentPosition[0] = newPoint[3];
					lastFilamentPosition[1] = newPoint[4];

					steppers::definePosition(newPoint);
				}

			}else if (command == HOST_CMD_SET_POT_VALUE){
				if (command_buffer.getLength() >= 3) {
					pop8(); // remove the command code
					uint8_t axis = pop8();
					uint8_t value = pop8();
					line_number++;
                    steppers::setAxisPotValue(axis, value);
				}
			}else if (command == HOST_CMD_SET_RGB_LED){
				if (command_buffer.getLength() >= 6) {
					pop8(); // remove the command code

					uint8_t red = pop8();
					uint8_t green = pop8();
					uint8_t blue = pop8();
					uint8_t blink_rate = pop8();

                    pop8();	//uint8_t effect
                    line_number++;
                    RGB_LED::setLEDBlink(blink_rate);
                    RGB_LED::setCustomColor(red, green, blue);

				}
			}else if (command == HOST_CMD_SET_BEEP){
				if (command_buffer.getLength() >= 6) {
					pop8(); // remove the command code
					uint16_t frequency= pop16();
					uint16_t beep_length = pop16();
					pop8();	//uint8_t effect
					line_number++;
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

								line_number++;
							}
				}
			}
			} else if (command == HOST_CMD_SET_BUILD_PERCENT){
				if (command_buffer.getLength() >= 3){
					pop8(); // remove the command code
					buildPercentage = pop8();
					pop8();	// uint8_t ignore; // remove the reserved byte
					line_number++;
				}
			} else if (command == HOST_CMD_QUEUE_SONG ) //queue a song for playing
 			{
				/// Error tone is 0,
				/// End tone is 1,
				/// all other tones user-defined (defaults to end-tone)
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					uint8_t songId = pop8();
					line_number++;
					if(songId == 0)
						Piezo::errorTone(4);
					else if (songId == 1 )
						Piezo::playTune(TUNE_PRINT_DONE);
					else
						Piezo::errorTone(2);
				}

			} else if ( command == HOST_CMD_RESET_TO_FACTORY) {
				/// reset EEPROM settings to the factory value. Reboot bot.
				if (command_buffer.getLength() >= 2){
				pop8(); // remove the command code
				pop8();	//uint8_t options
				line_number++;
				eeprom::factoryResetEEPROM();
				Motherboard::getBoard().reset(false);
				}
			} else if ( command == HOST_CMD_BUILD_START_NOTIFICATION) {
				if (command_buffer.getLength() >= 5){
					pop8(); // remove the command code
					pop32();	//int buildSteps
					line_number++;
					host::handleBuildStartNotification(command_buffer);		
				}
			} else if ( command == HOST_CMD_BUILD_END_NOTIFICATION) {
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					uint8_t flags = pop8();
					line_number++;
					host::handleBuildStopNotification(flags);
				}
			} else if ( command == HOST_CMD_SET_ACCELERATION_TOGGLE) {
				if (command_buffer.getLength() >= 2){
					pop8(); // remove the command code
					line_number++;
					uint8_t status = pop8();
					steppers::setSegmentAccelState(status == 1);
				}
			} else {
			}
		}
	}
	
	/// we're not handling overflows in the line counter.  Possibly implement this later.
	if(line_number > MAX_LINE_COUNT){
		line_number = MAX_LINE_COUNT + 1;
	}
}
}


