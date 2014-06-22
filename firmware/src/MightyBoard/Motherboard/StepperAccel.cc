/*
  StepperAccel.cc - stepper motor driver: executes motion plans using stepper motors
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

  The timer calculations of this module informed by the 'RepRap cartesian firmware' by Zack Smith
  and Philipp Tiefenbacher.

  This module has been heavily modified from the original Marlin (https://github.com/ErikZalm).
  JKN Advance, YAJ (Yet Another Jerk), Advance Pressure Relax and modifications originate from
  Jetty Firmware (https://github.com/jetty840/G3Firmware).  These modifications and features are
  copyrighted and authored by Dan Newman and Jetty under GPL.  Copyright (c) 2012.
*/


#include "Configuration.hh"
#include "StepperAccel.hh"

#ifdef LOOKUP_TABLE_TIMER
	#include "StepperAccelSpeedTable.hh"
#endif

#include "Motherboard.hh"

#include <avr/interrupt.h>
#include <string.h>
#include <math.h>
#include "StepperAxis.hh"
#include "Steppers.hh"


block_t		*current_block;				// A pointer to the block currently being traced
bool            extruder_deprime_travel;                // When false, only deprime on pauses
bool		extrude_when_negative[EXTRUDERS];	// True if negative values cause an extruder to extrude material
int16_t		extruder_deprime_steps[EXTRUDERS];	// Positive number of steps to prime / deprime
float		extruder_only_max_feedrate[EXTRUDERS];

#ifdef JKN_ADVANCE
	enum AdvanceState {
		ADVANCE_STATE_ACCEL = 0,
		ADVANCE_STATE_PLATEAU,
		ADVANCE_STATE_DECEL
	};
	static enum AdvanceState	advance_state;
	static int32_t			advance_pressure_relax_accumulator;
	#ifdef JKN_ADVANCE_LEAD_DE_PRIME
		static int16_t		lastAdvanceDeprime[EXTRUDERS];
	#endif

	#define ADVANCE_INTERRUPT_FREQUENCY 10000	// 10KHz

	static uint8_t			st_extruder_interrupt_rate[EXTRUDERS]		= {0, 0};
	static uint8_t			extruder_interrupt_steps_per_call[EXTRUDERS]	= {0, 0};
	static uint8_t			st_extruder_interrupt_rate_counter[EXTRUDERS];
#endif

static unsigned char		out_bits;		// The next stepping-bits to be output
volatile static uint32_t	step_events_completed;	// The number of step events executed in the current block

static int32_t		acceleration_time, deceleration_time;
static uint16_t		acc_step_rate, step_rate;
static char		step_loops, step_loops_nominal;
static uint16_t		OCRnA_nominal;

static bool		deprimed[EXTRUDERS];

static bool		deprime_enabled;		//If true, depriming is On, if not, it's Off.  It's normally switched on.
							//But is switched off when loading/unloading the extruder 
static uint8_t		last_active_toolhead = 0;

#if  defined(DEBUG_TIMER)
uint16_t debugTimer;
#endif

#ifdef OVERSAMPLED_DDA
uint8_t oversampledCount = 0;
#endif

#if defined(PSTOP_2_SUPPORT)
boolean extrusion_seen[EXTRUDERS];
#endif

// intRes = intIn1 * intIn2 >> 16
// uses:
// r26 to store 0
// r27 to store the byte 1 of the 24 bit result
#define MultiU16X8toH16(intRes, charIn1, intIn2)	\
	asm volatile (					\
		"clr r26 \n\t"				\
		"mul %A1, %B2 \n\t"			\
		"movw %A0, r0 \n\t"			\
		"mul %A1, %A2 \n\t"			\
		"add %A0, r1 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"lsr r0 \n\t"				\
		"adc %A0, r26 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"clr r1 \n\t"				\
		:					\
		"=&r" (intRes)				\
		:					\
		"d" (charIn1),				\
		"d" (intIn2)				\
		:					\
		"r26"					\
	)

// intRes = longIn1 * longIn2 >> 24
// uses:
// r26 to store 0
// r27 to store the byte 1 of the 48bit result
#define MultiU24X24toH16(intRes, longIn1, longIn2)	\
	asm volatile (					\
		"clr r26 \n\t"				\
		"mul %A1, %B2 \n\t"			\
		"mov r27, r1 \n\t"			\
		"mul %B1, %C2 \n\t"			\
		"movw %A0, r0 \n\t"			\
		"mul %C1, %C2 \n\t"			\
		"add %B0, r0 \n\t"			\
		"mul %C1, %B2 \n\t"			\
		"add %A0, r0 \n\t"			\
		"adc %B0, r1 \n\t"			\
		"mul %A1, %C2 \n\t"			\
		"add r27, r0 \n\t"			\
		"adc %A0, r1 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"mul %B1, %B2 \n\t"			\
		"add r27, r0 \n\t"			\
		"adc %A0, r1 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"mul %C1, %A2 \n\t"			\
		"add r27, r0 \n\t"			\
		"adc %A0, r1 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"mul %B1, %A2 \n\t"			\
		"add r27, r1 \n\t"			\
		"adc %A0, r26 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"lsr r27 \n\t"				\
		"adc %A0, r26 \n\t"			\
		"adc %B0, r26 \n\t"			\
		"clr r1 \n\t"				\
		:					\
		"=&r" (intRes)				\
		:					\
		"d" (longIn1),				\
		"d" (longIn2)				\
		:					\
		"r26" , "r27"				\
	)

// Some useful constants

#define ENABLE_STEPPER_DRIVER_INTERRUPT()	STEPPER_TIMSKn |= (1<<STEPPER_OCIEnA)
#define DISABLE_STEPPER_DRIVER_INTERRUPT()	STEPPER_TIMSKn &= ~(1<<STEPPER_OCIEnA)


//         __________________________
//        /|                        |\     _________________         ^
//       / |                        | \   /|               |\        |
//      /  |                        |  \ / |               | \       s
//     /   |                        |   |  |               |  \      p
//    /    |                        |   |  |               |   \     e
//   +-----+------------------------+---+--+---------------+----+    e
//   |               BLOCK 1            |      BLOCK 2          |    d
//
//                           time ----->
// 
//  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates 
//  first block->accelerate_until step_events_completed, then keeps going at constant speed until 
//  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
//  The slope of acceleration is calculated with the leib ramp alghorithm.

struct lookup_table_entry {
	union {
		uint32_t dword_entry;
		uint16_t word_entry[2];
	};
};



// Shift by 1 byte to the right
#define SHIFT1(x) (uint8_t)(x >> 8 )


FORCE_INLINE uint16_t calc_timer(uint16_t step_rate) {
	uint16_t timer;
	uint8_t step_rate_high = SHIFT1(step_rate);

	if (step_rate_high > SHIFT1(19968)) { // If steprate > 19.968 kHz >> step 8 times
		if (step_rate_high > SHIFT1(MAX_STEP_FREQUENCY)) // ~39.936 kHz
			step_rate = (MAX_STEP_FREQUENCY >> 3) & 0x1fff;
		else
			step_rate = (step_rate >> 3) & 0x1fff;
		step_loops = 8;
	}
	else if (step_rate_high > SHIFT1(9984)) { // If steprate > 9.984 kHz >> step 4 times
		step_rate = (step_rate >> 2) & 0x3fff;
		step_loops = 4;
	}
	else if (step_rate_high > SHIFT1(4864)) { // If steprate > 4.864 kHz >> step 2 times
		step_rate = (step_rate >> 1) & 0x7fff;
		step_loops = 2;
	} else {
		if (step_rate < 32) step_rate = 32;
		step_loops = 1;
	}

	#ifdef LOOKUP_TABLE_TIMER
		step_rate -= 32; // Correct for minimal speed

		if(step_rate >= (8*256)) { // higher step rate 
			uint16_t table_address		= (uint16_t)&speed_lookuptable_fast[(unsigned char)(step_rate>>8)][0];
			unsigned char tmp_step_rate	= (step_rate & 0x00ff);

			struct lookup_table_entry	table_entry;
			table_entry.dword_entry		= (uint32_t)pgm_read_dword_near(table_address);

			uint16_t gain			= table_entry.word_entry[1];

			MultiU16X8toH16(timer, tmp_step_rate, gain);

			timer = table_entry.word_entry[0] - timer;
		} else { // lower step rates
			uint16_t table_address		= (uint16_t)&speed_lookuptable_slow[0][0];

			table_address += ((step_rate)>>1) & 0xfffc;

			struct lookup_table_entry	table_entry;
			table_entry.dword_entry		= (uint32_t)pgm_read_dword_near(table_address);

			timer = table_entry.word_entry[0];
			timer -= ((table_entry.word_entry[1] * (unsigned char)(step_rate & 0x0007))>>3);
		}

		//if(timer < 100) { timer = 100; MSerial.print("Steprate to high : "); MSerial.println(step_rate); }//(20kHz this should never happen)

		return timer;
	#else
		return (uint16_t)((uint32_t)2000000 / (uint32_t)step_rate);
	#endif
}



// Sets up the next block from the buffer

FORCE_INLINE void setup_next_block() {
	//DEBUG_TIMER_START;

	// Using this instead of memcpy saves 64 cycles
	// starting_position is needed so that "definePosition" in Steppers.cc doesn't require a buffer drain before
	// setting the position.  By including the starting_position in the block, we can make definePosition
	// asynchronous
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
        dda_position[X_AXIS] = current_block->starting_position[X_AXIS] + current_block->starting_position[Y_AXIS]; 
        dda_position[Y_AXIS] = current_block->starting_position[X_AXIS] - current_block->starting_position[Y_AXIS]; 
	for ( uint8_t i = Z_AXIS; i < STEPPER_COUNT; i++ ) {
		dda_position[i] = current_block->starting_position[i];
	}
#else
	for ( uint8_t i = 0; i < STEPPER_COUNT; i++ ) {
		dda_position[i] = current_block->starting_position[i];
	}
#endif

	// Setup the next dda's and enabled axis
	out_bits = current_block->direction_bits;

#ifdef CORE_XY_STEPPER
	// Clear the bits which indicate if the underlying X and Y axis are moving in the negative direction
	// Shouldn't actually be necessary since the field was initialized to 0
	// current_block->direction_bits &= ~( (1 << (X_AXIS + B_AXIS + 1)) | (1 << (Y_AXIS + B_AXIS + 1)) );

	// Clear the direction bits for the Core-XY A and B axes
	out_bits &= ~((1 << X_AXIS) | (1 << Y_AXIS));

	{
	     int32_t asteps, bsteps, xsteps, ysteps;

	     xsteps = current_block->steps[X_AXIS];
	     ysteps = current_block->steps[Y_AXIS];

	     // Determine the signs of X and Y
	     if ( out_bits & (1 << X_AXIS) ) xsteps = -xsteps;
	     if ( out_bits & (1 << Y_AXIS) ) ysteps = -ysteps;

	     asteps = xsteps + ysteps;
	     bsteps = xsteps - ysteps;

	     // Now set the direction bits for the Core-XY A and B axes
	     if ( asteps < 0 ) out_bits |= (1 << X_AXIS);
	     if ( bsteps < 0 ) out_bits |= (1 << Y_AXIS);

	     if (asteps > 0 && bsteps > 0) { out_bits |= 1 << (X_AXIS + B_AXIS + 1); }  // +X
	     if (asteps > 0 && bsteps < 0) { out_bits |= 1 << (X_AXIS + B_AXIS + 1); }  // +Y

	     // Ensure that both axes are enabled if either axes will be used
	     if ( asteps != 0 || bsteps != 0 )
		  // Need both steppers holding
		  current_block->axesEnabled |= _BV(X_AXIS) | _BV(Y_AXIS);

	     current_block->steps[X_AXIS] = labs(asteps);
	     current_block->steps[Y_AXIS] = labs(bsteps);
	}

	{
	     // Recompute the master steps

	     uint32_t max_steps = current_block->steps[X_AXIS];
	     uint8_t max_index = X_AXIS;

	     for ( uint8_t i = Y_AXIS; i < STEPPER_COUNT; i++ )
	     {
		  if ( current_block->steps[i] <= max_steps )
		       continue;
		  max_steps = current_block->steps[i];
		  max_index = i;
	     }
	     current_block->step_event_count      = max_steps;
	     current_block->dda_master_axis_index = max_index;
	}
#endif

	last_active_toolhead = current_block->active_toolhead;

	#ifdef JKN_ADVANCE
		if ( deprime_enabled ) {
			// Something in the buffer, prime if we previously deprimed
			for (uint8_t e = 0; e < EXTRUDERS; e ++ ) {
				if (( current_block->steps[A_AXIS + e] != 0 ) && ( deprimed[e] )) {

					if ( extrude_when_negative[e] ) {
						e_steps[e] -= extruder_deprime_steps[e];
						#ifdef JKN_ADVANCE_LEAD_DE_PRIME
							e_steps[e] -= current_block->advance_lead_prime;
						#endif
					} else {
						e_steps[e] += extruder_deprime_steps[e];
						#ifdef JKN_ADVANCE_LEAD_DE_PRIME
							e_steps[e] += current_block->advance_lead_prime;
						#endif
					}
					deprimed[e] = false;

				}
			}
		}
	#endif

	deceleration_time = 0;

	OCRnA_nominal = calc_timer(current_block->nominal_rate);
	step_loops_nominal = step_loops;
  
	if ( current_block->use_accel ) {
		// step_rate to timer interval
		acc_step_rate = current_block->initial_rate;
		acceleration_time = calc_timer(acc_step_rate);
		#ifdef OVERSAMPLED_DDA
			STEPPER_OCRnA = acceleration_time >> OVERSAMPLED_DDA;
		#else
			STEPPER_OCRnA = acceleration_time;
		#endif
	} else {
		STEPPER_OCRnA = OCRnA_nominal;
	}

	//if we have e_steps, re-enable the active extruders
	uint8_t extruderOverriddenAxesEnabled = current_block->axesEnabled;
	if ( e_steps[0] || steppers::extruder_hold[0] ) extruderOverriddenAxesEnabled |= _BV(A_AXIS);
	if ( e_steps[1] || steppers::extruder_hold[1] ) extruderOverriddenAxesEnabled |= _BV(B_AXIS);

	stepperAxisSetHardwareEnabledToMatch(extruderOverriddenAxesEnabled);

	// Reset the dda's, doing it this way instead of a loop saves 325 cycles.
	stepperAxis_dda_reset(X_AXIS, (current_block->dda_master_axis_index == X_AXIS), current_block->step_event_count,
				(out_bits & (1 << X_AXIS)), current_block->steps[X_AXIS]);
	stepperAxis_dda_reset(Y_AXIS, (current_block->dda_master_axis_index == Y_AXIS), current_block->step_event_count, 
				(out_bits & (1 << Y_AXIS)), current_block->steps[Y_AXIS]);
	stepperAxis_dda_reset(Z_AXIS, (current_block->dda_master_axis_index == Z_AXIS), current_block->step_event_count, 
				(out_bits & (1 << Z_AXIS)), current_block->steps[Z_AXIS]);
	stepperAxis_dda_reset(A_AXIS, (current_block->dda_master_axis_index == A_AXIS), current_block->step_event_count, 
				(out_bits & (1 << A_AXIS)), current_block->steps[A_AXIS]);
	stepperAxis_dda_reset(B_AXIS, (current_block->dda_master_axis_index == B_AXIS), current_block->step_event_count, 
				(out_bits & (1 << B_AXIS)), current_block->steps[B_AXIS]);

#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
	stepperAxis_dda_reset_corexy(X_AXIS, out_bits & (1 << (X_AXIS + B_AXIS + 1)));
	stepperAxis_dda_reset_corexy(Y_AXIS, out_bits & (1 << (Y_AXIS + B_AXIS + 1)));
#endif

	#ifdef JKN_ADVANCE
		advance_state = ADVANCE_STATE_ACCEL;
	#endif
	step_events_completed = 0;

	#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		if ( current_block->move_index == 4 ) {
			debug_onscreen1 = (float)current_block->initial_rate;
			debug_onscreen2 = (float)current_block->final_rate;
		}
	#endif

	//DEBUG_TIMER_FINISH;
	//debug_onscreen1 = DEBUG_TIMER_TCTIMER_CYCLES;
}



// "The Stepper Driver Interrupt" - This timer interrupt is the workhorse.  
// It pops blocks from the block_buffer and executes them by pulsing the stepper pins appropriately.
// Returns true if we deleted an item in the pipeline buffer 

bool st_interrupt() {    
	//DEBUG_TIMER_START;
	bool block_deleted = false;

	#ifdef OVERSAMPLED_DDA
		if ( current_block != NULL ) {
			oversampledCount ++;

			if ( oversampledCount < (1 << OVERSAMPLED_DDA) ) {
				//Step the dda for each axis
				stepperAxis_dda_step(X_AXIS);
				stepperAxis_dda_step(Y_AXIS);
				stepperAxis_dda_step(Z_AXIS);
				stepperAxis_dda_step(A_AXIS);
				stepperAxis_dda_step(B_AXIS);

				return block_deleted;
			}
		}
	#endif

	// If there is no current block, attempt to pop one from the buffer
	if (current_block == NULL) {
		// Anything in the buffer?
		current_block = plan_get_current_block();

		if (current_block != NULL) {
			setup_next_block();
		} else {
			STEPPER_OCRnA=2000; // 1kHz.

			// Buffer is empty, because enabling/disabling axes doesn't require a block to be 
			// present, we better set the hardware to match the last enable/disable in software
			// If we're running JKN_ADVANCE, the e_steps are on a seperate interrupt so we need to wait for those to be
			// empty too
			#ifdef JKN_ADVANCE
				if (( e_steps[0] == 0 ) && ( e_steps[1] == 0 ))
			#endif
					stepperAxisSetHardwareEnabledToMatch(axesEnabled);
		}
	} 

	#ifdef JKN_ADVANCE
		// Nothing in the buffer or we have no e steps, deprime
		if ( deprime_enabled ) {
			for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {
			     if ( ( ! deprimed[e] ) && (( current_block == NULL ) || ( extruder_deprime_travel && (! (current_block->axesEnabled & _BV(A_AXIS + e))))) ) {

					if ( extrude_when_negative[e] ) {
						e_steps[e] += extruder_deprime_steps[e];
						#ifdef JKN_ADVANCE_LEAD_DE_PRIME
							e_steps[e] += lastAdvanceDeprime[e];
						#endif
					} else {
						e_steps[e] -= extruder_deprime_steps[e];
						#ifdef JKN_ADVANCE_LEAD_DE_PRIME
							e_steps[e] -= lastAdvanceDeprime[e];
						#endif
					}

					deprimed[e] = true;
				}    
			}
		}
	#endif

	if (current_block != NULL) {
		// Take multiple steps per interrupt (For high speed moves) 
		for(int8_t i=0; i < step_loops; i++) {
			#ifdef JKN_ADVANCE
				if ( current_block->use_accel ) {
					for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {
						if ( advance_state == ADVANCE_STATE_ACCEL ) {
							stepperAxis_dda_shift_phase16(A_AXIS + e, current_block->advance_lead_entry);
						}
						if ( advance_state == ADVANCE_STATE_DECEL ) {
							stepperAxis_dda_shift_phase16(A_AXIS + e, - current_block->advance_lead_exit);
							stepperAxis_dda_shift_phase32(A_AXIS + e, - advance_pressure_relax_accumulator >> 8);
						}
					}
				}
			#endif
      
			//Step the dda for each axis
			stepperAxis_dda_step(X_AXIS);
			stepperAxis_dda_step(Y_AXIS);
			stepperAxis_dda_step(Z_AXIS);
			stepperAxis_dda_step(A_AXIS);
			stepperAxis_dda_step(B_AXIS);

			#ifdef OVERSAMPLED_DDA
				oversampledCount = 0;
			#endif

			step_events_completed += 1;  

			if(step_events_completed >= current_block->step_event_count) break;
		}

		// Calculate new timer value
		uint16_t timer;
		if (step_events_completed <= (uint32_t)current_block->accelerate_until) { // ACCELERATION PHASE
      
			// Note that we need to convert acceleration_time from units of
			// 2 MHz to seconds.  That is done by dividing acceleration_time
			// by 2000000.  But, that will make it 0 when we use integer
			// arithmetic.  So, we first multiply block->acceleration_rate by
			// acceleration_time and then do the divide.  However, it's
			// convenient to divide by 2^24 ( >> 24 ).  So, block->acceleration_rate
			// has been prescaled by a factor of 8.388608.

			MultiU24X24toH16(acc_step_rate, acceleration_time, current_block->acceleration_rate);
			acc_step_rate += current_block->initial_rate;
      
			// upper limit
			if (acc_step_rate > current_block->nominal_rate)	acc_step_rate = current_block->nominal_rate;

			// step_rate to timer interval
			timer = calc_timer(acc_step_rate);
			#ifdef OVERSAMPLED_DDA
				STEPPER_OCRnA = timer >> OVERSAMPLED_DDA;
			#else
				STEPPER_OCRnA = timer;
			#endif

			acceleration_time += timer;
		} 
		else if (step_events_completed > (uint32_t)current_block->decelerate_after) {  // DECELERATION PHASE
			#ifdef JKN_ADVANCE
				if ( advance_state == ADVANCE_STATE_ACCEL ) {
					advance_state = ADVANCE_STATE_PLATEAU;
				}
				if ( advance_state == ADVANCE_STATE_PLATEAU ) {
					advance_state = ADVANCE_STATE_DECEL;
					advance_pressure_relax_accumulator = 0;
				}
				advance_pressure_relax_accumulator += current_block->advance_pressure_relax;
			#endif

			// Note that we need to convert deceleration_time from units of
			// 2 MHz to seconds.  That is done by dividing deceleration_time
			// by 2000000.  But, that will make it 0 when we use integer
			// arithmetic.  So, we first multiply block->acceleration_rate by
			// deceleration_time and then do the divide.  However, it's
			// convenient to divide by 2^24 ( >> 24 ).  So, block->acceleration_rate
			// has been prescaled by a factor of 8.388608.

			MultiU24X24toH16(step_rate, deceleration_time, current_block->acceleration_rate);
      
			if(step_rate > acc_step_rate) { // Check step_rate stays positive
				step_rate = current_block->final_rate;
			} else {
				step_rate = acc_step_rate - step_rate; // Decelerate from aceleration end point.
				// lower limit
				if(step_rate < current_block->final_rate)	step_rate = current_block->final_rate;
			}

			// step_rate to timer interval
			timer = calc_timer(step_rate);
			#ifdef OVERSAMPLED_DDA
				STEPPER_OCRnA = timer >> OVERSAMPLED_DDA;
			#else
				STEPPER_OCRnA = timer;
			#endif

			deceleration_time += timer;
		} else {	//NOMINAL PHASE
			#ifdef JKN_ADVANCE
				if ( advance_state == ADVANCE_STATE_ACCEL ) {
					advance_state = ADVANCE_STATE_PLATEAU;
				}
			#endif

			#ifdef OVERSAMPLED_DDA
				STEPPER_OCRnA = OCRnA_nominal >> OVERSAMPLED_DDA;
			#else
				STEPPER_OCRnA = OCRnA_nominal;
			#endif

			step_loops = step_loops_nominal;
		}

		// If current block is finished, reset pointer 
		if (step_events_completed >= current_block->step_event_count) {
			#ifdef JKN_ADVANCE_LEAD_DE_PRIME
				for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {	
					lastAdvanceDeprime[e] = current_block->advance_lead_deprime;
				}
			#endif

			current_block = NULL;
			plan_discard_current_block();
			block_deleted = true;
	
			// Preprocess the setup for the next block if have have one
			current_block = plan_get_current_block();
			if (current_block != NULL) {
				setup_next_block();
			} 
		}   
	} 

	//DEBUG_TIMER_FINISH;
	//debug_onscreen2 = DEBUG_TIMER_TCTIMER_CYCLES;

	return block_deleted;
}



#ifdef JKN_ADVANCE

void st_extruder_interrupt()
{
	uint8_t i;

	//Increment the rate counters
	for ( i = 0; i < EXTRUDERS; i ++ )	st_extruder_interrupt_rate_counter[i] ++;

#if defined(PSTOP_2_SUPPORT)
	if ( e_steps[i] ) extrusion_seen[i] = true;
#endif

	// Set E direction (Depends on E direction + advance)
	for ( i = 0; e_steps[0] &&
		     (st_extruder_interrupt_rate_counter[0] >= st_extruder_interrupt_rate[0]) &&
		     (i < extruder_interrupt_steps_per_call[0]); i ++ ) {

		stepperAxisStep(A_AXIS, false);
		if (e_steps[0] < 0) {
			stepperAxisSetDirection(A_AXIS, false);
			e_steps[0]++;
			stepperAxisStep(A_AXIS, true);
		} 
		else if (e_steps[0] > 0) {
			stepperAxisSetDirection(A_AXIS, true);
			e_steps[0]--;
			stepperAxisStep(A_AXIS, true);
		}

		st_extruder_interrupt_rate_counter[0] = 0;
	}

	#if EXTRUDERS > 1

	for ( i = 0; e_steps[1] &&
		     (st_extruder_interrupt_rate_counter[1] >= st_extruder_interrupt_rate[1]) &&
		     (i < extruder_interrupt_steps_per_call[1]); i ++ ) {

		stepperAxisStep(B_AXIS, false);
		if (e_steps[1] < 0) {
			stepperAxisSetDirection(B_AXIS, false);
			e_steps[1]++;
			stepperAxisStep(B_AXIS, true);
		} 
		else if (e_steps[1] > 0) {
			stepperAxisSetDirection(B_AXIS, true);
			e_steps[1]--;
			stepperAxisStep(B_AXIS, true);
		}

		st_extruder_interrupt_rate_counter[1] = 0;
	}

	#endif
}

#endif // JKN_ADVANCE



void st_init()
{
	#ifdef OVERSAMPLED_DDA
		oversampledCount = 0;
	#endif

	last_active_toolhead = 0;

	#ifdef JKN_ADVANCE
		// Calculate the smallest number of st_extruder_interrupt's between extruder steps based on the
		// st_extruder_interrupt of 10KHz (ADVANCE_INTERRUPT_FREQUENCY).
		for ( uint8_t e = 0; e < EXTRUDERS; e ++ ) {
			float st_extruder_interrupt_ratef = (float)ADVANCE_INTERRUPT_FREQUENCY / (extruder_only_max_feedrate[e] * stepperAxisStepsPerMM(A_AXIS + e));

			// Round up (slower), or if we need more steps than 1 in an interrupt (st_extruder_interrupt_ratef < 1), then
			// we need to process more steps in the loop
			if	( st_extruder_interrupt_ratef > 1.0 ) st_extruder_interrupt_ratef = ceil(st_extruder_interrupt_ratef);
			else if ( st_extruder_interrupt_ratef < 1.0 ) st_extruder_interrupt_ratef = 0;

			st_extruder_interrupt_rate[e] = (uint8_t)st_extruder_interrupt_ratef;

			extruder_interrupt_steps_per_call[e] = 0;
			if ( st_extruder_interrupt_rate[e] == 0 )	extruder_interrupt_steps_per_call[e] = (uint8_t)ceil(1.0 / st_extruder_interrupt_ratef);	
			if ( extruder_interrupt_steps_per_call[e] < 1 )	extruder_interrupt_steps_per_call[e] = 1;

			e_steps[e] = 0;
			st_extruder_interrupt_rate_counter[e] = st_extruder_interrupt_rate[e];
		}
	#endif
}



// Returns true if the block buffer is empty

bool st_empty()
{
	if (block_buffer_head == block_buffer_tail) return true;
	return false;
}



void st_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b)
{
	CRITICAL_SECTION_START;
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
		dda_position[X_AXIS] = x + y;
		dda_position[Y_AXIS] = x - y;
#else
		dda_position[X_AXIS] = x;
		dda_position[Y_AXIS] = y;
#endif
		dda_position[Z_AXIS] = z;
		dda_position[A_AXIS] = a;
		dda_position[B_AXIS] = b;
	CRITICAL_SECTION_END;
}



void st_set_e_position(const int32_t &a, const int32_t &b)
{
	CRITICAL_SECTION_START;
		dda_position[A_AXIS] = a;
		dda_position[B_AXIS] = b;
	CRITICAL_SECTION_END;
}



void st_get_position(int32_t *x, int32_t *y, int32_t *z, int32_t *a, int32_t *b, uint8_t *active_toolhead)
{
	CRITICAL_SECTION_START;
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
	        *x = (dda_position[X_AXIS] + dda_position[Y_AXIS]) / 2;
		*y = (dda_position[X_AXIS] - dda_position[Y_AXIS]) / 2;
#else
		*x = dda_position[X_AXIS];
		*y = dda_position[Y_AXIS];
#endif
		*z = dda_position[Z_AXIS];
		*a = dda_position[A_AXIS];
		*b = dda_position[B_AXIS];
		*active_toolhead = last_active_toolhead;
	CRITICAL_SECTION_END;
}



void quickStop()
{
	DISABLE_STEPPER_DRIVER_INTERRUPT();

		while(blocks_queued())	plan_discard_current_block();

		current_block = NULL;

		CRITICAL_SECTION_START;
#if defined(CORE_XY) || defined(CORE_XY_STEPPER)
		        planner_position[X_AXIS] = (dda_position[X_AXIS] + dda_position[Y_AXIS]) / 2;
		        planner_position[Y_AXIS] = (dda_position[X_AXIS] - dda_position[Y_AXIS]) / 2;
#else
			planner_position[X_AXIS] = dda_position[X_AXIS];
			planner_position[Y_AXIS] = dda_position[Y_AXIS];
#endif
			planner_position[Z_AXIS] = dda_position[Z_AXIS];
			planner_position[A_AXIS] = dda_position[A_AXIS];
			planner_position[B_AXIS] = dda_position[B_AXIS];
		CRITICAL_SECTION_END;

	ENABLE_STEPPER_DRIVER_INTERRUPT();
}



void st_deprime_enable(bool enable)
{
	deprime_enabled = enable;

	for ( uint8_t e = 0; e < EXTRUDERS; e++ ) {
		deprimed[e] = true;
		#ifdef JKN_ADVANCE
			#ifdef JKN_ADVANCE_LEAD_DE_PRIME
				lastAdvanceDeprime[e] = 0;
			#endif
		#endif
	}
}
