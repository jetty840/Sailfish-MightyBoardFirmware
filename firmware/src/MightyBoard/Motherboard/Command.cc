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
#include "EepromMap.hh"
#include "SDCard.hh"
#include "Pin.hh"
#include <util/delay.h>
#include "Piezo.hh"
#include "RGB_LED.hh"
#include "Interface.hh"
#include "UtilityScripts.hh"
#include "Planner.hh"

namespace command {

#define COMMAND_BUFFER_SIZE 512
uint8_t buffer_data[COMMAND_BUFFER_SIZE];
CircularBuffer command_buffer(COMMAND_BUFFER_SIZE, buffer_data);
uint8_t currentToolIndex = 0;

bool outstanding_tool_command = false;
bool check_temp_state = false;
bool paused = false;
bool heat_shutdown = false;

uint16_t getRemainingCapacity() {
	uint16_t sz;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sz = command_buffer.getRemainingCapacity();
	}
	return sz;
}

void pause(bool pause) {
	paused = pause;
}
void heatShutdown(){
	heat_shutdown = true;
}

bool isPaused() {
	return paused;
}

bool isEmpty() {
	return command_buffer.isEmpty();
}

void push(uint8_t byte) {
	command_buffer.push(byte);
}

uint8_t pop8() {
	return command_buffer.pop();
}

int16_t pop16() {
	union {
		// AVR is little-endian
		int16_t a;
		struct {
			uint8_t data[2];
		} b;
	} shared;
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
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
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
	shared.b.data[2] = command_buffer.pop();
	shared.b.data[3] = command_buffer.pop();
	return shared.a;
}

enum {
	READY,
	MOVING,
	DELAY,
	HOMING,
	WAIT_ON_TOOL,
	WAIT_ON_PLATFORM,
	WAIT_ON_BUTTON
} mode = READY;

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


void reset() {
	command_buffer.reset();
	mode = READY;
}

bool isWaiting(){
	return (mode == WAIT_ON_BUTTON);
}
    
bool isReady() {
    return (mode == READY);
}


// Handle movement comands -- called from a few places
static void handleMovementCommand(const uint8_t &command) {
	// if we're already moving, check to make sure the buffer isn't full
	if (/*mode == MOVING && */planner::isBufferFull()) {
		return; // we'll be back!
	}
	if (command == HOST_CMD_QUEUE_POINT_EXT) {
		// check for completion
		if (command_buffer.getLength() >= 25) {
			Motherboard::getBoard().resetUserInputTimeout();
			command_buffer.pop(); // remove the command code
			mode = MOVING;
			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t dda = pop32();
			planner::addMoveToBuffer(Point(x,y,z,a,b), dda);
		}
	}
	 else if (command == HOST_CMD_QUEUE_POINT_NEW) {
		// check for completion
		if (command_buffer.getLength() >= 26) {
			Motherboard::getBoard().resetUserInputTimeout();
			command_buffer.pop(); // remove the command code
			mode = MOVING;
			int32_t x = pop32();
			int32_t y = pop32();
			int32_t z = pop32();
			int32_t a = pop32();
			int32_t b = pop32();
			int32_t us = pop32();
			uint8_t relative = pop8();
			planner::addMoveToBufferRelative(Point(x,y,z,a,b), us, relative);
		}
	}
	
}

bool processExtruderCommandPacket() {
	Motherboard& board = Motherboard::getBoard();
        uint8_t	id = command_buffer.pop();
		uint8_t command = command_buffer.pop();
		uint8_t length = command_buffer.pop();
		uint16_t temp;
		
		int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
        int32_t a = 0;
        int32_t b = 0;
        int32_t us = 1000000;
        uint8_t relative = 0x02;
        bool enable = false;

		switch (command) {
		case SLAVE_CMD_SET_TEMP:
			board.getExtruderBoard(id).getExtruderHeater().set_target_temperature(pop16());
			if(board.getPlatformHeater().isHeating()){
				check_temp_state = true;
				board.getExtruderBoard(id).getExtruderHeater().Pause(true);}
			return true;
		// can be removed in process via host query works OK
 		case SLAVE_CMD_PAUSE_UNPAUSE:
			pause(!command::isPaused());
			return true;
		case SLAVE_CMD_TOGGLE_FAN:
			board.getExtruderBoard(id).setFan((pop8() & 0x01) != 0);
			return true;
		case SLAVE_CMD_TOGGLE_VALVE:
			board.setValve((pop8() & 0x01) != 0);
			return true;
		case SLAVE_CMD_SET_PLATFORM_TEMP:
			check_temp_state = true;
			board.setUsingPlatform(true);
			board.getPlatformHeater().set_target_temperature(pop16());
			// pause extruder heaters if active
			board.getExtruderBoard(0).getExtruderHeater().Pause(true);
			board.getExtruderBoard(1).getExtruderHeater().Pause(true);
			return true;
        // not being used with 5D
		case SLAVE_CMD_TOGGLE_MOTOR_1:
			enable = command_buffer.pop() & 0x01 ? true:false;
			mode = MOVING;
			steppers::enableAxis(4, enable);
			b = 360;
			return true;
        // not being used with 5D
		case SLAVE_CMD_TOGGLE_MOTOR_2: 
			enable = command_buffer.pop() & 0x01 ? true:false;
			steppers::enableAxis(3, enable);
			a = 160;
			return true;
		case SLAVE_CMD_SET_MOTOR_1_PWM:
			command_buffer.pop();
			return true;
		case SLAVE_CMD_SET_MOTOR_2_PWM:
			command_buffer.pop();
			return true;
		case SLAVE_CMD_SET_MOTOR_1_DIR:
			command_buffer.pop();
			return true;
		case SLAVE_CMD_SET_MOTOR_2_DIR:
			command_buffer.pop();
			return true;
		case SLAVE_CMD_SET_MOTOR_1_RPM:
			pop32();
			return true;
		case SLAVE_CMD_SET_MOTOR_2_RPM:
			pop32();
			return true;
		}
	return false;
}

// A fast slice for processing commands and refilling the stepper queue, etc.
void runCommandSlice() {
    // get command from SD card if building from SD
	if (sdcard::isPlaying()) {
		while (command_buffer.getRemainingCapacity() > 0 && sdcard::playbackHasNext()) {
			command_buffer.push(sdcard::playbackNext());
		}
		if(!sdcard::playbackHasNext() && command_buffer.isEmpty())
			sdcard::finishPlayback();
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
	// don't execute commands if paused or shutdown because of heater failure
	if (paused || heat_shutdown) {	return; }
    
	if (mode == HOMING) {
		if (!steppers::isRunning()) {
			mode = READY;
		} else if (homing_timeout.hasElapsed()) {
			planner::abort();
			mode = READY;
		}
	}
	if (mode == MOVING) {
		if (!steppers::isRunning()) {
			mode = READY;
		} else {
			if (command_buffer.getLength() > 0) {
				Motherboard::getBoard().resetUserInputTimeout();
				uint8_t command = command_buffer[0];
				if (command == HOST_CMD_QUEUE_POINT_EXT || command == HOST_CMD_QUEUE_POINT_NEW) {
					handleMovementCommand(command);
				}
				else if (command == HOST_CMD_ENABLE_AXES) {
					if (command_buffer.getLength() >= 2) {
						command_buffer.pop(); // remove the command code
						uint8_t axes = command_buffer.pop();
						}
				}
			}
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
			Motherboard::getBoard().errorResponse("I timed out while   attempting to heat  my extruder."); 
			mode = READY;		
		}
		else if(!Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().isHeating()){
			mode = READY;
		}else if( Motherboard::getBoard().getExtruderBoard(currentToolIndex).getExtruderHeater().has_reached_target_temperature()){
			Motherboard::getBoard().errorResponse("target temp reached."); 
            mode = READY;
		}
	}
	if (mode == WAIT_ON_PLATFORM) {
		if(tool_wait_timeout.hasElapsed()){
			Motherboard::getBoard().errorResponse("I timed out while   attempting to heat  my platform."); 
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
		
		// process next command on the queue.
		if ((command_buffer.getLength() > 0)){
			Motherboard::getBoard().resetUserInputTimeout();
			
			uint8_t command = command_buffer[0];
		if (command == HOST_CMD_QUEUE_POINT_ABS) {
				// check for completion
				if (command_buffer.getLength() >= 17) {
					// no longer supported
					command_buffer.pop(); // remove the command code
					mode = MOVING;
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					int32_t dda = pop32();
					//steppers::setTarget(Point(x,y,z),dda);
				}
			}  else if (command == HOST_CMD_QUEUE_POINT_EXT || command == HOST_CMD_QUEUE_POINT_NEW) {
					handleMovementCommand(command);
			}  else if (command == HOST_CMD_CHANGE_TOOL) {
				if (command_buffer.getLength() >= 2) {
					command_buffer.pop(); // remove the command code
                    currentToolIndex = command_buffer.pop();
                    planner::changeToolIndex(currentToolIndex);
				}
			} else if (command == HOST_CMD_ENABLE_AXES) {
				if (command_buffer.getLength() >= 2) {
					command_buffer.pop(); // remove the command code
					uint8_t axes = command_buffer.pop();
					bool enable = (axes & 0x80) != 0;
					for (int i = 0; i < STEPPER_COUNT; i++) {
						if ((axes & _BV(i)) != 0) {
							steppers::enableAxis(i, enable);
						}
					}
				}
			} else if (command == HOST_CMD_SET_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 13) {
					command_buffer.pop(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					planner::definePosition(Point(x,y,z));
				}
			} else if (command == HOST_CMD_SET_POSITION_EXT) {
				// check for completion
				if (command_buffer.getLength() >= 21) {
					command_buffer.pop(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					int32_t a = pop32();
					int32_t b = pop32();
					planner::definePosition(Point(x,y,z,a,b));
				}
			} else if (command == HOST_CMD_DELAY) {
				if (command_buffer.getLength() >= 5) {
					mode = DELAY;
					command_buffer.pop(); // remove the command code
					// parameter is in milliseconds; timeouts need microseconds
					uint32_t microseconds = pop32() * 1000;
					delay_timeout.start(microseconds);
				}
			} else if (command == HOST_CMD_PAUSE_FOR_BUTTON) {
				if (command_buffer.getLength() >= 5) {
					command_buffer.pop(); // remove the command code
					button_mask = command_buffer.pop();
					uint16_t timeout_seconds = pop16();
					button_timeout_behavior = command_buffer.pop();
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
					command_buffer.pop(); // remove the command code
					uint8_t options = command_buffer.pop();
					uint8_t xpos = command_buffer.pop();
					uint8_t ypos = command_buffer.pop();
					uint8_t timeout_seconds = command_buffer.pop();
                    // check message clear bit
					if ( (options & (1 << 0)) == 0 ) { scr->clearMessage(); }
					scr->setXY(xpos,ypos);
					scr->addMessage(command_buffer, (options & (1 << 1)));
                    // push message screen
					InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
					if (ib.getCurrentScreen() != scr) {
						ib.pushScreen(scr);
					}
                    // set message timeout if not a buttonWait call
					if ((timeout_seconds != 0) && (!(options & (1 <<2)))) {
							scr->setTimeout(timeout_seconds, true);
					}
                    
					if (options & (1 << 2)) { // button wait bit --> start button wait
						if (timeout_seconds != 0) {
							button_wait_timeout.start(timeout_seconds * 1000L * 1000L);
						} else {
							button_wait_timeout = Timeout();
						}
						button_mask = 0x01;  // center button
						button_timeout_behavior &= (1 << BUTTON_CLEAR_SCREEN);
						Motherboard::getBoard().interfaceBlink(25,15);
						InterfaceBoard& ib = Motherboard::getBoard().getInterfaceBoard();
						ib.waitForButton(button_mask);
						mode = WAIT_ON_BUTTON;
					}
				}
					
			} else if (command == HOST_CMD_FIND_AXES_MINIMUM ||
					command == HOST_CMD_FIND_AXES_MAXIMUM) {
				if (command_buffer.getLength() >= 8) {
					command_buffer.pop(); // remove the command
					uint8_t flags = pop8();
					uint32_t feedrate = pop32(); // feedrate in us per step
					uint16_t timeout_s = pop16();
					bool direction = command == HOST_CMD_FIND_AXES_MAXIMUM;
					mode = HOMING;
					homing_timeout.start(timeout_s * 1000L * 1000L);
					steppers::startHoming(command==HOST_CMD_FIND_AXES_MAXIMUM,
							flags,
							feedrate);
				}
			} else if (command == HOST_CMD_WAIT_FOR_TOOL) {
				if (command_buffer.getLength() >= 6) {
					mode = WAIT_ON_TOOL;
					command_buffer.pop();
					currentToolIndex = command_buffer.pop();
					uint16_t toolPingDelay = (uint16_t)pop16();
					uint16_t toolTimeout = (uint16_t)pop16();
					// if we re-add handling of toolTimeout, we need to make sure
					// that values that overflow our counter will not be passed)
					tool_wait_timeout.start(toolTimeout*1000000L);
				}
			} else if (command == HOST_CMD_WAIT_FOR_PLATFORM) {
        // FIXME: Almost equivalent to WAIT_FOR_TOOL
				if (command_buffer.getLength() >= 6) {
					mode = WAIT_ON_PLATFORM;
					command_buffer.pop();
					uint8_t currentToolIndex = command_buffer.pop();
					uint16_t toolPingDelay = (uint16_t)pop16();
					uint16_t toolTimeout = (uint16_t)pop16();
					// if we re-add handling of toolTimeout, we need to make sure
					// that values that overflow our counter will not be passed)
					tool_wait_timeout.start(toolTimeout*1000000L);
				}
			} else if (command == HOST_CMD_STORE_HOME_POSITION) {

				// check for completion
				if (command_buffer.getLength() >= 2) {
					command_buffer.pop();
					uint8_t axes = pop8();

					// Go through each axis, and if that axis is specified, read it's value,
					// then record it to the eeprom.
					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
						if ( axes & (1 << i) ) {
							uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS + 4*i;
							uint32_t position = steppers::getPosition()[i];
							cli();
							eeprom_write_block(&position, (void*) offset, 4);
							sei();
						}
					}
				}
			} else if (command == HOST_CMD_RECALL_HOME_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 2) {
					command_buffer.pop();
					uint8_t axes = pop8();
				

					Point newPoint = steppers::getPosition();

					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
						if ( axes & (1 << i) ) {
							uint16_t offset = eeprom_offsets::AXIS_HOME_POSITIONS + 4*i;
							cli();
							eeprom_read_block(&(newPoint[i]), (void*) offset, 4);
							sei();
						}
					}

					planner::definePosition(newPoint);
				}

			}else if (command == HOST_CMD_SET_POT_VALUE){
				if (command_buffer.getLength() >= 3) {
					command_buffer.pop(); // remove the command code
					uint8_t axis = pop8();
					uint8_t value = pop8();
                    steppers::setAxisPotValue(axis, value);
                    steppers::setAxisPotValue(axis, value);
				}
			}else if (command == HOST_CMD_SET_RGB_LED){
				if (command_buffer.getLength() >= 6) {
					command_buffer.pop(); // remove the command code

					uint8_t red = pop8();
					uint8_t green = pop8();
					uint8_t blue = pop8();
					uint8_t blink_rate = pop8();

                    uint8_t effect = pop8();
                    
                    RGB_LED::setLEDBlink(blink_rate);
                    RGB_LED::setCustomColor(red, green, blue);

				}
			}else if (command == HOST_CMD_SET_BEEP){
				if (command_buffer.getLength() >= 6) {
					command_buffer.pop(); // remove the command code
					uint16_t frequency= pop16();
					uint16_t beep_length = pop16();
					uint8_t effect = pop8();
                    Piezo::setTone(frequency, beep_length);

				}			
			}else if (command == HOST_CMD_TOOL_COMMAND) {
				if (command_buffer.getLength() >= 4) { // needs a payload
					uint8_t payload_length = command_buffer[3];
					if (command_buffer.getLength() >= 4+payload_length) {
							command_buffer.pop(); // remove the command code
							processExtruderCommandPacket();
				}
			}
			} else if (command == HOST_CMD_SET_BUILD_PERCENT){
				if (command_buffer.getLength() >= 3){
					command_buffer.pop(); // remove the command code
					uint8_t percent = pop8();
					uint8_t ignore = pop8(); // remove the reserved byte
					interface::setBuildPercentage(percent);
				}
			} else if (command == HOST_CMD_QUEUE_SONG ) //queue a song for playing
 			{
				/// Error tone is 0,
				/// End tone is 1,
				/// all other tones user-defined (defaults to end-tone)
				if (command_buffer.getLength() >= 2){
					command_buffer.pop(); // remove the command code
					uint8_t songId = pop8();
					if(songId == 0)
						Piezo::errorTone(4);
					else if (songId == 1 )
						Piezo::doneTone();
					else
						Piezo::errorTone(2);
				}

			} else if ( command == HOST_CMD_RESET_TO_FACTORY) {
				/// reset EEPROM settings to the factory value. Reboot bot.
				if (command_buffer.getLength() >= 2){
				command_buffer.pop(); // remove the command code
				uint8_t options = pop8();
				eeprom::factoryResetEEPROM();
				Motherboard::getBoard().reset(false);
				}
			} else if ( command == HOST_CMD_BUILD_START_NOTIFICATION) {
				if (command_buffer.getLength() >= 5){
					command_buffer.pop(); // remove the command code
					int buildSteps = pop32();
					host::handleBuildStartNotification(command_buffer);
				}
			 } else if ( command == HOST_CMD_BUILD_END_NOTIFICATION) {
				if (command_buffer.getLength() >= 2){
					command_buffer.pop(); // remove the command code
					uint8_t flags = command_buffer.pop();
					host::handleBuildStopNotification(flags);
				}
			
			} else {
			}
		}
	}
}
}


