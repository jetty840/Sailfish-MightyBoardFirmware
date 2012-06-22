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

	vt + (1/2)at^2=X for t
		x + vt + (1/2)at^2=X
		x = destination
		X = position
		v = current speed
		a = acceleration rate
		t = time
		See: http://www.wolframalpha.com/input/?i=vt+%2B+%281%2F2%29at%5E2%3DX+for+t
	
	Solved for time, simplified (with a few multiplications as possible) gives:
	(sqrt(v^2-2*a*(x-X)) - v)/a
	So, making x-X = D gives:
	(sqrt(v*v-2*a*D)-v)/a = time to accelerate from velocity v over D steps with acceleration a
	
*/


#include "Planner.hh"
#include <util/atomic.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> // for memmove and memcpy
#include "SDCard.hh" // sdcard::isPlaying()

#include "Steppers.hh"
#include "Point.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"



#define  FORCE_INLINE __attribute__((always_inline)) inline

/* Setup some utilities */

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

template <typename T>
inline const T& min(const T& a, const T& b) { return (a)<(b)?(a):(b); }

template <typename T>
inline const T& max(const T& a, const T& b) { return (a)>(b)?(a):(b); }

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#ifdef labs
#undef labs
#endif

template <typename T>
inline T abs(T x) { return (x)>0?(x):-(x); }

template <>
inline int abs(int x) { return __builtin_abs(x); }

template <>
inline long abs(long x) { return __builtin_labs(x); }


namespace planner {
	
	// Super-simple circular buffer, where old nodes are reused
	// TODO: Move to a seperate file
	// WARNING WARNING WARNING: The buffer must be sized a power of two, otherwise operations will not work
	// (2, 4, 8, 16, 32, 64, 128)
	template<typename T>
	class ReusingCircularBufferTempl
	{
	public:
		typedef T BufDataType;
		typedef uint8_t BufSizeType;
		
	private:
		volatile BufSizeType head, tail;
		// volatile bool full;
		BufSizeType size;
		BufSizeType size_mask;
		BufDataType* const data; /// Pointer to buffer data
	
	public:
		///@param buffer_in pre-allocaed data of <T>
		///@param size_in count in elements of buffer_in
		ReusingCircularBufferTempl(BufSizeType size_in, BufDataType* buffer_in) : head(0), tail(0), /*full(false),*/ size(size_in), size_mask(size_in-1), data(buffer_in) {
			for (BufSizeType i = 0; i < size; i++) {
				data[i] = BufDataType();
			}
		};
		
		inline BufDataType *getHead() {
			return &data[head];
		}
		inline BufSizeType getHeadIndex() {
			return head;
		}
		
		inline BufDataType *getTail() {
			return &data[tail];
		}
		inline BufSizeType getTailIndex() {
			return tail;
		}
		
		inline BufSizeType getNextIndex(BufSizeType from) {
			return ((from + 1) & size_mask);
		}
		
		inline BufSizeType getPreviousIndex(BufSizeType from) {
			return (((from+size) - 1) & size_mask);
		}
		
		inline BufDataType *getNextHead() {
			return &data[getNextIndex(head)];
		}
		
		inline BufDataType &operator[] (BufSizeType index) {
			 // adding size should make negative indexes < size work ok
			// int16_t offset = index < 0 ? index : ((index + size) & size_mask);
			return data[index];
		}
		
		// bump the head. cannot return anything useful, so it doesn't
		// WARNING: no sanity checks!
		inline void bumpHead() {
			head = getNextIndex(head);
		}

		// bump the tail. cannot return anything useful, so it doesn't
		// WARNING: no sanity checks!
		inline void bumpTail() {
			tail = getNextIndex(tail);
		}
		
		inline bool isEmpty() {
			return head == tail;
		}
		
		inline bool isFull() {
			return (getNextIndex(head) == tail);
		}
		
		inline BufSizeType getUsedCount() {
			return ((head-tail+size) & size_mask);
		}
		
		inline void clear() {
			head = 0;
			tail = 0;
		}
	};
	
	// this is very similar to the StepperAxis, but geared toward planning
	struct PlannerAxis
	{
		// how many steps does it take to go a mm (RepG should tell us this during init)
		float steps_per_mm;
		
		// how fast can we go, in mm/s (RepG should have already limited this, disabling)
		// float max_feedrate;

		// min and max length for the axis
		int32_t max_length;
		int32_t min_length;
		
		// maximum acceleration for this axis in steps/s^2 (should be in EEPROM)
		uint32_t max_acceleration;
		
		// the maximum amount of speed change allowable for this axis
		// note that X+Y has it's own setting, and this if for all the rest
		float max_axis_jerk;
	};
	
	PlannerAxis axes[STEPPER_COUNT];
	
	float default_acceleration;
	float minimum_planner_speed;
	Point position; // the current position (planning-wise, not bot/stepper-wise) in steps
	float previous_speed[STEPPER_COUNT]; // Speed of previous path line segment
#ifdef CENTREPEDAL
	float default_junction_deviation;
	float previous_unit_vec[3];
#endif
	float previous_nominal_speed; // Nominal speed of previous path line segment
	static float max_xy_jerk;
	
	/// List of accelerated moves to execute
	Block block_buffer_data[BLOCK_BUFFER_SIZE];
	ReusingCircularBufferTempl<Block> block_buffer(BLOCK_BUFFER_SIZE, block_buffer_data);
	
	bool accelerationON = true;

	Point tolerance_offset_T0;
	Point tolerance_offset_T1;
	Point *tool_offsets;
	
	uint32_t additional_ms_per_segment;
	
	volatile bool force_replan_from_stopped;
	
	void init()
	{
		/// if eeprom has not been initialized. store default values
		if (eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0xFFFFFFFF) == 0xFFFFFFFF) {
			eeprom::storeToolheadToleranceDefaults();
		}
		
		setAxisStepsPerMM(XSTEPS_PER_MM,0);           
		setAxisStepsPerMM(YSTEPS_PER_MM,1);               
		setAxisStepsPerMM(ZSTEPS_PER_MM,2);
		setAxisStepsPerMM(ASTEPS_PER_MM,3);
		setAxisStepsPerMM(BSTEPS_PER_MM,4);
		
		// check that acceleration settings have been initialized 
		// if not, load defaults
		uint8_t accelerationStatus = eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::DEFAULTS_FLAG, 0xFF);
		if(accelerationStatus !=  _BV(ACCELERATION_INIT_BIT)){
			eeprom::setDefaultsAcceleration();
		}
		
		// Master acceleration
		setAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACCELERATION_RATE_OFFSET, DEFAULT_ACCELERATION));
		
		setAxisAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_RATES_OFFSET+ 0, DEFAULT_X_ACCELERATION), 0);        
		setAxisAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_RATES_OFFSET+ 2, DEFAULT_Y_ACCELERATION), 1);
		setAxisAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_RATES_OFFSET+ 4, DEFAULT_Z_ACCELERATION), 2);
		setAxisAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_RATES_OFFSET+ 6, DEFAULT_A_ACCELERATION), 3);
		setAxisAcceleration((int32_t)eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_RATES_OFFSET+ 8, DEFAULT_B_ACCELERATION), 4);

		setMaxXYJerk(eeprom::getEepromFixed16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_JERK_OFFSET, DEFAULT_MAX_XY_JERK));
		setMaxAxisJerk(eeprom::getEepromFixed16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_JERK_OFFSET + 4, DEFAULT_MAX_Z_JERK), 2);
		setMaxAxisJerk(eeprom::getEepromFixed16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_JERK_OFFSET + 6, DEFAULT_MAX_A_JERK), 3);
		setMaxAxisJerk(eeprom::getEepromFixed16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::AXIS_JERK_OFFSET + 8, DEFAULT_MAX_B_JERK), 4);

		minimum_planner_speed = eeprom::getEeprom16(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::MINIMUM_SPEED, DEFAULT_MIN_SPEED);
		
		// Z axis max and min
		setAxisMaxLength(eeprom::getEeprom32(eeprom_offsets::AXIS_LENGTHS + 4*2, replicator_axis_lengths::axis_lengths[2]), 2, true);
		setAxisMaxLength(0, 2, false);

		abort();


#ifdef CENTREPEDAL
		previous_unit_vec[0]= 0.0;
		previous_unit_vec[1]= 0.0;
		previous_unit_vec[2]= 0.0;
#endif
	}

	void setAccelerationOn(bool on){
		accelerationON = on;
	}
	
	void setMaxAxisJerk(float jerk, uint8_t axis) {
		if (axis < STEPPER_COUNT)
			axes[axis].max_axis_jerk = jerk;
	}
	
	void setAxisMaxLength(int32_t length, uint8_t axis, bool max){
		if (axis < STEPPER_COUNT){
			if (max){
				axes[axis].max_length = length;
			} else {
				axes[axis].min_length = length;
			}
		}
	}
	
	
	void setMaxXYJerk(float jerk) {
		max_xy_jerk = jerk;
	}
	
	void setAxisStepsPerMM(float steps_per_mm, uint8_t axis) {
		if (axis < STEPPER_COUNT)
			axes[axis].steps_per_mm = steps_per_mm;
	}

	void setAcceleration(int32_t new_acceleration) {
		default_acceleration = (float)new_acceleration;
	}
	
	// This is in steps/mm.
	void setAxisAcceleration(int32_t new_acceleration, uint8_t axis) {
		if (axis < STEPPER_COUNT)
			axes[axis].max_acceleration = (float)new_acceleration*axes[axis].steps_per_mm;
	}

#ifdef CENTREPEDAL
	void setJunctionDeviation(float new_junction_deviation) {
		default_junction_deviation = new_junction_deviation;
	}
#endif
	
	// Calculates the maximum allowable speed at this point when you must be able to reach target_velocity using the 
	// acceleration within the allotted distance.
	// Needs to be conbverted to fixed-point.
	FORCE_INLINE float max_allowable_speed(const float &acceleration, const float &target_velocity, const float &distance) {
		return sqrt((target_velocity*target_velocity)-(acceleration*2.0)*distance);
	}

	// Calculates the distance (not time) it takes to accelerate from initial_rate to target_rate using the 
	// given acceleration:
	FORCE_INLINE int32_t estimate_acceleration_distance(const int32_t &initial_rate_squared, const int32_t &target_rate_squared, const int32_t &acceleration_doubled)
	{
		if (acceleration_doubled!=0) {
			return (target_rate_squared-initial_rate_squared)/acceleration_doubled;
		}
		else {
			return 0;  // acceleration was 0, set acceleration distance to 0
		}
	}

	// This function gives you the point at which you must start braking (at the rate of -acceleration) if 
	// you started at speed initial_rate and accelerated until this point and want to end at the final_rate after
	// a total travel of distance. This can be used to compute the intersection point between acceleration and
	// deceleration in the cases where the trapezoid has no plateau (i.e. never reaches maximum speed)

	FORCE_INLINE int32_t intersection_distance(const int32_t &initial_rate_squared, const int32_t &final_rate_squared, const int32_t &acceleration_mangled, const int32_t &acceleration_quadrupled, const int32_t &distance) 
	{
		if (acceleration_quadrupled!=0) {
			return (acceleration_mangled*distance-initial_rate_squared+final_rate_squared)/acceleration_quadrupled;
		}
		else {
			return 0;  // acceleration was 0, set intersection distance to 0
		}
	}

// Disabled because it's not used, but if it is in the future, here's how
#if 0
	// Calculates the time (not distance) in microseconds (S*1,000,000) it takes to go from initial_rate for distance at acceleration rate
	FORCE_INLINE uint32_t estimate_time_to_accelerate(float initial_rate, float acceleration, float distance) {

		if (acceleration!=0.0) {
			return abs((sqrt(2*acceleration*distance + initial_rate*initial_rate)-initial_rate)/acceleration) * 1000000;
		}
		else {
			return (distance/initial_rate) * 1000000; // no acceleration is just distance/rate
		}
	}
#endif

	// Calculates trapezoid parameters so that the entry- and exit-speed is compensated by the provided factors.
	// calculate_trapezoid_for_block(block, block->entry_speed/block->nominal_speed, exit_factor_speed/block->nominal_speed);
	bool Block::calculate_trapezoid(const float &exit_factor_speed) {

		float entry_factor = entry_speed/nominal_speed;
		float exit_factor = exit_factor_speed/nominal_speed;
		
		uint32_t local_initial_rate = ceil((float)nominal_rate*entry_factor); // (step/min)
		uint32_t local_final_rate = ceil((float)nominal_rate*exit_factor); // (step/min)
		
		// Limit minimal step rate (Otherwise the timer will overflow.)
		if(local_initial_rate < 120)
			local_initial_rate = 120;
		if(local_final_rate < 120)
			local_final_rate = 120;
		
		int32_t local_initial_rate_squared = (local_initial_rate * local_initial_rate);
		int32_t local_final_rate_squared   = (local_final_rate   * local_final_rate);
		int32_t nominal_rate_squared       = (nominal_rate       * nominal_rate);
		
		int32_t local_acceleration_doubled = acceleration_st<<(1); // == acceleration_st*2
		
		int32_t accelerate_steps =
			/*ceil*/(estimate_acceleration_distance(local_initial_rate_squared, nominal_rate_squared, local_acceleration_doubled));
		int32_t decelerate_steps =
			/*floor*/(estimate_acceleration_distance(nominal_rate_squared, local_final_rate_squared, -local_acceleration_doubled));

		// Calculate the size of Plateau of Nominal Rate.
		int32_t plateau_steps = step_event_count-accelerate_steps-decelerate_steps;

		// Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
		// have to use intersection_distance() to calculate when to abort acceleration and start braking
		// in order to reach the local_final_rate exactly at the end of this block.
		
		if (plateau_steps < 0) {

			// To get the math right when shifting, we need to alter the first acceleration_doubled by bit_shift_amount^2, and un-bit_shift_amount^2 after
			int32_t local_acceleration_quadrupled = local_acceleration_doubled<<(1); // == acceleration_st*2
			accelerate_steps = /*ceil*/(
				intersection_distance(local_initial_rate_squared, local_final_rate_squared, local_acceleration_doubled, local_acceleration_quadrupled, step_event_count));
			accelerate_steps = max(accelerate_steps, 0L); // Check limits due to numerical round-off
			
			accelerate_steps = min(accelerate_steps, (int32_t)step_event_count);
			plateau_steps = 0;

		}

		//bool successfully_replanned = true;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // Fill variables used by the stepper in a critical section
				accelerate_until = accelerate_steps;
				decelerate_after = accelerate_steps+plateau_steps;
				initial_rate     = local_initial_rate;
				final_rate       = local_final_rate;
		
		} // ISR state will be automatically restored here
		
	
		
		return true; //successfully_replanned;
	}
	
	// forward declare, so we can order the code in a slightly more readable fashion
	inline void planner_reverse_pass_kernel(Block *previous, Block *current, Block *next);
	void planner_reverse_pass();
	inline void planner_forward_pass_kernel(Block *previous, Block *current, Block *next);
	void planner_forward_pass();
	bool planner_recalculate_trapezoids();

	// Recalculates the motion plan according to the following algorithm:
	//
	//   1. Go over every block in reverse order and calculate a junction speed reduction (i.e. block_t.entry_speed) 
	//      so that:
	//     a. The junction speed is equal to or less than the maximum junction speed limit
	//     b. No speed reduction within one block requires faster deceleration than the one, true constant 
	//        acceleration.
	//   2. Go over every block in chronological order and dial down junction speed values if 
	//     a. The speed increase within one block would require faster acceleration than the one, true 
	//        constant acceleration.
	//
	// When these stages are complete all blocks have an entry speed that will allow all speed changes to 
	// be performed using only the one, true constant acceleration, and where no junction speed is greater
	// than the max limit. Finally it will:
	//
	//   3. Recalculate trapezoids for all blocks using the recently updated junction speeds. Block trapezoids
	//      with no updated junction speeds will not be recalculated and assumed ok as is.
	//
	// All planner computations are performed with doubles (float on Arduinos) to minimize numerical round-
	// off errors. Only when planned values are converted to stepper rate parameters, these are integers.

	void planner_recalculate() {   
			planner_reverse_pass();
			planner_forward_pass();
			planner_recalculate_trapezoids();

	}

	// The kernel called by planner_recalculate() when scanning the plan from last to first entry.
	inline void planner_reverse_pass_kernel(Block *current, Block *next) {
		if(!current) { return; }

		// If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
		// If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
		// check for maximum allowable speed reductions to ensure maximum possible planned speed.
		// We cannot change the entry speed of a Busy block ... we've already entered...
		if (current->entry_speed != current->max_entry_speed && !current->flags & Block::Busy) {
			// If nominal length true, max junction speed is guaranteed to be reached. Only compute
			// for max allowable speed if block is decelerating and nominal length is false.
			if ((!(current->flags & Block::NominalLength)) && (current->max_entry_speed >= next->entry_speed)) {
				current->entry_speed = min( current->max_entry_speed,
					max_allowable_speed(-current->acceleration,next->entry_speed,current->millimeters));
			} else {
				current->entry_speed = current->max_entry_speed;
			}
			current->flags |= Block::Recalculate;
		}
	}

	// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
	// implements the reverse pass.
	void planner_reverse_pass() {
		if (block_buffer.getUsedCount() > 1) {
			uint8_t block_index = block_buffer.getHeadIndex();
			Block *block[2] = { &block_buffer[block_index], NULL };
			do { 
				block_index = block_buffer.getPreviousIndex(block_index); 
				block[1] = block[0];
				block[0] = &block_buffer[block_index];
				planner_reverse_pass_kernel(block[0], block[1]);
			} while (block_index != block_buffer.getTailIndex());
		}
	}

	// The kernel called by planner_recalculate() when scanning the plan from first to last entry.
	inline void planner_forward_pass_kernel(Block *previous, Block *current, Block *next) {
		if(!previous) { return; }

		// If the previous block is an acceleration block, but it is not long enough to complete the
		// full speed change within the block, we need to adjust the entry speed accordingly. Entry
		// speeds have already been reset, maximized, and reverse planned by reverse planner.
		// If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
		if (!(previous->flags & Block::NominalLength)) {
			if (previous->entry_speed <= current->entry_speed) {
				float entry_speed = min( current->entry_speed,
					max_allowable_speed(-previous->acceleration,previous->entry_speed,previous->millimeters) );

				// Check for junction speed change
				if (current->entry_speed != entry_speed) {
					current->entry_speed = entry_speed;
					current->flags |= Block::Recalculate;
				}
			}
		}
	}

	// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
	// implements the forward pass.
	void planner_forward_pass() {
		uint8_t block_index = block_buffer.getTailIndex();
		Block *block[3] = { NULL, NULL, NULL };

		while(block_index != block_buffer.getHeadIndex()) {
			block[0] = block[1];
			block[1] = block[2];
			// Move two blocks worth of ram, from [1] to [0], using the overlap-safe memmove
			//memmove(block[1], block[0], sizeof(Block)<<1);
			block[2] = &block_buffer[block_index];
			planner_forward_pass_kernel(block[0],block[1],block[2]);
			block_index = block_buffer.getNextIndex(block_index);
		}
		planner_forward_pass_kernel(block[1], block[2], NULL);
	}

	// Recalculates the trapezoid speed profiles for all blocks in the plan according to the 
	// entry_factor for each junction. Must be called by planner_recalculate() after 
	// updating the blocks.
	// Returns true if planning succeded.
	bool planner_recalculate_trapezoids() {
		int8_t block_index = block_buffer.getTailIndex();
		Block *current;
		Block *next = NULL;
		
		while(block_index != block_buffer.getHeadIndex()) {
			current = next;
			next = &block_buffer[block_index];
			if (current) {
				// Recalculate if current block entry or exit junction speed has changed.
				if ((current->flags & Block::Recalculate) || (next->flags & Block::Recalculate) && !(current->flags & Block::Busy)) {
					// NOTE: Entry and exit factors always > 0 by all previous logic operations.
					current->calculate_trapezoid(next->entry_speed);
				
					// Reset current only to ensure next trapezoid is computed
					// Also make sure the PlannedToStop flag gets cleared, since we are planning to the next move
					current->flags &= ~Block::Recalculate;//(Block::Recalculate|Block::PlannedToStop);
				}
			}
			block_index = block_buffer.getNextIndex( block_index );
		}
		
		// Last/newest block in buffer. Exit speed is set with stop_speed. Always recalculated.
		next->calculate_trapezoid(next->stop_speed);
		next->flags &= ~Block::Recalculate;
		return true;
	}

	bool isBufferFull() {
		return block_buffer.isFull();//planner_buffer.isFull(); 
	}
	
	// Are we completely out of upcoming moves?
	bool isBufferEmpty() {
		bool is_buffer_empty = block_buffer.isEmpty();// && planner_buffer.isEmpty();
		return is_buffer_empty;
	}
	
	// Is the move in the buffer ready to use?
	bool isReady() {
		return !(force_replan_from_stopped || block_buffer.isEmpty());
	}
	
	uint8_t bufferCount() {
		return block_buffer.getUsedCount();
	}
	
	Block *getNextBlock() {
		Block *block = block_buffer.getTail();
		return block;
	}
	
	void doneWithNextBlock() {
		block_buffer.bumpTail();
	}

	/// Command calls this to add a move to the buffer,
	/// so that the planner will generate an acceleration trapezoid
	void addMoveToBufferRelative(const Point& move, const int32_t &ms, const int8_t relative)
	{
		
		Point target = move + *tool_offsets;
		int32_t max_delta = 0;
		for (int i = 0; i < STEPPER_COUNT; i++) {
			int32_t delta = 0;
			if ((relative & (1 << i))) {
				target[i] = position[i] + move[i];
				delta = abs(move[i]);
			} else {
				target[i] = move[i] + (*tool_offsets)[i];
				delta = abs(target[i] - position[i]);
				
			}
			if (delta > max_delta) {
				max_delta = delta;
			}
		}
		/// Clip Z axis so that plate cannot attempt to move out of build area
		/// other axis clipping will be added in a future revision
		if(target[Z_AXIS] > axes[Z_AXIS].max_length){
			target[Z_AXIS] = axes[Z_AXIS].max_length;
		}

		planNextMove(target, ms/max_delta, target-position);
		position = target;
	}

	// Buffer the move. IOW, add a new block, and recalculate the acceleration accordingly
	void addMoveToBuffer(const Point& target, const int32_t &us_per_step)
	{
		Point offset_target = target + *tool_offsets;
		
		/// Clip Z axis so that plate cannot attempt to move out of build area
		/// other axis clipping will be added in a future revision
		if(offset_target[Z_AXIS] > axes[Z_AXIS].max_length){
			offset_target[Z_AXIS] = axes[Z_AXIS].max_length;
		}

			
		planNextMove(offset_target, us_per_step, offset_target - position);
		position = target;
	}


	///
	bool planNextMove(Point& target, const int32_t us_per_step_in, const Point& steps)
	{
		Block *block = block_buffer.getHead();
		// Mark block as not busy (Not executed by the stepper interrupt)
		block->flags = 0;
		
		block->target = target;
		
		uint32_t us_per_step = us_per_step_in;

		float delta_mm[STEPPER_COUNT];
		float local_millimeters = 0.0;
		uint32_t local_step_event_count = 0;

		// intentionally unraveled loop
		int32_t abs_steps = abs(steps[X_AXIS]);
		local_step_event_count = abs_steps;
		delta_mm[X_AXIS] = ((float)steps[X_AXIS])/axes[X_AXIS].steps_per_mm;

		abs_steps = abs(steps[Y_AXIS]);
		local_step_event_count = max((int32_t)local_step_event_count, abs_steps);
		delta_mm[Y_AXIS] = ((float)steps[Y_AXIS])/axes[Y_AXIS].steps_per_mm;

		abs_steps = abs(steps[Z_AXIS]);
		local_step_event_count = max((int32_t)local_step_event_count, abs_steps);
		delta_mm[Z_AXIS] = ((float)steps[Z_AXIS])/axes[Z_AXIS].steps_per_mm;
		
		

		//local_step_event_cout is the max of x,y,z speeds)
		if (local_step_event_count > 0) {
			local_millimeters = sqrt(delta_mm[X_AXIS]*delta_mm[X_AXIS] + delta_mm[Y_AXIS]*delta_mm[Y_AXIS] + delta_mm[Z_AXIS]*delta_mm[Z_AXIS]);
		}

		abs_steps = abs(steps[A_AXIS]);
		local_step_event_count = max((int32_t)local_step_event_count, abs_steps);
		delta_mm[A_AXIS] = ((float)steps[A_AXIS])/axes[A_AXIS].steps_per_mm;

		if (local_millimeters == 0.0) {
			local_millimeters = abs(delta_mm[A_AXIS]);
		}

#if STEPPER_COUNT > 4
		abs_steps = abs(steps[B_AXIS]);
		local_step_event_count = max((int32_t)local_step_event_count, abs_steps);
		delta_mm[B_AXIS] = ((float)steps[B_AXIS])/axes[B_AXIS].steps_per_mm;

		if (local_millimeters == 0.0) {
			local_millimeters = abs(delta_mm[B_AXIS]);
		}
#endif

		//local_step_event_cout is the max of x,y,z,a,b speeds)
		if (local_step_event_count == 0)
			return false;

		if(!accelerationON){
			block->target = target;
			block->nominal_rate = 1000000/us_per_step;
			block->accelerate_until = 0;
			block->step_event_count = local_step_event_count;
			block->decelerate_after = local_step_event_count;
			block->acceleration_rate = 0;
			block_buffer.bumpHead();
			steppers::startRunning();
			return true; //acceleration was not on, just move value into queue and run it
		}

		// if printing from RepG, ensure moves take a minimum amount of time so that the serial transfer can keep up
		if(!sdcard::isPlaying() ){
			if ((us_per_step * local_step_event_count) < MIN_MS_PER_SEGMENT) {
				us_per_step = MIN_MS_PER_SEGMENT / local_step_event_count;
			}
		}
		
		//TODO : test buffer empty slowing
		
	/*	uint8_t moves_queued = block_buffer.getUsedCount();
		if ((moves_queued  > 1) && (moves_queued < (BLOCK_BUFFER_SIZE >> 1))){
			int32_t segment_time = us_per_step * local_step_event_count;
			//if ((us_per_step * local_step_event_count) < MIN_MS_PER_SEGMENT) {
			if (segment_time < MIN_MS_PER_SEGMENT){
				segment_time+= 2*(MIN_MS_PER_SEGMENT - segment_time) / moves_queued;
				us_per_step = segment_time / local_step_event_count;
			}
		}
		*/


		float inverse_millimeters = 1.0/local_millimeters; // Inverse millimeters to remove multiple divides
		// Calculate 1 second/(seconds for this movement)
		float inverse_second = 1000000.0/(float)(us_per_step * local_step_event_count);
		float steps_per_mm = (float)local_step_event_count * inverse_millimeters;

		// we are given microseconds/step, and we need steps/mm, and steps/second

		// Calculate speed in steps/sec
		uint32_t steps_per_second = 1000000/us_per_step;
		float mm_per_second = local_millimeters * inverse_second;

		// Calculate speed in mm/second for each axis. No divide by zero due to previous checks.
		float local_nominal_speed = mm_per_second; // (mm/sec) Always > 0
		block->nominal_rate = steps_per_second; // (step/sec) Always > 0

		float current_speed[STEPPER_COUNT];
		for(int i=0; i < STEPPER_COUNT; i++) {
			current_speed[i] = delta_mm[i] * inverse_second;
		}

		// Compute and limit the acceleration rate for the trapezoid generator.
		uint32_t local_acceleration_st = ceil(default_acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
		// Limit acceleration per axis
		for(int i=0; i < STEPPER_COUNT; i++) {
			// warning: arithmetic overflow is easy here. Try to mitigate.
			float step_scale = (float)abs(steps[i]) / (float)local_step_event_count;
			float axis_acceleration_st = (float)local_acceleration_st * step_scale;
			if((uint32_t)axis_acceleration_st > axes[i].max_acceleration)
				local_acceleration_st = axes[i].max_acceleration;
		}
		block->acceleration = local_acceleration_st / steps_per_mm;
		block->acceleration_rate = local_acceleration_st / ACCELERATION_TICKS_PER_SECOND;

		
		// Compute the speed trasitions, or "jerks"
		// The default value the junction speed is the minimum_planner_speed (or local_nominal_speed if it is less than the minimum_planner_speed)
		float vmax_junction = min(minimum_planner_speed, local_nominal_speed); 
		
		if ((!block_buffer.isEmpty()) && (previous_nominal_speed > 0.0)) {
			   float jerk = sqrt(pow((current_speed[X_AXIS]-previous_speed[X_AXIS]), 2)+pow((current_speed[Y_AXIS]-previous_speed[Y_AXIS]), 2));
			   if((previous_speed[X_AXIS] != 0.0) || (previous_speed[Y_AXIS] != 0.0)) {
					   vmax_junction = local_nominal_speed;
			   }

			   if (jerk > max_xy_jerk) {
					   vmax_junction *= (max_xy_jerk/jerk);
				   }
			   
			   for (int i_axis = Z_AXIS; i_axis < STEPPER_COUNT; i_axis++) {
					   jerk = abs(previous_speed[i_axis] - current_speed[i_axis]);
					   if (jerk > axes[i_axis].max_axis_jerk) {
							   vmax_junction *= (axes[i_axis].max_axis_jerk/jerk);                               }
			   }
         } 

		
		/// set the max_entry_speed to the junction speed
		block->max_entry_speed = vmax_junction;
		
		// Initialize block entry speed. Compute based on deceleration to stop_speed.
		/// the entry speed may change in the look ahead planner
		float v_allowable = max_allowable_speed(-block->acceleration, minimum_planner_speed, local_millimeters);// stop_speed, local_millimeters);
		block->entry_speed = min(vmax_junction, v_allowable);
	

		// Initialize planner efficiency flags
		// Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
		// If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
		// the current block and next block junction speeds are guaranteed to always be at their maximum
		// junction speeds in deceleration and acceleration, respectively. This is due to how the current
		// block nominal speed limits both the current and next maximum junction speeds. Hence, in both
		// the reverse and forward planners, the corresponding block junction speed will always be at the
		// the maximum junction speed and may always be ignored for any speed reduction checks.
		if (local_nominal_speed <= v_allowable)
			block->flags |= Block::NominalLength;
		else
			block->flags &= ~Block::NominalLength;
		block->flags |= Block::Recalculate; // Always calculate trapezoid for new block

		// Update previous path speed and nominal speed
		memcpy(previous_speed, current_speed, sizeof(previous_speed)); // previous_speed[] = current_speed[]
		previous_nominal_speed = local_nominal_speed;

		// move locals to the block
		block->millimeters = local_millimeters;
		block->step_event_count = local_step_event_count;
		block->nominal_speed = local_nominal_speed;
		block->acceleration_st = local_acceleration_st;
		block->stop_speed = minimum_planner_speed;

		// Move buffer head
		block_buffer.bumpHead();

		planner_recalculate();

		steppers::startRunning();

		return true;
	}
	
	inline void loadToleranceOffsets(){
		// get toolhead offsets
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(int i = 0; i  < 3; i++){
				int32_t tolerance_err = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + i*4, 0)) / 10;
				tolerance_offset_T0[i] = (tolerance_err/2);
			}
			// For now, force Z offset to be zero as bad things can happen if it has a value AND there is no use case for it having a value on the replicator
			// extruder axes are 0 because offset concept does not apply
			for (int i = 2; i < STEPPER_COUNT; i++)
				tolerance_offset_T0[i] = 0;

			for(int i = 0; i < STEPPER_COUNT; i++)
				tolerance_offset_T1[i] = -1 * tolerance_offset_T0[i];
		}
	}

	/// call when a move is starting or ending, or canceling a move
	/// clears planner buffer
	void abort() {
		steppers::abort();
		position = steppers::getPosition();

		/// load toolhead offset values from EEPROM
		loadToleranceOffsets();
		/// tool 0 is default
		changeToolIndex(0);
		
		// reset speed
		for (int i = 0; i < STEPPER_COUNT; i++) {
			previous_speed[i] = 0.0;
		}
		previous_nominal_speed = 0.0;
		
		block_buffer.clear();

		accelerationON = eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS, 1);

		additional_ms_per_segment = 0;
		force_replan_from_stopped = false;

#ifdef CENTREPEDAL
		previous_unit_vec[0]= 0.0;
		previous_unit_vec[1]= 0.0;
		previous_unit_vec[2]= 0.0;
#endif
	}
	
	void changeToolIndex(uint8_t tool){
		if(tool == 1)
			tool_offsets = &tolerance_offset_T1;
		else
			tool_offsets = &tolerance_offset_T0;
	}

	void definePosition(const Point& new_position)
	{
		position = new_position;
		steppers::definePosition(new_position);
		
		// reset speed
		for (int i = 0; i < STEPPER_COUNT; i++) {
			previous_speed[i] = 0.0;
		}
		previous_nominal_speed = 0.0;
		
#ifdef CENTREPEDAL
		previous_unit_vec[0]= 0.0;
		previous_unit_vec[1]= 0.0;
		previous_unit_vec[2]= 0.0;
#endif
	}

	const Point getPosition()
	{
		return position;
	}
}
