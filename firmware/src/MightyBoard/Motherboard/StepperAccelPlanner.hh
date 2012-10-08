/*
  StepperAccelPlanner.hh - buffers movement commands and manages the acceleration profile plan
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

  This module has been heavily modified from the original Marlin (https://github.com/ErikZalm).
  JKN Advance, YAJ (Yet Another Jerk), Advance Pressure Relax and modifications originate from
  Jetty Firmware (https://github.com/jetty840/G3Firmware).  These modifications and features are
  copyrighted and authored by Dan Newman and Jetty under GPL.  Copyright (c) 2012.
*/

// This module is to be considered a sub-module of stepper.c. Please don't include 
// this file from any other module.

#ifndef STEPPERACCELPLANNER_HH
#define STEPPERACCELPLANNER_HH

#include <stdio.h>
#include "avrfix.h"
#include "Configuration.hh"

#ifdef SIMULATOR
	#include "Simulator.hh"
#endif

#ifndef FORCE_INLINE
	#ifdef SIMULATOR
		#define FORCE_INLINE inline
	#else
		#define FORCE_INLINE __attribute__((always_inline)) inline
	#endif
#endif

#ifndef NOFIXED
	#define FIXED
#else
	#ifdef FIXED
		#undef FIXED
	#endif
#endif

//Drop ceil/floor calculations.  Making this available as a #define so we can test timing later
#define NO_CEIL

#ifdef FIXED
	#define FPTYPE			_Accum

	//Various constants we need, we preconvert these to fixed point to save time later
	#define KCONSTANT_MINUS_0_95	-62259		//ftok(-0.95)   
        #define KCONSTANT_0_001         65              //ftok(0.001)
	#define KCONSTANT_0_1		6553		//ftok(0.1)
	#define KCONSTANT_0_25		16384		//ftok(0.25)
	#define KCONSTANT_0_5		32768		//ftok(0.5)
	#define KCONSTANT_0_95		62259		//ftok(0.95)
	#define KCONSTANT_1		65536		//ftok(1.0)
	#define KCONSTANT_3             196608          //ftok(3.0)
	#define KCONSTANT_8_388608	549755		//ftok(8.388608)
	#define KCONSTANT_10		655360		//ftok(10.0)
        #define KCONSTANT_30            1966080         //ftok(30.0)
	#define KCONSTANT_100          	6553600		//ftok(100.0)
	#define KCONSTANT_256		16777216	//ftok(256.0)
	#define KCONSTANT_1000		65536000	//ftok(1000.0)
        #define KCONSTANT_1000000_LSR_16 1000000        //ftok(1000000.0) >> 16

	#ifndef SIMULATOR
		//Type Conversions
		#define FPTOI(x)		ktoli(x)	//FPTYPE  -> int32_t
		#define FPTOI16(x)		ktoi(x)		//FPTYPE  -> int16_t
		#define ITOFP(x)		itok(x)		//int32_t -> FPTYPE
		#define FTOFP(x)		ftok(x)		//float   -> FPTYPE
		#define FPTOF(x)		ktof(x)		//FPTYPE  -> float

		//Arithmetic
		#define FPSQUARE(x)		mulk(x,x)
		#define FPMULT2(x,y)		mulk(x,y)
		#define FPMULT3(x,y,a)		mulk(mulk(x,y),a)
		#define FPMULT4(x,y,a,b)	mulk(mulk(mulk(x,y),a),b)
		#define FPDIV(x,y)		divk(x,y)
		#define FPSQRT(x)		sqrtk(x)
		#define FPABS(x)		absk(x)
		#define FPSCALE2(x)		((x) << 1)
	#else
		//Type Conversions
		#define FPTOI(x)		ktoli(x)	//FPTYPE  -> int32_t
		#define FPTOI16(x)		ktoi(x)		//FPTYPE  -> int16_t
		#define FPTOF(x)		ktof(x)		//FPTYPE  -> float
		#define ITOFP(x)                itofpS((x),__LINE__,__FILE__)  //int32_t -> FPTYPE
		#define FTOFP(x)		ftofpS((x),__LINE__,__FILE__) //float   -> FPTYPE

		//Arithmetic
		#define FPSQUARE(x)		fpsquareS((x),__LINE__,__FILE__)
		#define FPMULT2(x,y)		fpmult2S((x),(y),__LINE__,__FILE__)
		#define FPMULT3(x,y,a)		fpmult3S((x),(y),(a),__LINE__,__FILE__)
		#define FPMULT4(x,y,a,b)	fpmult4S((x),(y),(a),(b),__LINE__,__FILE__)
		#define FPDIV(x,y)		fpdivS((x),(y),__LINE__,__FILE__)
		#define FPSQRT(x)		sqrtk(x)
		#define FPABS(x)		absk(x)
		#define FPSCALE2(x)		fpscale2S((x),,__LINE__,__FILE__)
	#endif		

	#ifndef NO_CEIL
		#define FPCEIL(x)	roundk(x + KCONSTANT_0_5, 3)
	#endif

#else
	#define FPTYPE			float

	//Various constants we need, we preconvert these to fixed point to save time later
	#define KCONSTANT_MINUS_0_95	-0.95   
        #define KCONSTANT_0_001         0.001
	#define KCONSTANT_0_1		0.1
	#define KCONSTANT_0_25		0.25
	#define KCONSTANT_0_5		0.5
	#define KCONSTANT_0_95		0.95
	#define KCONSTANT_1		1.0
	#define KCONSTANT_3             3.0
	#define KCONSTANT_8_388608	8.388608
	#define KCONSTANT_10		10.0
        #define KCONSTANT_30            30.0
	#define KCONSTANT_100          	100.0
	#define KCONSTANT_256		256.0
	#define KCONSTANT_1000		1000.0

	//Type Conversions
	#define FPTOI(x)		(int32_t)(x)	//FPTYPE  -> int32_t
	#define FPTOI16(x)		(int16_t)(x)	//FPTYPE  -> int16_t
	#define ITOFP(x)		(float)(x)	//int32_t -> FPTYPE
	#define FTOFP(x)		(x)		//Do nothing cos we're already float
	#define FPTOF(x)		(x)		//Do nothing cos we're already float

	//Arithmetic
	#define FPSQUARE(x)		((x) * (x))
	#define FPMULT2(x,y)		((x) * (y))
	#define FPMULT3(x,y,a)		((x) * (y) * (a))
	#define FPMULT4(x,y,a,b)	((x) * (y) * (a) * (b))
	#define FPDIV(x,y)		((x) / (y))
	#define FPSQRT(x)		sqrt(x)
	#define FPABS(x)		abs(x)
	#define FPSCALE2(x)		((x) * 2.0)

	#ifndef NO_CEIL
		#define FPCEIL(x)	ceil(x)
	#endif
#endif

#ifdef NO_CEIL
	#define FPCEIL(x)		(KCONSTANT_0_5 + (x))
#endif

//Limits
//The largest / smallest values that can be stored in FPTYPE 
//(which is s15.16, where s is sign and 15 bits = 0x7FFF)
#define FPTYPE_MAX 0x7FFF
#define FPTYPE_MIN (-0x7FFF)

//If defined, support for recording the current move within the block is compiled in
//#define DEBUG_BLOCK_BY_MOVE_INDEX

//Disabled for now, probably does work but needs a non-symmetrical prime/deprime
//If defined, enables advance_lead_prime / advance_lead_deprime
//#define JKN_ADVANCE_LEAD_DE_PRIME

//Use acceleration instead of delta velocity for advance_lead_entry / advance_lead_exit
//Doesn't work with JKN_ADVANCE_LEAD_DE_PRIME at this point
//#define JKN_ADVANCE_LEAD_ACCEL

// The number of linear motions that can be in the plan at any give time.
// THE BLOCK_BUFFER_SIZE NEEDS TO BE A POWER OF 2, i.g. 8,16,32 because shifts and ors are used to do the ringbuffering.
// Values less than 16 would not be wise
#define BLOCK_BUFFER_SIZE 16 // maximize block buffer

// When SAVE_SPACE is defined, the code doesn't take some optimizations which
// which lead to additional program space usage.
//#define SAVE_SPACE

// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in 
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
	// Fields used by the bresenham algorithm for tracing the line
	int32_t		steps[STEPPER_COUNT];			// Step count along each axis
	uint32_t	step_event_count;			// The number of step events required to complete this block
	int32_t		starting_position[STEPPER_COUNT];
	int32_t		accelerate_until;			// The index of the step event on which to stop acceleration
	int32_t		decelerate_after;			// The index of the step event on which to start decelerating
	int32_t		acceleration_rate;			// The acceleration rate used for acceleration calculation
	unsigned char	direction_bits;				// The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
	unsigned char	active_extruder;			// Selects the active extruder
	uint8_t		active_toolhead;			// The toolhead currently active.  Note this isn't the same as active extruder
	#ifdef JKN_ADVANCE
		bool	use_advance_lead;
		int16_t	advance_lead_entry;
		int16_t	advance_lead_exit;
		int32_t	advance_pressure_relax;			//Decel phase only
		int16_t	advance_lead_prime;
		int16_t	advance_lead_deprime;
	#endif

	// Fields used by the motion planner to manage acceleration
	FPTYPE		nominal_speed;				// The nominal speed for this block in mm/min  
	FPTYPE		entry_speed;				// Entry speed at previous-current junction in mm/min
	FPTYPE		max_entry_speed;			// Maximum allowable junction entry speed in mm/min
	FPTYPE		millimeters;				// The total travel of this block in mm
	FPTYPE		acceleration;				// acceleration mm/sec^2
	unsigned char	recalculate_flag;			// Planner flag to recalculate trapezoids on entry junction
	unsigned char	nominal_length_flag;			// Planner flag for nominal speed always reached

	// Settings for the trapezoid generator
	uint32_t	nominal_rate;				// The nominal step rate for this block in step_events/sec 
	int32_t		nominal_rate_sq;			// nominal_rate * nominal_rate
	uint32_t	initial_rate;				// The jerk-adjusted step rate at start of block  
	uint32_t	final_rate;				// The minimal rate at exit
	uint32_t	acceleration_st;			// acceleration steps/sec^2
	char		use_accel;				// Use acceleration when true
	char		speed_changed;				// Entry speed has changed
	volatile char	busy;

	#ifdef SIMULATOR
		FPTYPE	feed_rate;				// Original feed rate before being modified for nomimal_speed
		int	planned;				// Count of the number of times the block was passed to caclulate_trapezoid_for_block()
		char	message[1024];
	#endif

	#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		uint32_t move_index;
	#endif

	uint8_t		dda_master_axis_index;
	uint8_t		axesEnabled;
} block_t;

// Initialize the motion plan subsystem      
void plan_init(FPTYPE extruderAdvanceK, FPTYPE extruderAdvanceK2, bool zhold);

// Add a new linear movement to the buffer.
void plan_buffer_line(FPTYPE feed_rate, const uint32_t &dda_rate, const uint8_t &extruder, bool use_accel, uint8_t active_toolhead);

// Set position. Used for G92 instructions.
void plan_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b);
void plan_set_e_position(const int32_t &a, const int32_t &b);


#ifndef SIMULATOR
	#define SIMULATOR_RECORD(x...)
#else
	#include "SimulatorRecord.hh"
#endif

#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
	extern uint32_t	current_move_index;
#endif

extern uint32_t		minsegmenttime;
extern uint32_t		max_acceleration_units_per_sq_second[STEPPER_COUNT];	// Use M201 to override by software
extern uint32_t		p_acceleration;						// Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
extern uint32_t		p_retract_acceleration;					//  mm/s^2   filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
extern FPTYPE		max_speed_change[STEPPER_COUNT];			//The speed between junctions in the planner, reduces blobbing
extern FPTYPE		smallest_max_speed_change;

extern FPTYPE		minimumSegmentTime;
extern uint32_t		axis_steps_per_sqr_second[STEPPER_COUNT];
extern bool		acceleration_zhold;
extern FPTYPE		delta_mm[STEPPER_COUNT];
extern FPTYPE		planner_distance;
extern FPTYPE		minimumPlannerSpeed;
extern uint32_t		planner_master_steps;
extern uint8_t		planner_master_steps_index;
extern int32_t		planner_steps[STEPPER_COUNT];
extern int		slowdown_limit;
extern int32_t		planner_position[STEPPER_COUNT];
extern int32_t		planner_target[STEPPER_COUNT];
extern uint32_t		axis_accel_step_cutoff[STEPPER_COUNT];

extern block_t		block_buffer[BLOCK_BUFFER_SIZE];			// A ring buffer for motion instfructions

extern volatile unsigned char	block_buffer_head;				// Index of the next block to be pushed
extern volatile unsigned char	block_buffer_tail; 

#ifdef ACCEL_STATS
	extern void accelStatsGet(float *minSpeed, float *avgSpeed, float *maxSpeed);
#endif


// Called when the current block is no longer needed. Discards the block and makes the memory
// availible for new blocks.    
FORCE_INLINE void plan_discard_current_block()  
{
	if (block_buffer_head != block_buffer_tail) {
		block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);  
	}
}

// Gets the current block. Returns NULL if buffer empty
FORCE_INLINE block_t *plan_get_current_block() 
{
	if (block_buffer_head == block_buffer_tail) { 
		return(NULL); 
	}
	block_t *block = &block_buffer[block_buffer_tail];
	block->busy = true;

	return(block);
}

// Gets the current block. Returns NULL if buffer empty
FORCE_INLINE bool blocks_queued() 
{
	if (block_buffer_head == block_buffer_tail) { 
		return false; 
	}
	else	return true;
}

//Returns the number of moves in the planning buffer
FORCE_INLINE uint8_t movesplanned()
{
	return (block_buffer_head-block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
}

#endif
