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

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "Planner.hh"
#include <stdint.h>
#include "feedrate_table.hh"
#include "StepperPorts.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "stdio.h"

namespace steppers {


/// Set up the digipot pins 
DigiPots digi_pots[STEPPER_COUNT] = {
#if STEPPER_COUNT > 0
        DigiPots( X_POT_PIN,
				  eeprom_offsets::DIGI_POT_SETTINGS),
#endif
#if STEPPER_COUNT > 1
        DigiPots(Y_POT_PIN,
				  eeprom_offsets::DIGI_POT_SETTINGS),
#endif
#if STEPPER_COUNT > 2
        DigiPots(Z_POT_PIN,
				eeprom_offsets::DIGI_POT_SETTINGS),
#endif
#if STEPPER_COUNT > 3
        DigiPots(A_POT_PIN,
				eeprom_offsets::DIGI_POT_SETTINGS),
#endif
#if STEPPER_COUNT > 4
        DigiPots(B_POT_PIN,
				eeprom_offsets::DIGI_POT_SETTINGS),
#endif
};

volatile bool is_running;
volatile int32_t intervals;
volatile int32_t intervals_remaining;

bool invert_endstops[STEPPER_COUNT];             ///< True if endstops input polarity is inverted for this axis.
bool invert_axis[STEPPER_COUNT];                 ///< True if motions for this axis should be inverted

volatile int32_t position[STEPPER_COUNT];        ///< Current position of this axis, in steps
volatile int32_t counter[STEPPER_COUNT];         ///< Step counter; represents the proportion of
								                 ///< a step so far passed.  When the counter hits
								                 ///< zero, a step is taken.
volatile int32_t delta[STEPPER_COUNT];           ///< Amount to increment counter per tick
volatile bool direction[STEPPER_COUNT];          ///< True for positive, false for negative
volatile int8_t step_change[STEPPER_COUNT];    	 ///< Used internally. step_change = direction ? 1 : -1;

#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
	volatile bool prev_direction;   ///< Record the previous direction for endstop detection
	volatile int32_t endstop_play;  ///< Amount to move while endstop triggered, to see which way to move

	enum endstop_status_t {         ///< State of the endstop
		ESS_UNKNOWN,
		ESS_TRAVELING,
		ESS_AT_MAXIMUM,
		ESS_AT_MINIMUM
	};

	volatile endstop_status_t endstop_status;

	// If we started with an endstop triggered, then we don't know where 
	// we are. We can go this many steps either way until we find out.
	const static uint16_t ENDSTOP_DEFAULT_PLAY =10000;
	const static uint16_t ENDSTOP_DEBOUNCE =20;
#endif //SINGLE_SWITCH_ENDSTOPS

struct feedrate_element {
	uint32_t rate; // interval value of the feedrate axis
	uint32_t steps;     // number of steps of the master axis to change
	uint32_t target;
};
feedrate_element feedrate_elements[3];
volatile int32_t feedrate_steps_remaining;
volatile int32_t feedrate;
volatile int32_t feedrate_target; // convenient storage to save lookup time
volatile int8_t  feedrate_dirty; // indicates if the feedrate_inverted needs recalculated
volatile int32_t feedrate_inverted;
volatile int32_t feedrate_changerate;
volatile int32_t acceleration_tick_counter;
volatile uint8_t current_feedrate_index;

volatile int32_t timer_counter;


bool acceleration_on;
volatile bool is_homing;

bool holdZ = false;

planner::Block *current_block;

bool isRunning() {
	return is_running || is_homing || !planner::isBufferEmpty();
}

void InitPins(){
		
		// initialize stepper control pins
		_SET_DIRECTION(X_DIR, true);
		_SET_DIRECTION(X_STEP, true);
		_WRITE(X_ENABLE, true);
		_SET_DIRECTION(X_ENABLE, true);
				
		_SET_DIRECTION(Y_DIR, true);
		_SET_DIRECTION(Y_STEP, true);
		_WRITE(Y_ENABLE, true);
		_SET_DIRECTION(Y_ENABLE, true);
		
		_SET_DIRECTION(Z_DIR, true);
		_SET_DIRECTION(Z_STEP, true);
		_WRITE(Z_ENABLE, true);
		_SET_DIRECTION(Z_ENABLE, true);

#if STEPPER_COUNT > 3	
		_SET_DIRECTION(A_DIR, true);
		_SET_DIRECTION(A_STEP, true);
		_WRITE(A_ENABLE, true);
		_SET_DIRECTION(A_ENABLE, true);
#endif
#if STEPPER_COUNT > 4
		_SET_DIRECTION(B_DIR, true);
		_SET_DIRECTION(B_STEP, true);
		_WRITE(B_ENABLE, true);
		_SET_DIRECTION(B_ENABLE, true);
#endif	
		
		for (uint8_t i = 0; i < STEPPER_COUNT; i++){
			// get inversion characteristics
			uint8_t axes_invert = eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION, 0);
			uint8_t endstops_invert = eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION + 2, 0);
			
			bool endstops_present = (endstops_invert & (1<<7)) != 0;	
			
			// If endstops are not present, then we consider them inverted, since they will
			// always register as high (pulled up).
			invert_endstops[i] = !endstops_present || ((endstops_invert & (1<<i)) != 0);
			invert_axis[i] = (axes_invert & (1<<i)) != 0;
					
		}
		
		// intialize endstop pins
		if ( X_MAX != NULL){
			_SET_DIRECTION(X_MAX, false);
			_WRITE(X_MAX, invert_endstops[X_AXIS]);
		}if ( X_MIN != NULL) {
			_SET_DIRECTION(X_MIN, false);
			_WRITE(X_MIN, invert_endstops[X_AXIS]);
		}
		
		if ( Y_MAX != NULL){
			_SET_DIRECTION(Y_MAX, false);
			_WRITE(Y_MAX, invert_endstops[Y_AXIS]);
		}if ( Y_MIN != NULL) {
			_SET_DIRECTION(Y_MIN, false);
			_WRITE(Y_MIN, invert_endstops[Y_AXIS]);
		}
		
		if ( Z_MAX != NULL){
			_SET_DIRECTION(Z_MAX, false);
			_WRITE(Z_MAX, invert_endstops[Z_AXIS]);
		}if ( Z_MIN != NULL) {
			_SET_DIRECTION(Z_MIN, false);
			_WRITE(Z_MIN, invert_endstops[Z_AXIS]);
		}
		
		// there are no endstops for the extruder axes
		
		// set digi pots to stored default values
		for(int i = 0; i < STEPPER_COUNT; i++){
			digi_pots[i].init(i);
		}
}

void ResetCounters() {

	for(uint8_t i = 0; i < STEPPER_COUNT; i++){
		position[i] = 0;
		counter[i] = 0;
		delta[i] = 0;
		step_change[i] = 1;
#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
		endstop_play[i] = ENDSTOP_DEFAULT_PLAY;
		endstop_status[i] = ESS_UNKNOWN;
#endif //SINGLE_SWITCH_ENDSTOPS
	}
}

void reset(){

	InitPins();
}

//public:
void init() {
	is_running = false;
	is_homing = false;
	
	InitPins();
	
	ResetCounters();

	timer_counter = 0;

	current_block = NULL;
	
	for (int i = 0; i < 3; i++) {
		feedrate_elements[i] = feedrate_element();
		feedrate_elements[i].rate = 0;
		feedrate_elements[i].target = 0;
		feedrate_elements[i].steps = 0;
	}
	
	feedrate_steps_remaining = 0;
	feedrate = 0;
	feedrate_inverted = 0;
	feedrate_dirty = 1;
	acceleration_tick_counter = 0;
	current_feedrate_index = 0;
	acceleration_on = true;
}

void abort() {
	is_running = false;
	is_homing = false;
	timer_counter = 0;
	current_block = NULL;
	feedrate_steps_remaining = 0;
	feedrate = 0;
	feedrate_inverted = 0;
	feedrate_dirty = 1;
	acceleration_tick_counter = 0;
	current_feedrate_index = 0;
	OCR3A = INTERVAL_IN_MICROSECONDS * 16;
}

/// Define current position as given point
void definePosition(const Point& position_in) {
	for (int i = 0; i < STEPPER_COUNT; i++) {
		position[i] = position_in[i];
	}
}

/// Get current position
const Point getPosition() {
#if STEPPER_COUNT > 3
	return Point(position[0],position[1],position[2],position[3],position[4]);
#else
	return Point(position[0],position[1],position[2]);
#endif
}

void setHoldZ(bool holdZ_in) {
	holdZ = holdZ_in;
}

inline void prepareFeedrateIntervals() {
	if (current_feedrate_index > 2){
		return;
	}

	feedrate_steps_remaining  = feedrate_elements[current_feedrate_index].steps;
	feedrate_changerate       = feedrate_elements[current_feedrate_index].rate;
	feedrate_target           = feedrate_elements[current_feedrate_index].target;
}

inline void recalcFeedrate() {
	
	if((feedrate > 32768)){
		feedrate_inverted = 30;
	}
	else if(feedrate >= 8192)
		feedrate_inverted = (int32_t)pgm_read_byte(&rate_table_fast[(feedrate-8192) >> 4]);
	else {
		if(feedrate < 32) {feedrate = 32;}
		feedrate_inverted = (int32_t)pgm_read_word(&rate_table_slow[feedrate]);
	}
	
	feedrate_dirty = 0;
}

void setTarget(Point target_in) {

	delta[X_AXIS] = target_in[X_AXIS] - position[X_AXIS];
	delta[Y_AXIS] = target_in[Y_AXIS] - position[Y_AXIS];
	delta[Z_AXIS] = target_in[Z_AXIS] - position[Z_AXIS];
	delta[A_AXIS] = target_in[A_AXIS] - position[A_AXIS];
	delta[B_AXIS] = target_in[B_AXIS] - position[B_AXIS];

	// The A3982 stepper driver chip has an inverted enable.
	if(delta[X_AXIS] != 0){
		_WRITE(X_ENABLE, false);
		if(delta[X_AXIS] < 0){
			delta[X_AXIS] = -delta[X_AXIS];
			direction[X_AXIS] = false;
			step_change[X_AXIS] = -1;
		}else{
			direction[X_AXIS] = true;
			step_change[X_AXIS] = 1;
			}
		_WRITE(X_DIR, invert_axis[X_AXIS] ? !direction[X_AXIS] : direction[X_AXIS]);	
	}
	
	if(delta[Y_AXIS] != 0){
		_WRITE(Y_ENABLE, false);
		if(delta[Y_AXIS] < 0){
			delta[Y_AXIS] = -delta[Y_AXIS];
			direction[Y_AXIS] = false;
			step_change[Y_AXIS] = -1;
		}else{
			direction[Y_AXIS] = true;
			step_change[Y_AXIS] = 1;
			}
		_WRITE(Y_DIR, invert_axis[Y_AXIS] ? !direction[Y_AXIS] : direction[Y_AXIS]);	
	}
	
	if(delta[Z_AXIS] != 0){
		_WRITE(Z_ENABLE, false);
		if(delta[Z_AXIS] < 0){
			delta[Z_AXIS] = -delta[Z_AXIS];
			direction[Z_AXIS] = false;
			step_change[Z_AXIS] = -1;
		}else{
			direction[Z_AXIS] = true;
			step_change[Z_AXIS] = 1;
			}
		_WRITE(Z_DIR, invert_axis[Z_AXIS] ? !direction[Z_AXIS] : direction[Z_AXIS]);	
	}
	

#if STEPPER_COUNT > 3
	if(delta[A_AXIS] != 0){
		_WRITE(A_ENABLE, false);
		if(delta[A_AXIS] < 0){
			delta[A_AXIS] = -delta[A_AXIS];
			direction[A_AXIS] = false;
			step_change[A_AXIS] = -1;
		}else{
			direction[A_AXIS] = true;
			step_change[A_AXIS] = 1;
			}
		_WRITE(A_DIR, invert_axis[A_AXIS] ? !direction[A_AXIS] : direction[A_AXIS]);	
	}
#endif
#if STEPPER_COUNT > 4
	if(delta[B_AXIS] != 0){
		_WRITE(B_ENABLE, false);
		if(delta[B_AXIS] < 0){
			delta[B_AXIS] = -delta[B_AXIS];
			direction[B_AXIS] = false;
			step_change[B_AXIS] = -1;
		}else{
			direction[B_AXIS] = true;
			step_change[B_AXIS] = 1;
			}
		_WRITE(B_DIR, invert_axis[B_AXIS] ? !direction[B_AXIS] : direction[B_AXIS]);	
	}
#endif	

}


/// load up the next movement
/// WARNING: called from inside the ISR, so get out fast
bool getNextMove() {
	is_running = false; // this ensures that the interrupt does not .. interrupt us

	if (current_block != NULL) {
		current_block->flags &= ~planner::Block::Busy;
		planner::doneWithNextBlock();
		current_block = NULL;
	}

	if (!planner::isReady()) {
		is_running = !planner::isBufferEmpty();
		return false;
	}

	current_block = planner::getNextBlock();

	// Mark block as busy (being executed by the stepper interrupt)
	// Also mark it a locked
	current_block->flags |= planner::Block::Busy | planner::Block::Locked;

	Point &target = current_block->target;

	int32_t max_delta = current_block->step_event_count;
	
	setTarget(target);
	
	current_feedrate_index = 0;
	int feedrate_being_setup = 0;
	// setup acceleration
	feedrate = 0;
	if (current_block->accelerate_until > 0) {

		feedrate = current_block->initial_rate;

		feedrate_elements[feedrate_being_setup].steps     = current_block->accelerate_until;
		feedrate_elements[feedrate_being_setup].rate      = current_block->acceleration_rate;
		feedrate_elements[feedrate_being_setup].target    = current_block->nominal_rate;
		feedrate_being_setup++;

	}

	// setup plateau
	if (current_block->decelerate_after > current_block->accelerate_until) {
		if (feedrate_being_setup == 0)
			feedrate = current_block->nominal_rate;
			
		feedrate_elements[feedrate_being_setup].steps     = current_block->decelerate_after - current_block->accelerate_until;
		feedrate_elements[feedrate_being_setup].rate      = 0;
		feedrate_elements[feedrate_being_setup].target    = current_block->nominal_rate;
		feedrate_being_setup++;
	}

	// setup deceleration
	if (current_block->decelerate_after < current_block->step_event_count) {
		if (feedrate_being_setup == 0)
			feedrate = current_block->initial_rate;
		// To prevent "falling off the end" we will say we have a "bazillion" steps left...
		feedrate_elements[feedrate_being_setup].steps     = INT16_MAX; //current_block->step_event_count - current_block->decelerate_after;
		feedrate_elements[feedrate_being_setup].rate      = -current_block->acceleration_rate;
		feedrate_elements[feedrate_being_setup].target    = current_block->final_rate;
	
	} else {
		// and in case there wasn't a deceleration phase, we'll do the same for whichever phase was last...
		feedrate_elements[feedrate_being_setup-1].steps     = INT16_MAX;
		// We don't setup anything else because we limit to the target speed anyway.
	}
	
	// unlock the block
	current_block->flags &= ~planner::Block::Locked;

	if (feedrate == 0) {
		is_running = false;
		return false;
	}

	prepareFeedrateIntervals();
	recalcFeedrate();
	acceleration_tick_counter = TICKS_PER_ACCELERATION;

	timer_counter = 0;

	intervals = max_delta;
	intervals_remaining = intervals;
	const int32_t negative_half_interval = -(intervals>>1);
	counter[X_AXIS] = negative_half_interval;
	counter[Y_AXIS] = negative_half_interval;
	counter[Z_AXIS] = negative_half_interval;
#if STEPPER_COUNT > 3
	counter[A_AXIS] = negative_half_interval;
#endif
#if STEPPER_COUNT > 4
	counter[B_AXIS] = negative_half_interval;
#endif
	is_running = true;
	
	if(delta[Z_AXIS] > ZSTEPS_PER_MM*10){
		OCR3A = HOMING_INTERVAL_IN_MICROSECONDS * 16;
	} else {
		OCR3A = INTERVAL_IN_MICROSECONDS * 16;
	}
	
	return true;
}


/// Start homing
void startHoming(const bool maximums, const uint8_t axes_enabled, const uint32_t us_per_step) {
	intervals_remaining = INT32_MAX;
	intervals = 1;
	feedrate_inverted = us_per_step;
	// ToDo: Return to using the interval if the us_per_step > INTERVAL_IN_MICROSECONDS
	const int32_t negative_half_interval = -1;
	
	OCR3A = HOMING_INTERVAL_IN_MICROSECONDS * 16;
	
	for (int i = 0; i < STEPPER_COUNT; i++) {
		counter[i] = negative_half_interval;
		delta[i] = 0;
	}
	
	// The A3982 stepper driver chip has an inverted enable.
	if ((axes_enabled & (1<<X_AXIS)) != 0) {
		direction[X_AXIS] = maximums;
		_WRITE(X_DIR, invert_axis[X_AXIS] ? !direction[X_AXIS] : direction[X_AXIS]);
		_WRITE(X_ENABLE, false);
		delta[X_AXIS] = 1;
		step_change[X_AXIS] = direction[X_AXIS] ? 1 : -1;
	}
	
	if ((axes_enabled & (1<<Y_AXIS)) != 0) {
		direction[Y_AXIS] = maximums;
		_WRITE(Y_DIR, invert_axis[Y_AXIS] ? !direction[Y_AXIS] : direction[Y_AXIS]);
		_WRITE(Y_ENABLE, false);
		delta[Y_AXIS] = 1;
		step_change[Y_AXIS] = direction[Y_AXIS] ? 1 : -1;
	}
	
	if ((axes_enabled & (1<<Z_AXIS)) != 0) {
		direction[Z_AXIS] = maximums;
		_WRITE(Z_DIR, invert_axis[Z_AXIS] ? !direction[Z_AXIS] : direction[Z_AXIS]);
		_WRITE(Z_ENABLE, false);
		delta[Z_AXIS] = 1;
		step_change[Z_AXIS] = direction[Z_AXIS] ? 1 : -1;
	}
	
	timer_counter = feedrate_inverted;
	is_homing = true;
}

/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
	
	// The A3982 stepper driver chip has an inverted enable.
	switch(index){
		case X_AXIS: 
			_WRITE(X_ENABLE, !enable);
			break;
        case Y_AXIS: 
			_WRITE(Y_ENABLE, !enable);
			break;
		case Z_AXIS: 
			_WRITE(Z_ENABLE, !enable);
			break;
		case A_AXIS: 
			_WRITE(A_ENABLE, !enable);
			break;
		case B_AXIS: 
			_WRITE(B_ENABLE, !enable);
			break;
	}
}

/// set digital potentiometer for stepper axis
void setAxisPotValue(uint8_t index, uint8_t value){
		if (index < STEPPER_COUNT) {
			digi_pots[index].setPotValue(value);
	}
}

void startRunning() {
	if (is_running)
		return;
	is_running = true;
}

//TODO: MAKE THIS WORK WITH THE NEW SPUN-OUT STEPPER CODE
#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
bool checkEndstop(bool isHoming){
	bool hit_endstop = direction ? interface->isAtMaximum() : interface->isAtMinimum();
// We must move at least ENDSTOP_DEBOUNCE from where we hit the endstop before we declare traveling
	if (hit_endstop || ((endstop_play < ENDSTOP_DEFAULT_PLAY - ENDSTOP_DEBOUNCE) && endstop_status != ESS_TRAVELING)) {
	// Did we *just* hit the endstop?
		if (endstop_status == ESS_TRAVELING || (isHoming && endstop_status == ESS_UNKNOWN)) {
			endstop_play   = ENDSTOP_DEFAULT_PLAY;
			if (isHoming?direction:prev_direction)
				endstop_status = ESS_AT_MAXIMUM;
			else
				endstop_status = ESS_AT_MINIMUM;

	// OR, are we traveling away from the endstop we just hit and still have play...
		} else if ((direction && endstop_status != ESS_AT_MAXIMUM) || (!direction && endstop_status != ESS_AT_MINIMUM)) {
			if (endstop_play > 0) {
				--endstop_play;
				hit_endstop = false; // pretend this never happened...
			} else {
	// we ran out of play, so we must be ramming into the side, switch directions
	// endstop_status = !direction ? ESS_AT_MAXIMUM : ESS_AT_MINIMUM;
	// endstop_play   = ENDSTOP_DEFAULT_PLAY;
			}
		}
	// otherwise we hit the endstop

	// but if we didn't hit an endstop, clear the status
	} else {
		endstop_status = ESS_TRAVELING;
		if (!isHoming) {
			endstop_play   = ENDSTOP_DEFAULT_PLAY;
		}
	}
	prev_direction = direction;
	return hit_endstop;
}

bool IsActive(uint8_t axis){
	if (delta[axis] == 0) return false;
	
	return !checkEndstop(false);
}
#endif

bool SetAccelerationOn(bool on){
	acceleration_on = on;
}


bool doInterrupt() {
	
	if (is_running) {
		if (current_block == NULL) {
			bool got_a_move = getNextMove();
			if (!got_a_move) {
				return is_running;
			}
		}
	
		timer_counter -= INTERVAL_IN_MICROSECONDS; //interval_microseconds;

		if (timer_counter < 0) {
			// if we are supposed to step too fast, we simulate double-size microsteps
			int8_t feedrate_multiplier = 1;
			timer_counter += feedrate_inverted;
			
			//if(acceleration_on){
				while (timer_counter < 0 && feedrate_multiplier < intervals_remaining) {
					feedrate_multiplier++;
					timer_counter += feedrate_inverted;
				}
		//	}

			bool axis_active[STEPPER_COUNT];

#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
			for (int i = 0; i < STEPPER_COUNT; i++){
				axis_active[i] = IsActive(i);
			}
#else		
			//TODO: Port this to handle max/min pins = NULL and non-inverted endstops ( see old stepper interface functions)
			//TODO: READ ENDSTOPS ALL AT ONCE
			axis_active[X_AXIS] = (delta[X_AXIS] != 0) && !(direction[X_AXIS] ? !_READ(X_MAX) : !_READ(X_MIN));
			axis_active[Y_AXIS] = (delta[Y_AXIS] != 0) && !(direction[Y_AXIS] ? !_READ(Y_MAX) : !_READ(Y_MIN));	
			axis_active[Z_AXIS] = (delta[Z_AXIS] != 0) && !(direction[Z_AXIS] ? !_READ(Z_MAX) : !_READ(Z_MIN));
#if STEPPER_COUNT > 3
			axis_active[A_AXIS] = (delta[A_AXIS] != 0);
#endif
#if STEPPER_COUNT > 4
			axis_active[B_AXIS] = (delta[B_AXIS] != 0); 
#endif
#endif
			
			for (uint8_t i = 0; i < feedrate_multiplier; i++){
				if(axis_active[X_AXIS]){
					counter[X_AXIS] += delta[X_AXIS] ;
					if (counter[X_AXIS]  >= 0) {
						_WRITE(X_STEP, true);
						counter[X_AXIS]  -= intervals ;
						position[X_AXIS]  += step_change[X_AXIS] ;
						_WRITE(X_STEP, false);
					}
				}
				if(axis_active[Y_AXIS])	{
					counter[Y_AXIS] += delta[Y_AXIS] ;
					if (counter[Y_AXIS]  >= 0) {
						_WRITE(Y_STEP, true);
						counter[Y_AXIS]  -= intervals ;
						position[Y_AXIS]  += step_change[Y_AXIS] ;
						_WRITE(Y_STEP, false);
					}
				}
				if(axis_active[Z_AXIS])	{
					counter[Z_AXIS] += delta[Z_AXIS] ;
					if (counter[Z_AXIS]  >= 0) {
						_WRITE(Z_STEP, true);
						counter[Z_AXIS]  -= intervals ;
						position[Z_AXIS]  += step_change[Z_AXIS] ;
						_WRITE(Z_STEP, false);
					}
				}
	#if STEPPER_COUNT > 3
				if(axis_active[A_AXIS]){
					counter[A_AXIS] += delta[A_AXIS] ;
					if (counter[A_AXIS]  >= 0) {
						_WRITE(A_STEP, true);
						counter[A_AXIS]  -= intervals ;
						position[A_AXIS]  += step_change[A_AXIS] ;
						_WRITE(A_STEP, false);
					}
				}
	#endif
	#if STEPPER_COUNT > 4
				if(axis_active[B_AXIS]){
					counter[B_AXIS] += delta[B_AXIS] ;
					if (counter[B_AXIS]  >= 0) {
						_WRITE(B_STEP, true);
						counter[B_AXIS]  -= intervals ;
						position[B_AXIS]  += step_change[B_AXIS] ;
						_WRITE(B_STEP, false);
					}
				}
	#endif

			}
			intervals_remaining -= feedrate_multiplier;

			if (intervals_remaining <= 0) { // should never need the < part, but just in case...
				bool got_a_move = getNextMove();
				if (!got_a_move) {
					return is_running;
				}
			}

			if ((feedrate_steps_remaining-=feedrate_multiplier) <= 0) {
				current_feedrate_index++;
				prepareFeedrateIntervals();
			}

			if (feedrate_dirty) {
				recalcFeedrate();
			}
		}

		if ((feedrate_changerate != 0)){
			 
			// Change our feedrate. 
			feedrate += feedrate_changerate;
			feedrate_dirty = 1;

			if ((feedrate_changerate > 0 && feedrate > feedrate_target)
			    || (feedrate_changerate < 0 && feedrate < feedrate_target)) {
				
				feedrate_changerate = 0;
				feedrate = feedrate_target;
			} 
		}
	
		return is_running;
	} else if (is_homing) {
		timer_counter -= HOMING_INTERVAL_IN_MICROSECONDS;//interval_microseconds;
		if (timer_counter <= 0) {
			is_homing = false;
			// if we are supposed to step too fast, we simulate double-size microsteps
			int8_t feedrate_multiplier = 1;
			while (timer_counter <= -feedrate_inverted) {
				feedrate_multiplier++;
				timer_counter += feedrate_inverted;
			}

			//TODO: Port endstop check to handle max/min pins = NULL and non-inverted endstops ( see old stepper interface functions)
			for (int8_t i = 0; i < feedrate_multiplier; i++){
				if (delta[X_AXIS] != 0){
					counter[X_AXIS] += delta[X_AXIS];
					if (counter[X_AXIS] >= 0) {
						counter[X_AXIS] -= intervals;
						bool hit_endstop = direction[X_AXIS] ? !_READ(X_MAX) : !_READ(X_MIN);
						if (!hit_endstop) {
							_WRITE(X_STEP, true);
							is_homing |= true;
							position[X_AXIS] += step_change[X_AXIS];
							_WRITE(X_STEP, false);
						} else {
							is_homing |= false;
						}
					}
				}
				
				if (delta[Y_AXIS] != 0){
					counter[Y_AXIS] += delta[Y_AXIS];
					if (counter[Y_AXIS] >= 0) {
						counter[Y_AXIS] -= intervals;
						bool hit_endstop = direction[Y_AXIS] ? !_READ(Y_MAX) : !_READ(Y_MIN);
						if (!hit_endstop) {
							_WRITE(Y_STEP, true);
							is_homing |= true;
							position[Y_AXIS] += step_change[Y_AXIS];
							_WRITE(Y_STEP, false);
						} else {
							is_homing |= false;
						}
					}
				}
				
				if (delta[Z_AXIS] != 0){
					counter[Z_AXIS] += delta[Z_AXIS];
					if (counter[Z_AXIS] >= 0) {
						counter[Z_AXIS] -= intervals;
						bool hit_endstop = direction[Z_AXIS] ? !_READ(Z_MAX) : !_READ(Z_MIN);
						if (!hit_endstop) {
							_WRITE(Z_STEP, true);
							is_homing |= true;
							position[Z_AXIS] += step_change[Z_AXIS];
							_WRITE(Z_STEP, false);
						} else {
							is_homing |= false;
						}
					}
				}
			}
			
			// if we're done, force a sync with the planner
			if (!is_homing)
				planner::abort();

			timer_counter += feedrate_inverted;
		}
		// if we're done, force a sync with the planner
		if (!is_homing)
			planner::abort();
		return is_homing;
	}
	return false;
}

}
