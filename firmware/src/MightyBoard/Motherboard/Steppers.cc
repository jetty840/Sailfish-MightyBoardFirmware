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
#include "StepperAxis.hh"
#include <stdint.h>
#include "feedrate_table.hh"

namespace steppers {


volatile bool is_running;
volatile int32_t intervals;
volatile int32_t intervals_remaining;

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

StepperAxis axes[STEPPER_COUNT];
volatile bool is_homing;

bool holdZ = false;

planner::Block *current_block;

bool isRunning() {
	return is_running || is_homing || !planner::isBufferEmpty();
}


//public:
void init(Motherboard& motherboard) {
	is_running = false;
	is_homing = false;
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i] = StepperAxis(motherboard.getStepperInterface(i));
	}

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
}

/// Define current position as given point
void definePosition(const Point& position) {
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i].definePosition(position[i]);
	}
}

/// Get current position
const Point getPosition() {
#if STEPPER_COUNT > 3
	return Point(axes[0].position,axes[1].position,axes[2].position,axes[3].position,axes[4].position);
#else
	return Point(axes[0].position,axes[1].position,axes[2].position);
#endif
}

void setHoldZ(bool holdZ_in) {
	holdZ = holdZ_in;
}

inline void prepareFeedrateIntervals() {
	if (current_feedrate_index > 2)
		return;
	feedrate_steps_remaining  = feedrate_elements[current_feedrate_index].steps;
	feedrate_changerate       = feedrate_elements[current_feedrate_index].rate;
	feedrate_target           = feedrate_elements[current_feedrate_index].target;
}

inline void recalcFeedrate() {
	
	if(feedrate  >= 8192)
		feedrate_inverted = (int32_t)pgm_read_byte(&rate_table_fast[(feedrate-8192) >> 4]);
	else 
		feedrate_inverted = (int32_t)pgm_read_word(&rate_table_slow[feedrate]);

	feedrate_dirty = 0;
}

// load up the next movment
// WARNING: called from inside the ISR, so get out fast
bool getNextMove() {
	is_running = false; // this ensures that the interrupt does not .. interrupt us

	DEBUG_PIN2.setValue(true);

	if (current_block != NULL) {
		current_block->flags &= ~planner::Block::Busy;
		planner::doneWithNextBlock();
		current_block = NULL;
	}

	if (!planner::isReady()) {
		is_running = !planner::isBufferEmpty();
		DEBUG_PIN2.setValue(false);
		return false;
	}

	current_block = planner::getNextBlock();

	// Mark block as busy (being executed by the stepper interrupt)
	// Also mark it a locked
	current_block->flags |= planner::Block::Busy | planner::Block::Locked;

	Point &target = current_block->target;

	int32_t max_delta = current_block->step_event_count;

	axes[X_AXIS].setTarget(target[X_AXIS], false);
	axes[Y_AXIS].setTarget(target[Y_AXIS], false);
	axes[Z_AXIS].setTarget(target[Z_AXIS], false);
	// Disable z axis on inactivity, unless holdZ is true
	if (!holdZ && axes[Z_AXIS].delta==0)
		axes[Z_AXIS].enableStepper(false);

#if STEPPER_COUNT > 3
	axes[A_AXIS].setTarget(target[A_AXIS], false);
#endif
#if STEPPER_COUNT > 4
	axes[B_AXIS].setTarget(target[B_AXIS], false);

#endif

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
			feedrate = current_block->nominal_rate;

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
		DEBUG_PIN2.setValue(false);
		return false;
	}

	prepareFeedrateIntervals();
	recalcFeedrate();
	acceleration_tick_counter = TICKS_PER_ACCELERATION;

	timer_counter = 0;

	intervals = max_delta;
	intervals_remaining = intervals;
	const int32_t negative_half_interval = -(intervals>>1);
	axes[0].counter = negative_half_interval;
	axes[1].counter = negative_half_interval;
	axes[2].counter = negative_half_interval;
#if STEPPER_COUNT > 3
	axes[3].counter = negative_half_interval;
#endif
#if STEPPER_COUNT > 4
	axes[4].counter = negative_half_interval;
#endif
	is_running = true;
	DEBUG_PIN2.setValue(false);
	return true;
}

// This needs to be called with interrupts off
bool currentBlockChanged(const planner::Block *block_check) {
	// If we are here, then we are moving AND the interrupts are frozen, so get out *fast*
	
	// If the block passed in is not this block, then the planner was too slow, bail
	if (block_check != current_block) {
		return false;
	}
	
	// We have a few scenarios to handle:
	// A- We are still accelerating, and are below current_block->accelerate_until steps
	//  Then plan as usual, and drop us in the right spot
	// B- We are still in plateau, and are below current_block->decelerate_after steps
	//  Then plan as usual, and drop us in the right spot
	// C- We are decelerating, and are still above current_block->final_rate
	//  Then set the destination speed of the deceleration phase, and call it good
	//  Kinda bad, because we slowed down too soon, but otherwise it's ok
		
	uint32_t steps_in = intervals - intervals_remaining;
	
	// clear PlannedToStop so we know we got the new plan in
	current_block->flags &= ~planner::Block::PlannedToStop;

	int32_t temp_changerate = feedrate_elements[current_feedrate_index].rate;

	int feedrate_being_setup = 0;
	// A- We are still accelerating. (The phase can only get longer, so we'll assume the rest.)
	if (temp_changerate > 0) {
		// If we're accelerating, then we will only possibly extend the acceleration phase,
		// which means we have one for sure, and it has to be the first one, index 0.
		feedrate_elements[0].steps     = current_block->accelerate_until;
		feedrate_elements[0].rate      = current_block->acceleration_rate;
		feedrate_elements[0].target    = current_block->nominal_rate;
		
		feedrate_steps_remaining = current_block->accelerate_until - steps_in;
		feedrate_target = current_block->nominal_rate;
		feedrate_changerate = current_block->acceleration_rate;
		
		// leave it ready to setup plateau and deceleration
		feedrate_being_setup = 1;

		// We do the rest after the last else below
	}
	// B- We are still in plateau. (The plateau speed won't change, and won't get shorter.)
	else if (temp_changerate == 0 && current_block->decelerate_after > current_block->accelerate_until) {
		feedrate_steps_remaining = current_block->decelerate_after - steps_in;
		feedrate_target = current_block->nominal_rate;
		feedrate_changerate = 0;
		
		// We do the rest after the last else below
	}
	// C- We are decelerating, and are still above current_block->final_rate
	else if (feedrate > current_block->final_rate) {
		feedrate_elements[0].steps     = INT16_MAX;
		feedrate_elements[0].rate      = -current_block->acceleration_rate;
		feedrate_elements[0].target    = current_block->final_rate;

		// 'Till the end of *time*, er, this move...
		feedrate_steps_remaining = INT16_MAX;
		feedrate_changerate = -current_block->acceleration_rate;
		feedrate_target = current_block->final_rate;

		return true;
	}
	// In all other cases, we got here too late. Return that we failed.
	else {
		return false;
	}
	
	current_feedrate_index = 0;
	
	// setup plateau
	if (current_block->decelerate_after > current_block->accelerate_until) {
		feedrate_elements[feedrate_being_setup].steps     = current_block->decelerate_after - current_block->accelerate_until;
		feedrate_elements[feedrate_being_setup].rate      = 0;
		feedrate_elements[feedrate_being_setup].target    = current_block->nominal_rate;
		feedrate_being_setup++;
	}
	
	// setup deceleration
	if (current_block->decelerate_after < current_block->step_event_count) {
		// To prevent "falling off the end" we will say we have a "bazillion" steps left...
		feedrate_elements[feedrate_being_setup].steps     = INT16_MAX; //current_block->step_event_count - current_block->decelerate_after;
		feedrate_elements[feedrate_being_setup].rate      = -current_block->acceleration_rate;
		feedrate_elements[feedrate_being_setup].target    = current_block->final_rate;
	} else {
		// and in case there wasn't a deceleration phase, we'll do the same for whichever phase was last...
		feedrate_elements[feedrate_being_setup-1].steps     = INT16_MAX;
		// We don't setup anything else because we limit to the target speed anyway.
	}

	// We should be setup now so that the stepper interrupt will just flow into the new plan.
	// The steppers themselves haven't changed.
	
	return true;
}


/// Start homing
void startHoming(const bool maximums, const uint8_t axes_enabled, const uint32_t us_per_step) {
	intervals_remaining = INT32_MAX;
	intervals = 1;
	feedrate_inverted = us_per_step;
	// ToDo: Return to using the interval if the us_per_step > INTERVAL_IN_MICROSECONDS
	const int32_t negative_half_interval = -1;
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i].counter = negative_half_interval;
		if ((axes_enabled & (1<<i)) != 0) {
			axes[i].setHoming(maximums);
		} else {
			axes[i].delta = 0;
		}
	}
	timer_counter = feedrate_inverted;
	is_homing = true;
}

/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
        if (index < STEPPER_COUNT) {
                axes[index].enableStepper(enable);
	}
}

/// set digital potentiometer for stepper axis
void setAxisPotValue(uint8_t index, uint8_t value){
		if (index < STEPPER_COUNT) {
			axes[index].setStepperPotValue(value);
	}
}

void startRunning() {
	if (is_running)
		return;
	is_running = true;
	// getNextMove();
}


bool doInterrupt() {
	DEBUG_PIN1.setValue(true);
	if (is_running) {
		if (current_block == NULL) {
			bool got_a_move = getNextMove();
			if (!got_a_move) {
				DEBUG_PIN1.setValue(false);
				return is_running;
			}
		}
		
		timer_counter -= INTERVAL_IN_MICROSECONDS;

		if (timer_counter < 0) {
			// if we are supposed to step too fast, we simulate double-size microsteps
			int8_t feedrate_multiplier = 1;
			timer_counter += feedrate_inverted;
			while (timer_counter < 0 && feedrate_multiplier < intervals_remaining) {
				feedrate_multiplier++;
				timer_counter += feedrate_inverted;
			}
			
			bool not_at_end = false;
			not_at_end |= axes[0].doInterrupt(intervals, feedrate_multiplier);
			not_at_end |= axes[1].doInterrupt(intervals, feedrate_multiplier);
			not_at_end |= axes[2].doInterrupt(intervals, feedrate_multiplier);
#if STEPPER_COUNT > 3
			not_at_end |= axes[3].doInterrupt(intervals, feedrate_multiplier);
#endif
#if STEPPER_COUNT > 4
			not_at_end |= axes[4].doInterrupt(intervals, feedrate_multiplier);
#endif
			// do something with not_at_end ...?


			intervals_remaining -= feedrate_multiplier;

			if (intervals_remaining <= 0) { // should never need the < part, but just in case...
				bool got_a_move = getNextMove();
				if (!got_a_move) {
					DEBUG_PIN1.setValue(false);
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

		if (feedrate_changerate != 0 && acceleration_tick_counter-- <= 0) {
			acceleration_tick_counter = TICKS_PER_ACCELERATION;
			// Change our feedrate. Here it's important to note that we can over/undershoot

			feedrate += feedrate_changerate;
			feedrate_dirty = 1;

			if ((feedrate_changerate > 0 && feedrate > feedrate_target)
			    || (feedrate_changerate < 0 && feedrate < feedrate_target)) {
				
				feedrate_changerate = 0;
				feedrate = feedrate_target;
			} 
		}
		DEBUG_PIN1.setValue(false);
		return is_running;
	} else if (is_homing) {
		timer_counter -= INTERVAL_IN_MICROSECONDS;
		if (timer_counter <= 0) {
			is_homing = false;
			// if we are supposed to step too fast, we simulate double-size microsteps
			int8_t feedrate_multiplier = 1;
			while (timer_counter <= -feedrate_inverted) {
				feedrate_multiplier++;
				timer_counter += feedrate_inverted;
			}

			// Warning: do N || is_homing
			// is_homing || N will not execute N if is_homing
			is_homing = axes[0].doHoming(1, feedrate_multiplier) || is_homing;
			is_homing = axes[1].doHoming(1, feedrate_multiplier) || is_homing;
			is_homing = axes[2].doHoming(1, feedrate_multiplier) || is_homing;
	#if STEPPER_COUNT > 3
			is_homing = axes[3].doHoming(1, feedrate_multiplier) || is_homing;
	#endif
	#if STEPPER_COUNT > 4
			is_homing = axes[4].doHoming(1, feedrate_multiplier) || is_homing;
	#endif
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
	DEBUG_PIN1.setValue(false);
	return false;
}

}
