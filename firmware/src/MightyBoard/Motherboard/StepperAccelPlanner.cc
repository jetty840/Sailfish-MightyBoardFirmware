/*
  StepperAccelPlanner.cc - buffers movement commands and manages the acceleration profile plan
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

  The ring buffer implementation gleaned from the wiring_serial library by David A. Mellis.

  This module has been heavily modified from the original Marlin (https://github.com/ErikZalm).
  JKN Advance, YAJ (Yet Another Jerk), Advance Pressure Relax and modifications originate from
  Jetty Firmware (https://github.com/jetty840/G3Firmware).  These modifications and features are
  copyrighted and authored by Dan Newman and Jetty under GPL.  Copyright (c) 2012.
*/

/*  
  Reasoning behind the mathematics in this module (in the key of 'Mathematica'):
  
  s == speed, a == acceleration, t == time, d == distance

  Basic definitions:

    Speed[s_, a_, t_] := s + (a*t) 
    Travel[s_, a_, t_] := Integrate[Speed[s, a, t], t]

  Distance to reach a specific speed with a constant acceleration:

    Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, d, t]
      d -> (m^2 - s^2)/(2 a) --> estimate_acceleration_distance()

  Speed after a given distance of travel with constant acceleration:

    Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, m, t]
      m -> Sqrt[2 a d + s^2]    

    DestinationSpeed[s_, a_, d_] := Sqrt[2 a d + s^2]

  When to start braking (di) to reach a specified destionation speed (s2) after accelerating
  from initial speed s1 without ever stopping at a plateau:

    Solve[{DestinationSpeed[s1, a, di] == DestinationSpeed[s2, a, d - di]}, di]
      di -> (2 a d - s1^2 + s2^2)/(4 a) --> intersection_distance()

    IntersectionDistance[s1_, s2_, a_, d_] := (2 a d - s1^2 + s2^2)/(4 a)
*/

#ifdef SIMULATOR
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "Simulator.hh"
#endif

#include "Configuration.hh"
#include "StepperAccel.hh"
#include "StepperAccelPlanner.hh"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef SIMULATOR
#include  <avr/interrupt.h>
#include "Motherboard.hh"
#endif

#ifdef abs
#undef abs
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))

#define VEPSILON 1.0e-5

// v1 != v2
#ifdef FIXED
#define VNEQ(v1,v2) ((v1) != (v2))
#define VLT(v1,v2)  ((v1) < (v2))
#else
#define VNEQ(v1,v2) (abs((v1)-(v2)) > VEPSILON)
#define VLT(v1,v2)  (((v1) + VEPSILON) < (v2))
#endif


uint32_t	max_acceleration_units_per_sq_second[STEPPER_COUNT];	// Use M201 to override by software
FPTYPE		smallest_max_speed_change;
FPTYPE		max_speed_change[STEPPER_COUNT];			//The speed between junctions in the planner, reduces blobbing
FPTYPE		minimumPlannerSpeed;
int		slowdown_limit;

bool		disable_slowdown = true;
uint32_t	axis_steps_per_sqr_second[STEPPER_COUNT];

#ifdef JKN_ADVANCE
FPTYPE	extruder_advance_k = 0, extruder_advance_k2 = 0;
#endif

uint8_t         planner_axes;
FPTYPE		delta_mm[STEPPER_COUNT];
FPTYPE		planner_distance;
uint32_t	planner_master_steps;
uint8_t		planner_master_steps_index;
int32_t		planner_steps[STEPPER_COUNT];
FPTYPE		vmax_junction;
uint32_t	axis_accel_step_cutoff[STEPPER_COUNT];

#ifdef CORE_XY
int32_t         delta_ab[2];
#endif

// minimum time in seconds that a movement needs to take if the buffer is emptied.
// Increase this number if you see blobs while printing high speed & high detail.
// It will slowdown on the detailed stuff.
// Comment out to disable
FPTYPE		minimumSegmentTime;

// The current position of the tool in absolute steps
int32_t		planner_position[STEPPER_COUNT];			//rescaled from extern when axisStepsPerMM are changed by gcode
int32_t		planner_target[STEPPER_COUNT];

static FPTYPE	prev_speed[STEPPER_COUNT];
static FPTYPE   prev_final_speed = 0;

#ifdef SIMULATOR
static block_t	*sblock = NULL;
#endif

bool		acceleration_zhold = true;

#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
uint32_t current_move_index = 0;
#endif

#if defined(FIXED) && ! defined(SIMULATOR)

//http://members.chello.nl/j.beentjes3/Ruud/sqrt32avr.htm

static int16_t isqrt1(int16_t value)
{
	int16_t result;

	asm volatile (
		";  Fast and short 16 bits AVR sqrt routine" "\n\t"
		";" "\n\t"
		";  R17:R16 = sqrt(R3:R2), rounded down to integer" "\n\t"
		";" "\n\t"
		";  Registers:" "\n\t"
		";  Destroys the argument in R3:R2" "\n\t"
		";" "\n\t"
		";  Cycles incl call & ret = 90 - 96" "\n\t"
		";" "\n\t"
		";  Stack incl call = 2" "\n\t"
		"Sqrt16:	ldi   %B0,0xc0		; Rotation mask register" "\n\t"
		"		ldi   %A0,0x40		; Developing sqrt" "\n\t"
		"		clc			; Enter loop with C=0" "\n\t"
		"_sq16_1:	brcs  _sq16_2		; C --> Bit is always 1" "\n\t"
		"		cp    %B1,%A0		; Does value fit?" "\n\t"
		"		brcs  _sq16_3		; C --> bit is 0" "\n\t"
		"_sq16_2:	sub   %B1,%A0		; Adjust argument for next bit" "\n\t"
		"		or    %A0,%B0		; Set bit to 1" "\n\t"
		"_sq16_3:	lsr   %B0		; Shift right rotation mask" "\n\t"
		"		lsl   %A1" "\n\t"
		"		rol   %B1		; Shift left argument, C --> Next sub is MUST" "\n\t"
		"		eor   %A0,%B0		; Shift right test bit in developing sqrt" "\n\t"
		"		andi  %B0,0xfe		; Becomes 0 for last bit" "\n\t"
		"		brne  _sq16_1		; Develop 7 bits" "\n\t"
		"		brcs  _sq16_4		; C--> Last bit always 1" "\n\t"
		"		lsl   %A1			; Need bit 7 in C for cpc" "\n\t"
		"		cpc   %A0,%B1		; After this C is last bit" "\n\t"
		"_sq16_4:	adc   %A0,%B0		; Set last bit if C (R17=0)" "\n\t"

		: "=&r" (result)
		: "r" (value)
	);

	return result;
}

#endif


block_t			block_buffer[BLOCK_BUFFER_SIZE];	// A ring buffer for motion instfructions
volatile unsigned char	block_buffer_head;			// Index of the next block to be pushed
volatile unsigned char	block_buffer_tail;			// Index of the block to process now


// Returns the index of the next block in the ring buffer
// NOTE: Removed modulo (%) operator, which uses an expensive divide and multiplication.

#ifndef SAVE_SPACE
FORCE_INLINE
#endif
static uint8_t next_block_index(uint8_t block_index) {
	block_index++;
	if (block_index == BLOCK_BUFFER_SIZE) { block_index = 0; }
	return(block_index);
}


// Returns the index of the previous block in the ring buffer

#ifndef SAVE_SPACE
FORCE_INLINE
#endif
static uint8_t prev_block_index(uint8_t block_index) {
	if (block_index == 0) { block_index = BLOCK_BUFFER_SIZE; }
	block_index--;
	return(block_index);
}


// Calculates the distance (not time) it takes to accelerate from initial_rate to target_rate using the 
// given acceleration:

// Note the equation used below is EXACT: there's no "estimation" involved.
// As such, the name is a bit misleading.

//   t = time
//   a = acceleration (constant)
//   d(t) = distance travelled at time t
//   initial_rate = rate of travel at time t = 0
//   rate(t) = rate of travel at time t
//
// From basic kinematics, we have
//
// [1]   d(t) = d(0) + initial_rate * t + 0.5 * a * t^2,
//
// and
//
// [2]   rate(t) = initial_rate + a * t.
//
// For our purposes, d(0)
//
// [3]   d(0) = 0.
//
// Solving [2] for time t, gives us
//
// [4]   t = ( rate(t) - initial_rate ) / a.
//
// Substituting [3] and [4] into [1] produces,
//
// [5]   d(t) = initial_rate * ( rate(t) - intial_rate ) / a + ( rate(t) - initial_rate )^2 / 2a.
//
// With some algebraic simplification, we then find that d(t) is given by
//
// [6]   d(t) = ( rate(t)^2 - initial_rate^2 ) / 2a.
//
// So, if we know our desired initial rate, initial_rate, and our acceleration, the distance d
// required to reach a target rate, target_rate, is then
//
// [7]  d = ( target_rate^2 - initial_rate^2 ) / 2a.
//
// Note that if the acceleration is 0, we can never reach the target rate unless the
// initial and target rates are the same.  This stands to reason since if the acceleration
// is zero, then our speed never changes and thus no matter how far we move, we're still
// moving at our initial speed.

FORCE_INLINE int32_t estimate_acceleration_distance(int32_t initial_rate_sq, int32_t target_rate_sq, int32_t acceleration_doubled)
{
	if (acceleration_doubled!=0) {
		return((target_rate_sq-initial_rate_sq)/acceleration_doubled);
	} else {
		return 0;  // acceleration was 0, set acceleration distance to 0
	}
}


// This function gives you the point at which you must start braking (at the rate of -acceleration) if 
// you started at speed initial_rate and accelerated until this point and want to end at the final_rate after
// a total travel of distance. This can be used to compute the intersection point between acceleration and
// deceleration in the cases where the trapezoid has no plateau (i.e. never reaches maximum speed)

//
//      accelerate +a         decelerate -a
//     |<---- d1 ---->|<---------- d2 ---------->|
//     |<------------- d = d1 + d2 ------------->|
//    t=0            t=t1                       t=t1+t2
//   initial_rate   peak_rate                 final_rate
//
// From basic kinematics,
//
// [1]  d1 = initial_rate t1 + 0.5 a t1^2
// [2]  d2 = final_rate t2 + 0.5 a t2^2  [think of starting at speed final_rate and accelerating by a]
// [3]  final_rate = initial_rate + a (t1 - t2)
// [4]  d2 = d - d1
//
// We wish to solve for d1 given a, d, initial_rate, and final_rate.
// By the quadratic equation,
//
// [5]  t1 = [ -initial_rate +/- sqrt( initial_rate^2 + 2 a d1 ) ] / a
// [6]  t2 = [ -final_rate +/- sqrt( final_rate^2 + 2 a d2 ) ] / a
//
// Replacing t1 and t2 in [6] then produces,
//
// [7]  final_rate = initial_rate - initial_rate +/- sqrt( initial_rate^2 + 2 a d1 ) +
//                    + final_rate -/+ sqrt( final_rate^2 + 2 a d2 )
//
// And thus,
//
// [8]  +/- sqrt( initial_rate^2 + 2 a d1 ) = +/- sqrt( final_rate^2 + 2 a d2 )
//
// Squaring both sides and substituting d2 = d - d1 [4] yields,
//
// [9]  initial_rate^2 + 2 a d1 = final_rate^2 + 2 a d - 2 a d1
//
// Solving [9] for d1 then gives our desired result,
//
// [10]  d1 =  ( final_rate^2 - initial_rate^2 + 2 a d ) / 4a

FORCE_INLINE int32_t intersection_distance(int32_t initial_rate_sq, int32_t final_rate_sq, int32_t acceleration_doubled, int32_t distance) 
{
	if (acceleration_doubled!=0) {
		return((acceleration_doubled*distance-initial_rate_sq+final_rate_sq)/(acceleration_doubled << 1) );
	} else {
		return 0;  // acceleration was 0, set intersection distance to 0
	}
}


#ifdef JKN_ADVANCE

// Same as final_speed, except this one works with step_rates.
// Regular final_speed will overflow if we use step_rates instead of mm/s

FORCE_INLINE FPTYPE final_speed_step_rate(uint32_t acceleration, uint32_t initial_velocity, int32_t distance) {
     uint32_t v2 = initial_velocity * initial_velocity;

#ifdef SIMULATOR
     uint64_t sum2 = (uint64_t)initial_velocity * (uint64_t)initial_velocity +
	  2 * (uint64_t)acceleration * (uint64_t)distance;
     float fres = (sum2 > 0) ? sqrt((float)sum2) : 0.0;
     FPTYPE result;
#endif

     // Although it's highly unlikely, if target_rate < initial_rate, then distance could be negative.
     if ( distance < 0 ) {
	  uint32_t term2 = (acceleration * (uint32_t)abs(distance)) << 1;

	  if ( term2 >= v2 )	return 0;
	  v2 -= term2;
     }
     else	v2 += (acceleration * (uint32_t)distance) << 1;
     if (v2 <= 0x7fff)
#ifndef SIMULATOR
	  return ITOFP(isqrt1((uint16_t)v2));
#else
#ifndef isqrt1
#define isqrt1(x) ((int32_t)sqrt((float)(x)))
#endif
     result = ITOFP(isqrt1((uint16_t)v2));
#endif
     else {
	  uint8_t n = 0;
	  while (v2 > 0x7fff) {
	       v2 >>= 2;
	       n++;
	  }

#ifndef SIMULATOR
	  return ITOFP(isqrt1((int16_t)v2)) << n;
#else
	  result = ITOFP(isqrt1((int16_t)v2)) << n;
#endif
     }

#ifdef SIMULATOR
     if ((fres != 0.0) && ((fabsf(fres - FPTOF(result))/fres) > 0.01)) {
	  char buf[1024];
	  snprintf(buf, sizeof(buf), "!!! final_speed_step_rate(%d, %d, %d): fixed result = %f; "
		   "float result = %f !!!\n", acceleration, initial_velocity, distance,
		   FPTOF(result), fres);
	  if (sblock)	strlcat(sblock->message, buf, sizeof(sblock->message));
	  else		printf("%s", buf);
     }
     return result;
#endif
}

#endif



// Calculates trapezoid parameters so that the entry- and exit-speed is compensated by the provided factors.

void calculate_trapezoid_for_block(block_t *block, FPTYPE entry_factor, FPTYPE exit_factor) {

	// If exit_factor or entry_factor are larger than unity, then we will scale
	// initial_rate or final_rate to exceed nominal_rate.  However, maximum feed rates
	// have been applied to nominal_rate and as such we should not exceed nominal_rate.

	// For example, if the next block's entry_speed exceeds this block's nominal_speed,
	// then we can be called with exit_factor = next->entry_speed / current->nominal_speed > 1.
	// Basically, that's saying that the next block's nominal speed which was subject to
	// per axis feed rates for a different combination of axes steps can override this block's
	// speed limits.  We don't want that.  For example, if this block's motion is primarily
	// Z or E axis steps and the next block's is all X axis steps, we don't want the next
	// block's X-axis limited entry_speed telling this block that it's Z or E axis limited
	// final_rate can be increased past the block's nominal_rate.

	if ( (!block->use_accel) || (entry_factor > KCONSTANT_1) )	entry_factor = KCONSTANT_1;
	if ( (!block->use_accel) || (exit_factor > KCONSTANT_1) )	exit_factor  = KCONSTANT_1;

	uint32_t initial_rate = (uint32_t)FPTOI(FPCEIL(FPMULT2(ITOFP(block->nominal_rate), entry_factor))); // (step/min)
	uint32_t final_rate   = (uint32_t)FPTOI(FPCEIL(FPMULT2(ITOFP(block->nominal_rate), exit_factor))); // (step/min)

	// If we really need to squeeze cyles, then we can instead do the following
	// but then we'd be testing for rates < 128
	// if (0 == (initial_rate & 0xffffff80)) initial_rate=127;  // initial_rate < 127
	// if (0 == (final_rate & 0xffffff80)) final_rate=127;    // final_rate < 127

	// Limit minimal step rate (Otherwise the timer will overflow.)
	if ( initial_rate < 120 )	initial_rate	= 120;
	if ( final_rate   < 120 )	final_rate	= 120;

	int32_t initial_rate_sq = (int32_t)(initial_rate * initial_rate);
	int32_t final_rate_sq   = (int32_t)(final_rate   * final_rate);
  
	int32_t acceleration = block->acceleration_st;
	int32_t acceleration_doubled = acceleration << 1;
	int32_t accelerate_steps = 0;
	int32_t decelerate_steps = 0;
	if ( block->use_accel ) {
		accelerate_steps = estimate_acceleration_distance(initial_rate_sq, block->nominal_rate_sq, acceleration_doubled);
		decelerate_steps = estimate_acceleration_distance(block->nominal_rate_sq, final_rate_sq, -acceleration_doubled);
	}

	// accelerate_steps = max(accelerate_steps,0); // Check limits due to numerical round-off
	// accelerate_steps = min(accelerate_steps,(int32_t)block->step_event_count);

	// Calculate the size of Plateau of Nominal Rate.
	int32_t plateau_steps = block->step_event_count-accelerate_steps-decelerate_steps;
  
	// Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
	// have to use intersection_distance() to calculate when to abort acceleration and start braking
	// in order to reach the final_rate exactly at the end of this block.
	if (plateau_steps < 0) {
		accelerate_steps = intersection_distance(initial_rate_sq, final_rate_sq, acceleration_doubled, (int32_t)block->step_event_count);
		accelerate_steps = max(accelerate_steps,0); // Check limits due to numerical round-off
		accelerate_steps = min(accelerate_steps,(int32_t)block->step_event_count);
		plateau_steps = 0;
	}
	int32_t decelerate_after = accelerate_steps + plateau_steps;

	#ifdef JKN_ADVANCE
		#ifdef SIMULATOR
			sblock = block;
		#endif
		int16_t advance_lead_entry = 0, advance_lead_exit = 0, advance_lead_prime = 0, advance_lead_deprime = 0;
		int32_t advance_pressure_relax = 0;

		if ( block->use_advance_lead ) {
			uint32_t maximum_rate;

			// Note that we accelerate between step 0 & 1, between 1 & 2, ..., between
			// acclerate_steps-1 & accelerate_steps, AND accelerate_steps & accelerate_steps+1
			// So, we accelerate for accelerate_steps + 1.  This is because in st_interrupt()
			// the test is "if (step_events_completed <= accelerate_until)" which means that
			// between step accelerate_until and the next step, we're still doing acceleration.

			if ( plateau_steps == 0 ) maximum_rate = FPTOI(final_speed_step_rate(block->acceleration_st, initial_rate,
											     accelerate_steps + 1));
			else maximum_rate = block->nominal_rate;

			// Don't waste cycles computing these values if we won't use them in st_interrupt()
			if (accelerate_steps > 0) {
				#ifdef JKN_ADVANCE_LEAD_ACCEL
					// acceleration_st is in units of steps/s^2
					// On the ToM with 1/8th stepping we use >> 4
					// On the Replicator with 1/16th stepping we use >> 5
					// This makes the overflow ranges comparable in mm/s^2 units AND makes the K ranges comparable
					// Acceleration limit to prevent overflow is 0xFFFFF / axis-steps-per-mm
					advance_lead_entry = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)block->acceleration_st >> (1+MICROSTEPPING)));
				#else
					// Acceleration dependent portion

					// Basis of computation is as follows.  Suppose the filament velocity, Vf, should be
					//
					//   [1]  Vf = C1 * Vn + C2 * a
					//
					// where
					//
					//   Vn = extruded noodle velocity
					//    a = acceleration
					//   C1 = constant (ratio of volumes and whatnot)
					//   C2 = another constant
					//
					// But we're normally just taking Vf = C1 * Vn and thus we're missing a contribution
					// associated with acceleration (e.g., a contribution associated with energy loss to
					// friction in the extruder nozzle).  We can then ask, well how many e-steps do we
					// miss as a result?  Using,
					//
					//   [2]  distance in e-space = velocity x time
					//
					// we would then appear to be missing
					//
					//   [3]  delta-e = C2 * a * time-spent-accelerating
					//
					// From basic kinematics, we know the time spent accelerating under constant acceleration,
					//
					//   [4]  Vfinal = Vinitial + a * time-spent-accelerating
					//
					// and thus
					//
					//   [5]  time-spent-accelerating = (Vfinal - Vinitial) / a
					//
					// Substituting [5] into [3] yields,
					//
					//   [6]  delta-e = C2 * (Vfinal - Vinitial)
					//
					// where Vinitial and Vfinal are the initial and final speeds
					// at the start and end of the acceleration or deceleration phase.

					advance_lead_entry = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)(maximum_rate - initial_rate))));
				#endif

				#ifdef JKN_ADVANCE_LEAD_DE_PRIME
					// Prime.  Same as advance_lead_entry, but for when we're priming from 0 to initial_rate
					// Note that we may or may not use this value, it's only used if the previous segment was not available or had e steps of 0
					advance_lead_prime = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)initial_rate)) - KCONSTANT_0_5);
				#endif

				#ifndef SIMULATOR
					if (advance_lead_entry < 0) advance_lead_entry = 0;
					if (advance_lead_prime < 0) advance_lead_prime = 0;
				#endif
			}

			if ((decelerate_after+1) < (int32_t)block->step_event_count) {
				#ifdef JKN_ADVANCE_LEAD_ACCEL
					// acceleration_st is in units of steps/s^2
					// On the ToM with 1/8th stepping we use >> 4
					// On the Replicator with 1/16th stepping we use >> 5
					// This makes the overflow ranges comparable in mm/s^2 units AND makes the K ranges comparable
					// Acceleration limit to prevent overflow is 0xFFFFF / axis-steps-per-mm
					advance_lead_exit = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)block->acceleration_st >> (1+MICROSTEPPING)));
				#else
					// Acceleration dependent portion
					advance_lead_exit = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)(maximum_rate - final_rate))));
				#endif

				#ifdef JKN_ADVANCE_LEAD_DE_PRIME
					// Deprime.  Same as advance_lead_exit, but for when we're depriming from final_rate to 0
					// Note that we may or may not use this value, it's only used if the next segment is not available or has e steps of 0
					advance_lead_deprime = FPTOI16(FPMULT2(extruder_advance_k, ITOFP((int32_t)final_rate)) - KCONSTANT_0_5);
				#endif

				//Pressure relaxation
				if ( extruder_advance_k2 != 0 ) {
					// acceleration_st is in units of steps/s^2
					// On the ToM with 1/8th stepping we use >> 4
					// On the Replicator with 1/16th stepping we use >> 5
					// Acceleration limit to prevent overflow is 0xFFFFF / axis-steps-per-mm
					advance_pressure_relax =
						FPTOI(FPMULT3(extruder_advance_k2, KCONSTANT_100,
							      FPDIV(ITOFP((int32_t)block->acceleration_st >> (1+MICROSTEPPING)),
								    ITOFP((int32_t)decelerate_steps))));
				}
	
				#ifndef SIMULATOR
					//If we've overflowed, reset to 0
					if ( advance_pressure_relax < 0 ) advance_pressure_relax = 0;

					if (advance_lead_exit    < 0) advance_lead_exit = 0;
					if (advance_lead_deprime < 0) advance_lead_deprime = 0;
				#endif
			}

			#ifdef SIMULATOR
				// Owing to roundoff errors, it's not abnormal to see values of -1
				if ( (advance_lead_entry < -1) || (advance_lead_exit < -1) || (advance_pressure_relax < 0) || (advance_pressure_relax >> 8) > 0x7fff) {
					char buf[1024];
					snprintf(buf, sizeof(buf),
						 "!!! calculate_trapezoid_for_block(): advance_lead_entry=%d, advance_lead_exit=%d, "
						 "advance_pressure_relax=%d; initial/nominal/maximum/final_rate=%d/%d/%d/%d; "
						 "accelerate_until/decelerate_after/step_events/plateau_steps=%d/%d/%d/%d; "
						 "i/n/f/a=%d/%d/%d/%d !!!\n",
						 advance_lead_entry, advance_lead_exit, advance_pressure_relax,initial_rate, block->nominal_rate,
						 maximum_rate, final_rate, accelerate_steps, decelerate_after, block->step_event_count,
						 plateau_steps, initial_rate_sq, block->nominal_rate_sq, final_rate_sq, acceleration_doubled);
					strlcat(block->message, buf, sizeof(block->message));
				}
			#endif
		}
	#endif
  
	CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
		if(block->busy == false) { // Don't update variables if block is busy.
			if ( block->use_accel ) {
				block->accelerate_until = accelerate_steps;
				block->decelerate_after = decelerate_after;
			}
			block->initial_rate = initial_rate;
			block->final_rate = final_rate;

			#ifdef JKN_ADVANCE
				block->advance_lead_entry     = advance_lead_entry;
				block->advance_lead_exit      = advance_lead_exit;
				block->advance_lead_prime     = advance_lead_prime;
				block->advance_lead_deprime   = advance_lead_deprime;
				block->advance_pressure_relax = advance_pressure_relax;
			#endif
		}
	CRITICAL_SECTION_END;

	#ifdef SIMULATOR
		block->planned += 1;
	#endif
}                    


// Calculates the speed you must start at in order to reach target_velocity using the 
// acceleration within the allotted distance.
//
// See final_speed() for a derivation of this code.  For initial_speed(), "-a" should
// be used in place of "a" for the acceleration.  And the target_velocity used in place
// of the initial_velocity.
//
// Note bene: if the distance or acceleration is sufficiently large, then there's
//   no initial speed which will work.  The acceleration is so large that the
//   the target velocity will be attained BEFORE the distance is covered.  As
//   such even an initial speed of zero won't work.  When this happens, the value
//   under the square root is negative.  In that case, we simply return a value
//   of zero.

FORCE_INLINE FPTYPE initial_speed(FPTYPE acceleration, FPTYPE target_velocity, FPTYPE distance) {
	#ifdef FIXED
		#ifdef SIMULATOR
			FPTYPE acceleration_original = acceleration;
			FPTYPE distance_original = distance;
			FPTYPE target_velocity_original = target_velocity;
			float  ftv = FPTOF(target_velocity);
			float  fac = FPTOF(acceleration);
			float   fd = FPTOF(distance);
			float fres = ftv * ftv - 2.0 * fac * fd;
			if (fres <= 0.0) fres = 0.0;
			else fres = sqrt(fres);
		#endif

		// We wish to compute
		//
		//    sum2 = target_velocity * target_velocity - 2 * acceleration * distance
		//
		// without having any overflows.  We therefore divide everything in
		// site by 2^12.  After computing sqrt(sum2), we will then multiply
		// the result by 2^6 (the square root of 2^12).

		target_velocity >>= 12;
		acceleration >>= 6;
		distance >>= 5;  // 2 * (distance >> 6) 
		FPTYPE sum2 = FPSQUARE(target_velocity) - FPMULT2(distance, acceleration);

		// Now, comes the real speed up: use our fast 16 bit integer square
		// root (in assembler nonetheles). To pave the way for this, we shift
		// sum2 as much to the left as possible thereby maximizing the use of
	  	// the "whole" or "integral" part of the fixed point number.  We then
		// take the square root of the integer part of sum2 which has been
		// multiplied by 2^(2n) [n left shifts by 2].  After taking the square
		// root, we correct this scaling by dividing the result by 2^n (which
		// is the square root of 2^(2n)

		uint8_t n = 0;
		while ((sum2 != 0) && (sum2 & 0xe0000000) == 0) {
			sum2 <<= 2;
			n++;
		}

		#ifndef SIMULATOR
			// Generate the final result.  We need to undo two sets of
			// scalings: our original division by 2^12 which we rectify
			// by multiplying by 2^6.  But also we need to divide by 2^n
			// so as to counter the 2^(2n) scaling we did.  This means
			// a net multiply by 2^(6-n).
			if	(sum2 <= 0)	return 0;
			else if	(n > 6)		return ITOFP(isqrt1(FPTOI16(sum2))) >> (n - 6);
			else			return ITOFP(isqrt1(FPTOI16(sum2))) << (6 - n);
		#else
			FPTYPE result;
			if (sum2 <= 0)		result = 0;
			#ifndef isqrt1
				#define isqrt1(x) ((int32_t)sqrt((float)(x)))
			#endif
			else			result = ITOFP(isqrt1(FPTOI16(sum2)));

			if (n > 6)		result >>= (n - 6);
			else			result <<= (6 - n);

			if ((fres != 0.0) && ((fabsf(fres - FPTOF(result))/fres) > 0.05)) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "!!! initial_speed(%f, %f, %f): fixed result = %f; float result = %f !!!\n",
					 FPTOF(acceleration_original),
				 	 FPTOF(target_velocity_original),
					 FPTOF(distance_original),
					 FPTOF(result), fres);
				if (sblock)	strlcat(sblock->message, buf, sizeof(sblock->message));
				else		printf("%s", buf);
			}

			return result;
		#endif // SIMULATOR
	#else
		FPTYPE v2 = FPSQUARE(target_velocity) - FPSCALE2(FPMULT2(acceleration, distance));

		if (v2 <= 0)	return 0;
		else		return FPSQRT(v2);
	#endif  // !FIXED
}



// Calculates the final speed (terminal speed) which will be attained if we start at
// speed initial_velocity and then accelerate at the given rate over the given distance
// From basic kinematics, we know that displacement d(t) at time t for an object moving
// initially at speed v0 and subject to constant acceleration a is given by
//
// [1]   d(t) = v0 t + 0.5 a t^2, t >= 0
//
// We also know that the speed v(t) at time t is governed by
//
// [2]   v(t) = v0 + a t
//
// Now, without reference to time, we desire to find the speed v_final we will
// find ourselves moving at after travelling a given distance d.  To find an answer
// to that question, we need to solve one of the two above equations for t and
// then substitute the result into the remaining equation.  As it turns out, the
// algebra is a little easier whne [1] is solved for t using the quadratic
// equation then first solving [2] for t and then plugging into [1] and then
// solving the result for v(t) with the quadratic equation.
//
// So, moving forward and solving [1] for t via the quadratic equation, gives
//
// [3]   t = - [ v0 +/- sqrt( v0^2 - 2ad(t) ) ] / a
//
// Substituting [3] into [2] then 
//
// [4]   v(t) = v0 - [ v0 +/- sqrt( v0^2 + 2ad(t) ) ]
//
// With some minor simplification and droping the (t) notation, we then have
//
// [5]  v = -/+ sqrt( v0^2 + 2ad )
//
// With equation [5], we then know the final speed v attained after accelerating
// from initial speed v0 over distance d.

FORCE_INLINE FPTYPE final_speed(FPTYPE acceleration, FPTYPE initial_velocity, FPTYPE distance) {
	#ifdef FIXED
		//  static int counts = 0;
		#ifdef SIMULATOR
			FPTYPE acceleration_original = acceleration;
			FPTYPE distance_original = distance;
			FPTYPE initial_velocity_original = initial_velocity;
			float  ftv = FPTOF(initial_velocity);
			float  fac = FPTOF(acceleration);
			float   fd = FPTOF(distance);
			float fres = ftv * ftv + 2.0 * fac * fd;
			if (fres <= 0.0)	fres = 0.0;
			else			fres = sqrt(fres);
		#endif

		// We wish to compute
		//
		//    sum2 = initial_velocity * initial_velocity + 2 * acceleration * distance
		//
		// without having any overflows.  We therefore judiciously divide
		// both summands by 2^12.  After computing sqrt(sum2), we will the
		// multiply the result by 2^6 which is he square root of 2^12.

		// Note that when initial_velocity < 1, we lose velocity resolution.
		// When acceleration or distance are < 1, we lose some resolution
		// in them as well.  We're here taking advantage of the fact that
		// the acceleration is usually pretty large as are the velocities.
		// Only the distances are sometimes small which is why we shift the
		// distance the least.  If this were to become a problem, we could
		// shift the acceleration more and the distance even less.  And,
		// when the velocity is tiny relative to the product 2 * a * d,
		// we really don't care as 2 * a * d will then dominate anyway.
		// That is, losing resolution in the velocity is not, in practice,
		// harmful since 2 * a * d will likely dominate in that case.

		initial_velocity >>= 12;
		acceleration >>= 6;
		distance >>= 5;  // 2 * (distance >> 6) 
		FPTYPE sum2 = FPSQUARE(initial_velocity) + FPMULT2(distance, acceleration);

		// Now, comes the real speed up: use our fast 16 bit integer square
		// root (in assembler nonetheles). To pave the way for this, we shift
		// sum2 as much to the left as possible thereby maximizing the use of
		// the "whole" or "integral" part of the fixed point number.  We then
		// take the square root of the integer part of sum2 which has been
		// multiplied by 2^(2n) [n left shifts by 2].  After taking the square
		// root, we correct this scaling by dividing the result by 2^n (which
		// is the square root of 2^(2n)

		uint8_t n = 0;
		while ((sum2 != 0) && (sum2 & 0xe0000000) == 0) {
			sum2 <<= 2;
			n++;
		}

		#ifndef SIMULATOR

			// Generate the final result.  We need to undo two sets of
			// scalings: our original division by 2^12 which we rectify
			// by multiplying by 2^6.  But also we need to divide by 2^n
			// so as to counter the 2^(2n) scaling we did.  This means
			// a net multiply by 2^(6-n).

			if	(sum2 <= 0)	return 0;
			else if	(n > 6)		return ITOFP(isqrt1(FPTOI16(sum2))) >> (n - 6);
			else			return ITOFP(isqrt1(FPTOI16(sum2))) << (6 - n);

			//#ifdef DEBUG_ONSCREEN
			//	Timing code
			//	FPTYPE result;
			//	if (sum2 <= 0) result = 0;
			// 	else if (n > 6) result = ITOFP(isqrt1(FPTOI16(sum2))) >> (n - 6);
			//	else result = ITOFP(isqrt1(FPTOI16(sum2))) << (6 - n);
			//	DEBUG_TIMER_FINISH;
			//	debug_onscreen2 += DEBUG_TIMER_TCTIMER_US;
			//	counts += 1;
			//	debug_onscreen1 = debug_onscreen2 / counts;
			//	return result;
			//#endif
		#else
			#define isqrt1(x) ((int32_t)sqrt((float)(x)))
			FPTYPE result;
			if (sum2 <= 0)	result = 0;
			else		result = ITOFP(isqrt1(FPTOI16(sum2)));
			if (n > 6)	result >>= (n - 6);
			else		result <<= (6 - n);

			if ((fres != 0.0) && ((fabsf(fres - FPTOF(result))/fres) > 0.05)) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "!!! final_speed(%f, %f, %f): fixed result = %f; float result = %f !!!\n",
					 FPTOF(acceleration_original),
					 FPTOF(initial_velocity_original),
					 FPTOF(distance_original),
					 FPTOF(result), fres);
				if (sblock)	strlcat(sblock->message, buf, sizeof(sblock->message));
				else		printf("%s", buf);
			}
			return result;
		#endif // SIMULATOR
	#else
		// Just assume we're doing everything with floating point arithmetic
		// and do not need to worry about overflows or underflows
		FPTYPE v2 = FPSQUARE(initial_velocity) + FPSCALE2(FPMULT2(acceleration, distance));
		if (v2 <= 0)	return 0;
		else		return FPSQRT(v2);
	#endif  // !FIXED
}



// The kernel called by planner_recalculate() when scanning the plan from last to first entry.

void planner_reverse_pass_kernel(block_t *current, block_t *next) {
	if (!current) { return; }
  
	if (next) {
		// If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
		// If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
		// check for maximum allowable speed reductions to ensure maximum possible planned speed.
		if ((current->max_entry_speed - current->entry_speed) > KCONSTANT_3) {
			// If nominal length true, max junction speed is guaranteed to be reached. Only compute
			// for max allowable speed if block is decelerating and nominal length is false.
			if ((!current->nominal_length_flag) && next->speed_changed && (current->max_entry_speed > next->entry_speed)) {
				// We want to know what speed to start at so that if we decelerate -- negative acceleration --
				// over distance current->millimeters, we end up at speed next->entry_speed

				#ifdef SIMULATOR
					sblock = current;
				#endif

				current->entry_speed = min( current->max_entry_speed,
				initial_speed(-current->acceleration,next->entry_speed,current->millimeters));
			} else {
				current->entry_speed = current->max_entry_speed;
			}
			current->speed_changed = true;
			current->recalculate_flag = true;
			}
	} // Skip last block. Already initialized and set for recalculation.
}



// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the reverse pass.

void planner_reverse_pass() {
	uint8_t block_index	= block_buffer_head;
	block_t *block[2]	= { NULL, NULL};

	//Make a local copy of block_buffer_tail, because the interrupt can alter it
	CRITICAL_SECTION_START;
  		unsigned char tail = block_buffer_tail;
	CRITICAL_SECTION_END;

	while(block_index != tail) { 
		block_index = prev_block_index(block_index); 
		block[1]= block[0];
		block[0] = &block_buffer[block_index];
		planner_reverse_pass_kernel(block[0], block[1]);
	}
}



// The kernel called by planner_recalculate() when scanning the plan from first to last entry.

void planner_forward_pass_kernel(block_t *previous, block_t *current) {
	if (!previous || !current->use_accel) { return; }
  
	// If the previous block is an acceleration block, but it is not long enough to complete the
	// full speed change within the block, we need to adjust the entry speed accordingly. Entry
	// speeds have already been reset, maximized, and reverse planned by reverse planner.
	// If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
	if (!previous->nominal_length_flag && previous->speed_changed) {
		if ((((previous->nominal_speed) < current->nominal_speed) &&
		     ((previous->entry_speed + KCONSTANT_3) > current->entry_speed)) ||
		    (((previous->nominal_speed) > current->nominal_speed) &&
		     ((previous->entry_speed + KCONSTANT_3) < current->entry_speed))) {

			// We want to know what the terminal speed from the prior block would be if
			// it accelerated over the entire block with starting speed prev->entry_speed

			#ifdef SIMULATOR
				sblock = previous;
			#endif

			FPTYPE entry_speed = min( current->entry_speed,
						  final_speed(previous->acceleration,previous->entry_speed,previous->millimeters) );

			// Check for junction speed change
			if (VNEQ(current->entry_speed, entry_speed)) {
				current->entry_speed = entry_speed;
				current->recalculate_flag = true;
				current->speed_changed = true;
			}
		}
	}
}



// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the forward pass.

void planner_forward_pass() {
	uint8_t block_index	= block_buffer_tail;
	block_t *block[2]	= { NULL, NULL };

	while(block_index != block_buffer_head) {
		block[0] = block[1];
		block[1] = &block_buffer[block_index];
		planner_forward_pass_kernel(block[0],block[1]);
		block_index = next_block_index(block_index);
	}
}



// Recalculates the trapezoid speed profiles for all blocks in the plan according to the 
// entry_factor for each junction. Must be called by planner_recalculate() after 
// updating the blocks.

void planner_recalculate_trapezoids() {
	uint8_t block_index	= block_buffer_tail;
	block_t *current;
	block_t *next		= NULL;
  
	while(block_index != block_buffer_head) {
		current = next;
		next = &block_buffer[block_index];
		if (current && current->use_accel && next->use_accel) {
			// Recalculate if current block entry or exit junction speed has changed.
			if (current->recalculate_flag || next->recalculate_flag) {
				// NOTE: Entry and exit factors always > 0 by all previous logic operations.
				calculate_trapezoid_for_block(current, FPDIV(current->entry_speed,current->nominal_speed),
							      FPDIV(next->entry_speed,current->nominal_speed));
				current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
			}
		}
		block_index = next_block_index( block_index );
	}

	// Last/newest block in buffer. Exit speed is set with minimumPlannerSpeed. Always recalculated.
	if(next != NULL) {
		FPTYPE scaling = FPDIV(next->entry_speed,next->nominal_speed);
		calculate_trapezoid_for_block(next, scaling, scaling);

		// calculate_trapezoid_for_block(next,
		//    FPDIV(next->entry_speed,next->nominal_speed),
		//    FPDIV(minimumPlannerSpeed,next->nominal_speed));

		next->recalculate_flag = false;
	}
}



// Recalculates the motion plan according to the following algorithm:
//
//   1. Go over every block in reverse order and calculate a junction speed reduction (i.e. block_t.entry_factor) 
//      so that:
//     a. The junction jerk is within the set limit
//     b. No speed reduction within one block requires faster deceleration than the one, true constant 
//        acceleration.
//   2. Go over every block in chronological order and dial down junction speed reduction values if 
//     a. The speed increase within one block would require faster accelleration than the one, true 
//        constant acceleration.
//
// When these stages are complete all blocks have an entry_factor that will allow all speed changes to 
// be performed using only the one, true constant acceleration, and where no junction jerk is jerkier than 
// the set limit. Finally it will:
//
//   3. Recalculate trapezoids for all blocks.

void planner_recalculate() {   
	planner_reverse_pass();
	planner_forward_pass();
	planner_recalculate_trapezoids();
}



void plan_init(FPTYPE extruderAdvanceK, FPTYPE extruderAdvanceK2, bool zhold) {
	#ifdef SIMULATOR
		if ( (B_AXIS+1) != STEPPER_COUNT ) abort();
		if ( (X_AXIS >= STEPPER_COUNT) ||
		     (Y_AXIS >= STEPPER_COUNT) ||
		     (Z_AXIS >= STEPPER_COUNT) ||
		     (A_AXIS >= STEPPER_COUNT) ||
		     (B_AXIS >= STEPPER_COUNT)) abort();
	#endif

	block_buffer_head = 0;
	block_buffer_tail = 0;

	// clear planner_position & prev_speed info
	prev_final_speed = 0;
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
	{
		prev_speed[i] = 0;
		planner_position[i] = 0;
	}

	#ifdef JKN_ADVANCE
		extruder_advance_k  = extruderAdvanceK;
		extruder_advance_k2 = extruderAdvanceK2;
	#endif

	acceleration_zhold = zhold;
	disable_slowdown = true;

	#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		current_move_index = 0;
	#endif
}



// Add a new linear movement to the buffer. 
// planner_target[5] should be set outside this function prior to entry to denote the 
// absolute target position in steps.
// icroseconds specify how many microseconds the move should take to perform. To aid acceleration
// calculation the caller must also provide the physical length of the line in millimeters.
// The stepper module, gaurantees this never gets called with 0 steps

void plan_buffer_line(FPTYPE feed_rate, const uint32_t &dda_rate, const uint8_t &extruder, bool use_accel, uint8_t active_toolhead)
{
	//If we have an empty buffer, then disable slowdown until the buffer has become at least 1/2 full
	//This prevents slow start and gradual speedup at the beginning of a print, due to the SLOWDOWN algorithm
	if ( slowdown_limit && block_buffer_head == block_buffer_tail ) disable_slowdown = true;

	// Calculate the buffer head after we push this byte
	uint8_t next_buffer_head = next_block_index(block_buffer_head);

	// Prepare to set up new block
	block_t *block = &block_buffer[block_buffer_head];

	// Mark block as not busy (Not executed by the stepper interrupt)
	block->busy = false;

	// Note whether block is accelerated or not
	block->use_accel = use_accel;

	// Note the active toolhead
	block->active_toolhead = active_toolhead;

	CRITICAL_SECTION_START;
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		block->starting_position[i] = planner_position[i];
	CRITICAL_SECTION_END;

	#ifdef SIMULATOR
		// Track how many times this block is worked on by the planner
		// Namely, how many times it is passed to calculate_trapezoid_for_block()
		block->planned = 0;
		block->message[0] = '\0';
		sblock = block;
	#endif

	// Number of steps for each axis
	block->steps[X_AXIS] = planner_steps[X_AXIS];
	block->steps[Y_AXIS] = planner_steps[Y_AXIS];
	block->steps[Z_AXIS] = planner_steps[Z_AXIS];
	block->steps[A_AXIS] = planner_steps[A_AXIS];
	block->steps[B_AXIS] = planner_steps[B_AXIS];
	block->step_event_count = planner_master_steps;
	block->dda_master_axis_index = planner_master_steps_index;

	#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		if ( block->use_accel )	block->move_index = ++ current_move_index;
	#endif

	bool extruder_only_move = false;
	if ( 0 == (planner_axes & ((1 << X_AXIS) | (1 << Y_AXIS) | (1 << Z_AXIS))) )
	     extruder_only_move = true;

	#ifdef SIMULATOR
		// Save the original feed rate prior to modification by limits
		block->feed_rate = feed_rate;
	#endif
  
	// Compute direction bits for this block
	block->direction_bits = 0;
#ifndef CORE_XY
	if (planner_target[X_AXIS] < planner_position[X_AXIS]) { block->direction_bits |= (1<<X_AXIS); }
	if (planner_target[Y_AXIS] < planner_position[Y_AXIS]) { block->direction_bits |= (1<<Y_AXIS); }
#else
	if (delta_ab[X_AXIS] < 0) { block->direction_bits |= (1<<X_AXIS); }
	if (delta_ab[Y_AXIS] < 0) { block->direction_bits |= (1<<Y_AXIS); }

	// The following two bits are used to aid in endstop control
	if (delta_ab[X_AXIS] > 0 && delta_ab[Y_AXIS] > 0) { block->direction_bits |= (1 << (X_AXIS + B_AXIS + 1)); }  // +X
	if (delta_ab[X_AXIS] > 0 && delta_ab[Y_AXIS] < 0) { block->direction_bits |= (1 << (Y_AXIS + B_AXIS + 1)); }  // +Y
#endif
	if (planner_target[Z_AXIS] < planner_position[Z_AXIS]) { block->direction_bits |= (1<<Z_AXIS); }
	if (planner_target[A_AXIS] < planner_position[A_AXIS]) { block->direction_bits |= (1<<A_AXIS); }
	if (planner_target[B_AXIS] < planner_position[B_AXIS]) { block->direction_bits |= (1<<B_AXIS); }
  
	//Set block->active_extruder based on either the extruder that has steps,
	//or if 2 extruders have steps, use the current tool index that was passed to this function
	//as extruder.
	if 	( ((1 << A_AXIS) | (1 << B_AXIS)) ==
		  ( planner_axes & ((1 << A_AXIS) | (1 << B_AXIS))) )
		block->active_extruder = extruder;
	else if ( planner_axes & (1 << A_AXIS) )
		block->active_extruder = A_AXIS - A_AXIS;	//0
	else if ( planner_axes & (1 << B_AXIS) )
		block->active_extruder = B_AXIS - A_AXIS;	//1
	else	block->active_extruder = extruder;

	#ifndef SIMULATOR
		//enable active axes

	#ifndef CORE_XY
	       if ( planner_axes & (1 << X_AXIS) ) stepperAxisSetEnabled(X_AXIS, true);
	       if ( planner_axes & (1 << Y_AXIS) ) stepperAxisSetEnabled(Y_AXIS, true);
	#else
	       // Need both steppers holding for Core XY
	       if ( planner_axes & ((1 << X_AXIS) | (1 << Y_AXIS)) )
	       {
		     stepperAxisSetEnabled(X_AXIS, true);
		     stepperAxisSetEnabled(Y_AXIS, true);
		}
	#endif
	       if ( planner_axes & (1 << Z_AXIS) ) stepperAxisSetEnabled(Z_AXIS, true);
	       if ( planner_axes & (1 << A_AXIS) ) stepperAxisSetEnabled(A_AXIS, true);
	       if ( planner_axes & (1 << B_AXIS) ) stepperAxisSetEnabled(B_AXIS, true);

		// Note the current enabled axes
		block->axesEnabled = axesEnabled;

		//Hold Z
		if ( acceleration_zhold ) block->axesEnabled |= _BV(Z_AXIS);
                else if ( 0 == (planner_axes & (1 << Z_AXIS)) )	block->axesEnabled &= ~(_BV(Z_AXIS));
	#endif

	int moves_queued = movesplanned();

	//#ifdef DEBUG_ONSCREEN
	//	if ( moves_queued < 2 )	debug_onscreen1 += 1.0;
	//#endif

	//If we have an empty buffer, then anything "previous" should be wiped
	if ( moves_queued == 0 ) {
		for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
			prev_speed[i] = 0;
	}

	block->nominal_rate = dda_rate;

	#ifndef PLANNER_OFF	//Don't slowdown the buffer if the planner is constrained to a pipeline size of 1

		// SLOWDOWN
		// slow down when the buffer starts to empty, rather than wait at the corner for a buffer refill
		if ( slowdown_limit ) {
			//Renable slowdown if we have half filled up the buffer
			if (( disable_slowdown ) && ( moves_queued >= slowdown_limit ))	disable_slowdown = false;
  
			//If the buffer is less than half full, start slowing down the feed_rate
			//according to how little we have left in the buffer
			if ( moves_queued < slowdown_limit && (! disable_slowdown ) && moves_queued > 1) {
				FPTYPE slowdownScaling = FPDIV(ITOFP(moves_queued), ITOFP((int32_t)slowdown_limit));
				feed_rate = FPMULT2(feed_rate, slowdownScaling);
				block->nominal_rate = (uint32_t)FPTOI(FPMULT2( ITOFP((int32_t)block->nominal_rate), slowdownScaling));
			}
		}

		// END SLOWDOWN
	#endif

	FPTYPE current_speed[STEPPER_COUNT];
	FPTYPE inverse_millimeters = 0;

	//If we have a feed_rate, we calculate some stuff early, because it's also needed for non-accelerated blocks
	if ( feed_rate != 0 ) {
		FPTYPE inverse_second;
		if ( extruder_only_move )	block->millimeters = FPABS(delta_mm[A_AXIS + block->active_extruder]);
		else				block->millimeters = planner_distance;

		inverse_millimeters = FPDIV(KCONSTANT_1, block->millimeters);  // Inverse millimeters to remove multiple divides 

		// Calculate speed in mm/second for each axis. No divide by zero due to previous checks.
		inverse_second = FPMULT2(feed_rate, inverse_millimeters);

		// Calculate speed in mm/sec for each axis
		for(unsigned char i=0; i < STEPPER_COUNT; i++) {
			current_speed[i] = FPMULT2(delta_mm[i], inverse_second);
		}

		// If the user has changed the print speed dynamically, then ensure that
		//   the maximum feedrate limits are observed 
		if ( block->use_accel && steppers::alterSpeed ) {
			FPTYPE speed_factor = KCONSTANT_1;
			for (unsigned char i=0; i < STEPPER_COUNT; i++)
				if ( FPABS(current_speed[i]) > stepperAxis[i].max_feedrate )
					speed_factor = min(speed_factor, FPDIV(stepperAxis[i].max_feedrate, FPABS(current_speed[i])));
			if ( speed_factor < KCONSTANT_1 ) {
				for (unsigned char i=0; i < STEPPER_COUNT; i++)
					current_speed[i] = FPMULT2(current_speed[i], speed_factor);
				feed_rate = FPMULT2(feed_rate, speed_factor);
				block->nominal_rate = (uint32_t)FPTOI(FPMULT2( ITOFP((int32_t)block->nominal_rate), speed_factor));
			}
		}
	}

	//For code clarity purposes, we add to the buffer and drop out here for accelerated blocks
	//Saves having a very long spanning "if"
	if ( ! block->use_accel ) {
		bool docopy = true;

	        // Note that re-assignment here overrides any speed reduction made by the slowdown logic
		block->nominal_rate = dda_rate;

		//Non-accelerated blocks are constrained to max_speed_change
		//But we can only do this if we are the type of move that has a feed rate 
		if ( feed_rate != 0 ) {
			FPTYPE speed_factor = KCONSTANT_1; //factor <=1 do decrease speed

			for(unsigned char i=0; i < STEPPER_COUNT; i++) {
				if(FPABS(current_speed[i]) > max_speed_change[i])
					speed_factor = min(speed_factor, FPDIV(max_speed_change[i], FPABS(current_speed[i])));
			}

			if ( dda_rate > (uint32_t)FPTYPE_MAX ) {
				//Prevent overflows for large dda_rate's
				block->nominal_rate >>= 4;
				block->nominal_rate = (uint32_t)FPTOI(FPMULT2(ITOFP((int32_t)block->nominal_rate), speed_factor));
				block->nominal_rate <<= 4;
			}
			else	block->nominal_rate = (uint32_t)FPTOI(FPMULT2(ITOFP((int32_t)block->nominal_rate), speed_factor));

			if (speed_factor != KCONSTANT_1) {
				for (uint8_t i = 0; i < STEPPER_COUNT; i++)
					prev_speed[i] = FPMULT2(current_speed[i], speed_factor);
				docopy = false;
			}
		}

  		//For non accelerated block, if we set accelerate_until to 0 and decelerate_after to bigger than the number
  		//of steps, then the accelerate / decelerate phases won't fire in acceleration interrupt and we can
  		//avoid having to calculate accelerated stuff that we don't need.
		block->accelerate_until = 0;
		block->decelerate_after = (int32_t)block->step_event_count + 1;

		if ( docopy ) {
			for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
				prev_speed[i] = current_speed[i];
		}

		#ifdef SIMULATOR
		        block->millimeters   = 0;
			block->nominal_speed = feed_rate;
			block->entry_speed   = feed_rate;
		#endif

		// Max rate is sqrt(0x7fffffff) = 46,340.95 steps/s
		block->nominal_rate_sq	= (int32_t)(block->nominal_rate * block->nominal_rate);

		// Add to the buffer
		block_buffer_head = next_buffer_head;
		prev_final_speed = feed_rate;

		// Update position
		CRITICAL_SECTION_START;
			for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
				planner_position[i] = planner_target[i];
		CRITICAL_SECTION_END;
		#ifdef SIMULATOR
			sblock = NULL;
		#endif

		return;
	}

	if ( ! extruder_only_move ) {
		//If we have one item in the buffer, then control it's minimum time with minimumSegmentTime
		if ((moves_queued < 1 ) && (minimumSegmentTime > 0) && ( block->millimeters > 0 ) && 
		    ( feed_rate > 0 ) && (( FPDIV(block->millimeters, feed_rate) ) < minimumSegmentTime)) {
			FPTYPE originalFeedRate  = feed_rate;
			feed_rate = FPDIV(block->millimeters, minimumSegmentTime);
			// block->nominal_rate <= 0x7fff (32,767 steps/s)
			block->nominal_rate = (uint32_t)FPTOI(FPMULT2( ITOFP((int32_t)block->nominal_rate), FPDIV(feed_rate, originalFeedRate)));

			#ifdef SIMULATOR
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "!!! Minimum segment time kicked in: old feed rate=%f; new feed rate=%f !!!\n",
					 FPTOF(originalFeedRate), FPTOF(feed_rate));
					 strlcat(block->message, buf, sizeof(block->message));
			#endif
		}
	}

	block->nominal_rate_sq	= (int32_t)(block->nominal_rate * block->nominal_rate);
	block->nominal_speed	= feed_rate; // (mm/sec) Always > 0

	// Compute and limit the acceleration rate for the trapezoid generator.

	// We're here limited to a max step event count of 0xffff
	// For the Z-axis -- the highest res axis -- that amounts to 163.8 mm (65,535/400)
	// Can increase by shifting right more

	FPTYPE steps_per_mm;
	if (block->step_event_count < 0x7fff)
		steps_per_mm = FPMULT2(ITOFP((int32_t)block->step_event_count), inverse_millimeters);
	else if (block->step_event_count < 0xffff)
		steps_per_mm = FPMULT2(ITOFP((int32_t)block->step_event_count >> 1), inverse_millimeters << 1);
	else if (block->step_event_count < 0x1ffff)
		// Someone had a Z resolution of 630 steps/mm which made a 115.5 mm Z travel exceed 0xffff steps
		steps_per_mm = FPMULT2(ITOFP((int32_t)block->step_event_count >> 2), inverse_millimeters << 2);
	else
		// Switch to floating point.  But if someone has this high of resolution for X | Y
		// then they have bigger problems: not enough CPU cycles to run the stepper interrupt
		// at the necessary frequency.
		steps_per_mm = FTOFP(FPTOF(inverse_millimeters) * (float)block->step_event_count);

	// Limit acceleration per axis
	// Start with the max axial acceleration for an axis
	// with block->step_event_count since we're going to require
	// acceleration_st <= max_acceleration[master-axis] anyway
	block->acceleration_st = axis_steps_per_sqr_second[planner_master_steps_index]; // *
	// (uint32_t)FPTOI(steps_per_mm); // convert to: acceleration steps/sec^2

	// Now skip this axis in our checks
	uint8_t axes = planner_axes & ~(1 << planner_master_steps_index);

	//Assumptions made, due to the high value of acceleration_st / p_retract acceleration, dropped
	//ceil and floating point multiply

	//   Note, we've previously limited step_event_count to 0xffff = 65,536 steps
	//   However, the product of the step count and the max per axis acceleration in steps/s^2 can
	//   overflow a uint32_t for moves with a lot of steps....  So, to prevent overflows, we escape
	//   to 64bits when step_event_count > 0xffffffff / axis_steps_per_sqr_second[i]

	for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
	     if ( axes & (1 << i ) ) {
		  if (block->step_event_count <= axis_accel_step_cutoff[i]) {
		       // We're below the cutoff: do the comparisons in 32 bits
		       if ((block->acceleration_st * (uint32_t)block->steps[i]) > (axis_steps_per_sqr_second[i] * block->step_event_count))
			    // We only need to reduce the acceleration to
			    //
			    //   axis_steps_per_sqr_second[i] * ( step_event_count / steps[i] )
			    //
			    //   block->acceleration_st = (axis_steps_per_sqr_second[i] * (uint32_t)block->step_event_count) / (uint32_t)block->steps[i];
			    //
			    // However, that's computationally more expensive and, more importantly,
			    // doesn't typically yield faster results.  Typically, we're reducing the
			    // acceleration along the axis with step_event_count steps in which
			    // case that ratio of step counts is unity and the division was
			    // unnecessary.  The other axes then require no further reduction in the
			    // acceleration.  So, we just reduce the acceleration to the max for the
			    // axis in question.
			    //
			    // Note that Marlin does the same thing, although there's no code comments
			    // in Marlin indicating if any thought was given to the matter or not.
			    block->acceleration_st = axis_steps_per_sqr_second[i];
		  } else {
		       // Above the cutoffs: do the comparisons in 64 bits
		       if (((uint64_t)block->acceleration_st * (uint64_t)block->steps[i]) >
			   ((uint64_t)axis_steps_per_sqr_second[i] * (uint64_t)block->step_event_count))
			    // block->acceleration_st = (uint32_t)(((uint64_t)axis_steps_per_sqr_second[i] * (uint64_t)block->step_event_count) / (uint64_t)block->steps[i]);
			    block->acceleration_st = axis_steps_per_sqr_second[i];
		  }
	     }
	}

	// Acceleration limit to prevent overflow is 
	if	(block->acceleration_st <= 0x7FFF)
		// Acceleration limit to prevent overflow is 0x7FFF / axis-steps-per-mm
		// good up to about 81.9175 mm/s^2 @ 400 steps/mm || 341.32 mm/s^2 @ 96 steps/mm
		block->acceleration = FPDIV(ITOFP((int32_t)block->acceleration_st), steps_per_mm);
	else if (block->acceleration_st <= 0x1FFFF)
		// Acceleration limit to prevent overflow is 0x1FFFF / axis-steps-per-mm
		// good up to about 327.67 mm/s^2 @ 400 steps/mm || 1,365.3 mm/s^2 @ 96 steps/mm
		block->acceleration = FPDIV(ITOFP(((int32_t)block->acceleration_st)>>2), (steps_per_mm>>2));
	else if (block->acceleration_st <= 0x7FFFF)
		// Acceleration limit to prevent overflow is 0x7FFFF / axis-steps-per-mm
		// good up to 1311 mm/s^2 @ 400 steps/mm || 5,461 mm/s^2 @ 96 steps/mm
		block->acceleration = FPDIV(ITOFP(((int32_t)block->acceleration_st)>>4), (steps_per_mm>>4));
	else
		// Acceleration limit to prevent overflow is 0xFFFFF / axis-steps-per-mm
		// good up to 2,621 mm/s^2 @ 400 steps/mm || 10,922 mm/s^2 @ 96 steps/mm
		// STOP HERE SINCE JKN Advance K2 calculations limit accel to 0xFFFFF / axis-steps-per-mm
		block->acceleration = FPDIV(ITOFP(((int32_t)block->acceleration_st)>>5), (steps_per_mm>>5));

	#if 0
		else if (block->acceleration_st <= 0x1FFFFF)
			// Acceleration limit to prevent overflow is 0x1FFFFF / axis-steps-per-mm
			// good up to 5,243 mm/s^2 @ 400 steps/mm || 21,845 mm/s^2 @ 96 steps/mm
			block->acceleration = FPDIV(ITOFP(((int32_t)block->acceleration_st)>>6), (steps_per_mm>>6));
		else
			// Acceleration limit to prevent overflow is 0x7FFFFF / axis-steps-permm
			// good up to 20,972 mm/s^2 @ 400 steps/mm || 87,379 mm/s^2 @ 96 steps/mm
			block->acceleration = FPDIV(ITOFP(((int32_t)block->acceleration_st)>>8), (steps_per_mm>>8));
	#endif

	// The value 8.388608 derives from the timer frequency used for
	// st_interrupt().  That interrupt is driven by a timer counter which
	// ticks at a frequency of 2 MHz.  To convert counter values to seconds
	// the counter value needs to be divided by 2000000.  So that we
	// can do integer arithmetic (rather than floating point), we first
	// multiply the acceleration by the counter value and THEN divide the
	// result by 2000000.  However, the divide can be done by a shift and
	// it turns out that it is convenient to use >> 24 which is a divide
	// by approximately 16777216.  That's too large by about 8.388608.
	// Therefore, we pre-scale the acceleration here by 8.388608

	//This can potentially overflow in fixed point, due to a large block->acceleration_st,
	//so we don't use fixed point for this calculation
	#ifdef FIXED
		block->acceleration_rate = (int32_t)(((int64_t)block->acceleration_st * 137439) >> 14);
	#else
		block->acceleration_rate = (int32_t)((FPTYPE)block->acceleration_st * 8.388608);
	#endif
  
	//START OF YET_ANOTHER_JERK

	FPTYPE scaling = KCONSTANT_1;
	bool docopy = true;
	if ( moves_queued == 0 ) {
	     vmax_junction = minimumPlannerSpeed;
	     scaling = FPDIV(vmax_junction, block->nominal_speed);
	} else if ( block->nominal_speed <= smallest_max_speed_change ) {
	     vmax_junction = block->nominal_speed;
	     // scaling remains KCONSTANT_1
	} else {
		FPTYPE delta_v;
		for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
			delta_v = FPABS(current_speed[i] - prev_speed[i]);
			if ( delta_v > max_speed_change[i] ) {

				// We wish to moderate max_entry_speed such that delta_v
				// remains <= max_speed_change.  Moreover, any moderation we
				// apply to the speed along this axis, we need to uniformly
				// apply to all axes and, more importantly, to nominal_speed.
				// As such, we need to determine a scaling factor, s.
				FPTYPE s;
				if ( current_speed[i] == 0 ) {
					scaling = 0;
					break;
				}
				else if ( current_speed[i] > prev_speed[i] ) {
					s = FPDIV(prev_speed[i] + max_speed_change[i], current_speed[i]);
				}
				else {
					s = FPDIV(prev_speed[i] - max_speed_change[i], current_speed[i]);
				}
				if ( s < scaling ) {
					if ( s <= 0 ) {
						scaling = 0;
						break;
					}
					scaling = s;
				}
			}
		}

		if (scaling != KCONSTANT_1) {
			vmax_junction = FPMULT2(block->nominal_speed, scaling);
			for (uint8_t i = 0; i < STEPPER_COUNT; i++)
				prev_speed[i] = FPMULT2(current_speed[i], scaling);
			docopy = false;
		} else
			// scaling remains KCONSTANT_1
			vmax_junction = block->nominal_speed;
	}

	if ( docopy ) {
		for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
			prev_speed[i] = current_speed[i];
	}

	//END OF YET ANOTHER JERK

	//#ifdef DEBUG_ONSCREEN
	//	if ( planner_axes & (1 << Z_AXIS) )
	//		debug_onscreen2 = FPTOF(vmax_junction);
	//#endif

	// Initialize block entry speed. Compute based on deceleration to user-defined minimumPlannerSpeed.
	// We want our speed to be AT LEAST fast enough such that we hit minimumPlannerSpeed by the block's end
	// It's okay, however, if we go even faster so take the max of vmax_junction and v_allowable

	// It's the max. speed we can achieve if we accelerate the entire length of the block
	//   starting with an initial speed of minimumPlannerSpeed
	FPTYPE v_allowable = final_speed(block->acceleration,minimumPlannerSpeed,block->millimeters);

	// And this will typically produce a larger value
	if (vmax_junction < minimumPlannerSpeed) {
		block->entry_speed = minimumPlannerSpeed;
		block->max_entry_speed = minimumPlannerSpeed;
	} else {
		// Use final speed of the prior block as starting speed for this block
		block->entry_speed = min(prev_final_speed, vmax_junction);
		block->max_entry_speed = vmax_junction;
	}

	// Initialize planner efficiency flags
	// Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
	// If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
	// the current block and next block junction speeds are guaranteed to always be at their maximum
	// junction speeds in deceleration and acceleration, respectively. This is due to how the current
	// block nominal speed limits both the current and next maximum junction speeds. Hence, in both
	// the reverse and forward planners, the corresponding block junction speed will always be at the
	// the maximum junction speed and may always be ignored for any speed reduction checks.
	if (block->nominal_speed <= v_allowable)	{ block->nominal_length_flag = true; }
	else						{ block->nominal_length_flag = false; }

	block->recalculate_flag = true; // Always calculate trapezoid for new block
	block->speed_changed = false;

	#ifdef JKN_ADVANCE
		block->advance_pressure_relax = 0;
		if ((0 == (planner_axes & ((1 << A_AXIS)|(1 << B_AXIS)))) ||
		    ( extruder_only_move ) || (( extruder_advance_k == 0 ) && ( extruder_advance_k2 == 0))) {
			block->use_advance_lead = false;
			block->advance_lead_entry   = 0;
			block->advance_lead_exit    = 0;
			block->advance_lead_prime   = 0;
			block->advance_lead_deprime = 0;
			block->advance_pressure_relax = 0;
		} else {
			block->use_advance_lead = true;
		}
	#endif

	calculate_trapezoid_for_block(block, scaling, scaling);
	prev_final_speed = FPMULT2(block->nominal_speed, scaling);

	//#ifdef JKN_ADVANCE
	//	if ( block->advance_lead_entry < 0 )	debug_onscreen1 = block->advance_lead_entry;
	//	if ( block->advance_lead_exit < 0 )	debug_onscreen1 = block->advance_lead_exit;
	//	if ( block->advance_lead_prime < 0 )	debug_onscreen1 = block->advance_lead_prime;
	//	if ( block->advance_lead_deprime < 0 )	debug_onscreen1 = block->advance_lead_deprime;
	//	debug_onscreen2 = block->advance_pressure_relax;
	//#endif
    
	// Move buffer head
	block_buffer_head = next_buffer_head;
  
	// Update planner_position
	{
		CRITICAL_SECTION_START;
			for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
				planner_position[i] = planner_target[i];
		CRITICAL_SECTION_END;
	}

	//#ifdef DEBUG_ONSCREEN
	//	#ifdef FIXED
	//		debug_onscreen1 = FPTOF(roundk(FTOFP(2.8934), 3));	//0 = 7 1,2 = 3.0 8 = 2.895
	//		debug_onscreen2 = FPTOF(roundk(FTOFP(2.3846), 3));	//0 = 6 1,2 = 2.5 8 = 2.383
	//	#endif
	//#endif

	planner_recalculate();

	#ifdef SIMULATOR
		sblock = NULL;
	#endif

	return;
}



void plan_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b)
{
	CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
		planner_position[X_AXIS] = x;
		planner_position[Y_AXIS] = y;
		planner_position[Z_AXIS] = z;
		planner_position[A_AXIS] = a;
		planner_position[B_AXIS] = b;

		//If the buffer is empty, we set the stepper position to match
		if ( movesplanned() == 0 ) {
			st_set_position( planner_position[X_AXIS], planner_position[Y_AXIS], planner_position[Z_AXIS],
					 planner_position[A_AXIS], planner_position[B_AXIS] );
		}

	CRITICAL_SECTION_END;  // Fill variables used by the stepper in a critical section
}



void plan_set_e_position(const int32_t &a, const int32_t &b)
{
	CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
		planner_position[A_AXIS] = (int32_t)a;
		planner_position[B_AXIS] = (int32_t)b;

		//If the buffer is empty, we set the stepper position to match
		if ( movesplanned() == 0 ) {
			st_set_e_position( planner_position[A_AXIS], planner_position[B_AXIS] );
		}

	CRITICAL_SECTION_END;  // Fill variables used by the stepper in a critical section
}



#ifdef ACCEL_STATS

//Figure out the acceleration stats by scanning through the command pipeline

void accelStatsGet(float *minSpeed, float *avgSpeed, float *maxSpeed) {
	block_t *block;
	int32_t count = 0;
	uint8_t block_index = block_buffer_tail;

	FPTYPE smax = 0, savg = 0;
	FPTYPE smin = KCONSTANT_1000;

	while(block_index != block_buffer_head) {
		block = &block_buffer[block_index];

		smin = min(smin, block->entry_speed);
		smax = max(smax, block->nominal_speed);
		savg += block->nominal_speed;
	
		block_index = next_block_index(block_index);
		count ++;
	}
	
	if ( count ) {
		//We have stats
		*minSpeed = FPTOF(smin);
		*maxSpeed = FPTOF(smax);
		*avgSpeed = FPTOF(FPDIV(savg, ITOFP(count)));
	} else {
		//We have no stats
		*minSpeed = 0.0;
		*maxSpeed = 0.0;
		*avgSpeed = 0.0;
	}
}

#endif
