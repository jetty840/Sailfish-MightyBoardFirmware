/*
 *   Copyright 2011 by Rob Giseburt http://tinkerin.gs
 *   
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 *   This is heavily influenced by the Marlin RepRap firmware
 *   (https://github.com/ErikZalm/Marlin) which is derived from
 *   the Grbl firmware (https://github.com/simen/grbl/tree).
 */

/* In this implenmentation, the motor control is handled by steppers, but this code does the planning. */

#ifndef PLANNER_HH
#define PLANNER_HH

#include "Types.hh"
#include "Configuration.hh"
#include "Point.hh"

// The number of movements we can plan ahead at a time
// THIS MUST BE A POWER OF 2! 4, 8, 16, 32, you get the idea...
// this is because the circular buffer implementation uses bit shifting for size and increment operations
#define BLOCK_BUFFER_SIZE 8


//TODO: remove TICKS_PER_ACCELERATION
#define TICKS_PER_ACCELERATION   1  // lower is better
const int32_t ACCELERATION_TICKS_PER_SECOND  = (1000000/(INTERVAL_IN_MICROSECONDS*TICKS_PER_ACCELERATION));

// Give the processor some time to breathe and plan...
//#define MIN_MS_PER_SEGMENT_SD 10000
#define MIN_MS_PER_SEGMENT 10000

enum axes{ 
	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2,
	A_AXIS = 3,
	B_AXIS = 4
};

namespace planner {
	// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in 
	// the source g-code and may never actually be reached if acceleration management is active.
	class Block {
	public:
		typedef enum {
			Busy            = 1<<0,
			Recalculate     = 1<<1,
			NominalLength   = 1<<2,
			PlannedToStop   = 1<<3,
			PlannedFromStop = 1<<4,
			Locked          = 1<<5,
		} PlannerFlags;

		// Fields used by the bresenham algorithm for tracing the line
		Point target;                        // Final 5-axis target
		uint16_t step_event_count;           // The number of step events required to complete this block
		uint16_t accelerate_until;            // The index of the step event on which to stop acceleration
		uint16_t decelerate_after;            // The index of the step event on which to start decelerating
		int32_t acceleration_rate;           // The acceleration rate used for acceleration calculation
		

		// Fields used by the motion planner to manage acceleration
		float nominal_speed;                               // The nominal speed for this block in mm/min  
		float entry_speed;                                 // Entry speed at previous-current junction in mm/min
		float max_entry_speed;                             // Maximum allowable junction entry speed in mm/min
		float millimeters;                                 // The total travel of this block in mm
		float acceleration;                                // acceleration mm/sec^2
		float stop_speed;                            // Speed to decelerate to if this is the last move

		// Settings for the trapezoid generator
		uint32_t nominal_rate;                        // The nominal step rate for this block in step_events/sec 
		uint32_t initial_rate;                        // The jerk-adjusted step rate at start of block  
		uint32_t final_rate;                          // The minimal rate at exit
		uint32_t acceleration_st;                     // acceleration steps/sec^2
		uint8_t flags;
		
		Block() : target() {};
		
	// functions
		bool calculate_trapezoid(const float &exit_factor_speed);
	};
	class planner_move_t {
		public:
			Point target;
			uint32_t us_per_step;
			Point steps;
		
			planner_move_t() : target() {};
	};

	/// Initilaize the planner data structures
	void init();
	
	/// Buffer a movement to the target point (in step-space), with us_per_step gaps between steps
	/// \param[in] target New position to move to, in step-space
	/// \param[in] us_per_step Homing speed, in us per step
	void addMoveToBuffer(const Point& target, const int32_t &us_per_step);

	/// Buffer a movement to the target point (in step-space). We should avoid this, as it requires more calculation.
	/// \param[in] target New position to move to, in step-space
	/// \param[in] ms Duration of the move, in milliseconds
	/// \param[in] relative Bitfield specifying whether each axis should
	///                     interpret the new position as absolute or
	///                     relative.
	void addMoveToBufferRelative(const Point& move, const int32_t &ms, const int8_t relative);

	/// Home one or more axes
	/// \param[in] maximums If true, home in the positive direction
	/// \param[in] axes_enabled Bitfield specifiying which axes to
	///                         home
	/// \param[in] us_per_step Homing speed, in us per step
	void startHoming(const bool maximums,
	                 const uint8_t axes_enabled,
	                 const uint32_t us_per_step);

	/// Reset the current system position to the given point
	/// \param[in] position New system position
	void definePosition(const Point& position);

	/// Abort the current motion (and all planeed movments) and set the stepper subsystem to
	/// the not-running state.
	void abort();

	/// Get the current system position
	/// \return The current machine position.
	const Point getPosition();

	void setMaxXYJerk(float jerk);
	void setMaxAxisJerk(float jerk, uint8_t axis);

	void setMinimumPlannerSpeed(float speed);
	void setAxisMaxLength(int32_t length, uint8_t axis, bool max);
	void setAcceleration(int32_t acceleration);
	void setAxisAcceleration(int32_t new_acceleration, uint8_t axis);
#ifdef CENTREPEDAL
	void setJunctionDeviation(float new_junction_deviation);
#endif	
	void setAxisStepsPerMM(float steps_per_mm, uint8_t axis);
	
	bool isBufferFull();
	bool isBufferEmpty();
	bool isReady();
	
	// Fetches the *tail*
	Block *getNextBlock();
	
	// pushes the tail forward, making it available
	void doneWithNextBlock();
	
	// how many items are in the buffer
	uint8_t bufferCount();

	/// Change active tool.  Applies offsets to tool for nozzle separation
	void changeToolIndex(uint8_t tool);

	
	void runStepperPlannerSlice();
	bool planNextMove(Point& target, const int32_t us_per_step, const Point& steps);
	void setAccelerationOn(bool on);
}

#endif /* end of include guard: PLANNER_HH */
