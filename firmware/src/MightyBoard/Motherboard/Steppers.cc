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
 *
 * Modifications for Jetty Marlin compatability, authored by Dan Newman and Jetty.
 */

#ifndef SIMULATOR

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "StepperAxis.hh"
#include <stdint.h>
#include <util/delay.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "stdio.h"

#else

#include "Steppers.hh"
#include "StepperAxis.hh"
#include <stdint.h>
#include "Eeprom.hh"
#include "EepromMap.hh"

#ifdef ATOMIC_BLOCK
#undef ATOMIC_BLOCK
#endif
#define ATOMIC_BLOCK(x)

#ifdef labs
#undef labs
#endif
#define labs(x) abs(x)

#define st_init()
#define st_interrupt() false
#define st_extruder_interrupt()
#define quickStop()
#define DEBUG_TIMER_TCTIMER_USI 0
#define DEBUG_TIMER_START
#define DEBUG_TIMER_FINISH

#endif

#ifdef DEBUG_ONSCREEN
	volatile float debug_onscreen1 = 0.0, debug_onscreen2 = 0.0;
#endif

namespace steppers {

#ifndef SIMULATOR

/// Set up the digipot pins 
DigiPots digi_pots[STEPPER_COUNT] = {
        DigiPots(X_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
        DigiPots(Y_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
        DigiPots(Z_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
        DigiPots(A_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
        DigiPots(B_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
};

#else

typedef struct {
	void (*init)(int);
	void (*setPotValue)(uint8_t);
} DigiPots;

static void dummy_init(int i) { (void)i; }
static void dummy_setPotValue(uint8_t v) { (void)v; }

DigiPots digi_pots[STEPPER_COUNT] = {
	{dummy_init, dummy_setPotValue},
	{dummy_init, dummy_setPotValue},
	{dummy_init, dummy_setPotValue},
	{dummy_init, dummy_setPotValue},
	{dummy_init, dummy_setPotValue}
};

#endif

void initPots(){
	// set digi pots to stored default values
	for ( int i = 0; i < STEPPER_COUNT; i++ ) {
		digi_pots[i].init(i);
	}
}

volatile bool is_running;
volatile bool is_homing;
bool acceleration = true;
uint8_t plannerMaxBufferSize;
FPTYPE axis_steps_per_unit_inverse[STEPPER_COUNT];

// Segments are accelerated when segmentAccelState is true; unaccelerated otherwise
static bool segmentAccelState = true;

bool holdZ = false;

Point tolerance_offset_T0;
Point tolerance_offset_T1;
Point *tool_offsets;
uint8_t toolIndex = 0;

//Also requires DEBUG_ONSCREEN to be defined in StepperAccel.h
//#define TIME_STEPPER_INTERRUPT

#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
uint16_t debugTimer = 0;
#endif


bool isRunning() {
	return is_running || is_homing;
}


void loadToleranceOffsets() {
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


void reset() {
	stepperAxisInit(false);

	initPots();

	loadToleranceOffsets();
	changeToolIndex(0);

	// If acceleration has not been initialized before (i.e. last time we ran we were an earlier firmware),
	// then we initialize the acceleration eeprom settings here
	uint8_t accelerationStatus = eeprom::getEeprom8(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::DEFAULTS_FLAG, 0xFF);
	if (accelerationStatus !=  _BV(ACCELERATION_INIT_BIT)) {
		eeprom::setDefaultsAcceleration();
	}

        //Get the acceleration settings
	uint8_t accel = eeprom::getEeprom8(eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::ACTIVE_OFFSET, 0) & 0x01;
        acceleration = accel & 0x01;

	setSegmentAccelState(acceleration);
	deprimeEnable(true);

	//Here's more documentation on the various settings / features
	//http://wiki.ultimaker.com/Marlin_firmware_for_the_Ultimaker
	//https://github.com/ErikZalm/Marlin/commits/Marlin_v1
	//http://forums.reprap.org/read.php?147,94689,94689
	//http://reprap.org/pipermail/reprap-dev/2011-May/003323.html
	//http://www.brokentoaster.com/blog/?p=358

	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		axis_steps_per_unit_inverse[i] = FTOFP(1.0 / stepperAxisStepsPerMM(i));

	//Macros to clean things up a bit
	#define NAC2(LOCATION) eeprom_offsets::ACCELERATION2_SETTINGS + acceleration_eeprom_offsets::LOCATION
	#define AC2(LOCATION,INT16INDEX) NAC2(LOCATION) + sizeof(uint16_t) * INT16INDEX

	// Set max acceleration in units/s^2 for print moves
	// X,Y,Z,A,B maximum start speed for accelerated moves.
	// A,B default values are good for skeinforge 40+, for older versions raise them a lot.
	max_acceleration_units_per_sq_second[X_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,0), DEFAULT_MAX_ACCELERATION_AXIS_X);
	max_acceleration_units_per_sq_second[Y_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,1), DEFAULT_MAX_ACCELERATION_AXIS_Y);
	max_acceleration_units_per_sq_second[Z_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,2), DEFAULT_MAX_ACCELERATION_AXIS_Z);
	max_acceleration_units_per_sq_second[A_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,3), DEFAULT_MAX_ACCELERATION_AXIS_A);
	max_acceleration_units_per_sq_second[B_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,4), DEFAULT_MAX_ACCELERATION_AXIS_B);

	for (uint8_t i = 0; i < STEPPER_COUNT; i ++) {
		// Limit the max accelerations so that the calculation of block->acceleration & JKN Advance K2
		// can be performed without overflow issues
		if (max_acceleration_units_per_sq_second[i] > (uint32_t)((float)0xFFFFF / stepperAxisStepsPerMM(i)))
		     max_acceleration_units_per_sq_second[i] = (uint32_t)((float)0xFFFFF / stepperAxisStepsPerMM(i));
		axis_steps_per_sqr_second[i] = (uint32_t)((float)max_acceleration_units_per_sq_second[i] * stepperAxisStepsPerMM(i));
		axis_accel_step_cutoff[i] = (uint32_t)0xffffffff / axis_steps_per_sqr_second[i];
	}

	//Set default acceleration for "Normal Moves (acceleration)" and "filament only moves (retraction)" in mm/sec^2

	// X,Y,Z,A,B max acceleration in mm/s^2 for printing moves
	p_acceleration = (uint32_t)eeprom::getEeprom16(NAC2(MAX_ACCELERATION_NORMAL_MOVE), DEFAULT_MAX_ACCELERATION_NORMAL_MOVE);
	if (p_acceleration > 10000)
	     // 10,000 limit is actually a little smaller than 0xFFFFF / 96 steps/mm
	     p_acceleration = 10000;

#ifdef DEBUG_SLOW_MOTION
	p_acceleration = (uint32_t)20;
#endif

	// X,Y,Z,A,B max acceleration in mm/s^2 for retracts
	p_retract_acceleration  = (uint32_t)eeprom::getEeprom16(NAC2(MAX_ACCELERATION_EXTRUDER_MOVE), DEFAULT_MAX_ACCELERATION_EXTRUDER_MOVE);
	if (p_retract_acceleration > 10000)
	     // 10,000 limit is actually a little smaller than 0xFFFFF / 96 steps/mm
	     p_retract_acceleration = 10000;

#ifdef DEBUG_SLOW_MOTION
	p_retract_acceleration	= (uint32_t)20;
#endif

	//Number of steps when priming or deprime the extruder
	extruder_deprime_steps[0]    = (int16_t)eeprom::getEeprom16(AC2(EXTRUDER_DEPRIME_STEPS,0), DEFAULT_EXTRUDER_DEPRIME_STEPS_A);
	extruder_deprime_steps[1]    = (int16_t)eeprom::getEeprom16(AC2(EXTRUDER_DEPRIME_STEPS,1), DEFAULT_EXTRUDER_DEPRIME_STEPS_B);

	//Maximum speed change
	max_speed_change[X_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,0), DEFAULT_MAX_SPEED_CHANGE_X));
	max_speed_change[Y_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,1), DEFAULT_MAX_SPEED_CHANGE_Y));
	max_speed_change[Z_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,2), DEFAULT_MAX_SPEED_CHANGE_Z));
	max_speed_change[A_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,3), DEFAULT_MAX_SPEED_CHANGE_A));
	max_speed_change[B_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,4), DEFAULT_MAX_SPEED_CHANGE_B));

#ifdef DEBUG_SLOW_MOTION
	max_speed_change[X_AXIS]  = FTOFP((float)1);
	max_speed_change[Y_AXIS]  = FTOFP((float)1);
	max_speed_change[Z_AXIS]  = FTOFP((float)0.15);
	max_speed_change[A_AXIS]  = FTOFP((float)1);
	max_speed_change[B_AXIS]  = FTOFP((float)1);
#endif

#ifdef FIXED
	smallest_max_speed_change = max_speed_change[Z_AXIS];
	for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
		if ( max_speed_change[i] < smallest_max_speed_change )
			smallest_max_speed_change = max_speed_change[i];
	}
#endif

	FPTYPE advanceK         = FTOFP((float)eeprom::getEeprom32(NAC2(JKN_ADVANCE_K),  DEFAULT_JKN_ADVANCE_K)         / 100000.0);
	FPTYPE advanceK2        = FTOFP((float)eeprom::getEeprom32(NAC2(JKN_ADVANCE_K2), DEFAULT_JKN_ADVANCE_K2)        / 100000.0);

	minimumSegmentTime = FTOFP((float)ACCELERATION_MIN_SEGMENT_TIME);

	// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
	// of the buffer and all stops. This should not be much greater than zero and should only be changed
	// if unwanted behavior is observed on a user's machine when running at very slow speeds.
	minimumPlannerSpeed = FTOFP((float)ACCELERATION_MIN_PLANNER_SPEED);

	if ( eeprom::getEeprom8(NAC2(SLOWDOWN_FLAG), DEFAULT_SLOWDOWN_FLAG) ) {
		slowdown_limit = (int)ACCELERATION_SLOWDOWN_LIMIT;
		if ( slowdown_limit > (BLOCK_BUFFER_SIZE / 2))  slowdown_limit = 0;
	}
	else	slowdown_limit = 0;	

	//Clockwise extruder
	extrude_when_negative[0] = ACCELERATION_EXTRUDE_WHEN_NEGATIVE_A;
	extrude_when_negative[1] = ACCELERATION_EXTRUDE_WHEN_NEGATIVE_B;

	//These max feedrates limit the speed the extruder can move at when
	//it's been advanced, primed/deprimed and depressurized
	//It acts as an overall speed governer for the A/B axis
	//The values are obtained via the RepG xml and are updated on connection
	//with RepG if they're different than stored.  These values are in mm per
	//min, we divide by 60 here to get mm / sec.
	extruder_only_max_feedrate[0] = stepperAxis[A_AXIS].max_feedrate;
	extruder_only_max_feedrate[1] = stepperAxis[B_AXIS].max_feedrate;

#ifdef PLANNER_OFF
	plannerMaxBufferSize = 1;
#else
	plannerMaxBufferSize = BLOCK_BUFFER_SIZE - 1;
#endif

	plan_init(advanceK, advanceK2, holdZ);		//Initialize planner
	st_init();					//Initialize stepper accel
}

//public:
void init() {
	is_running = false;
	is_homing = false;

	stepperAxisInit(true);

	initPots();
	
        /// if eeprom has not been initialized. store default values
	if (eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS, 0xFFFFFFFF) == 0xFFFFFFFF) {
		eeprom::storeToolheadToleranceDefaults();
	}
}


void abort() {
	//Stop the stepper subsystem and get the current position
	//after stopping
	quickStop();

        is_running = false;
        is_homing = false;
	
	stepperAxisInit(false);

	setSegmentAccelState(acceleration);
	deprimeEnable(true);
}

/// Define current position as given point
void definePosition(const Point& position_in) {
	Point position_offset = position_in;

	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		stepperAxis[i].hasDefinePosition = true;

		//Add the toolhead offset
		position_offset[i] += (*tool_offsets)[i];
	}

	plan_set_position(position_offset[X_AXIS], position_offset[Y_AXIS], position_offset[Z_AXIS], position_offset[A_AXIS], position_offset[B_AXIS]);
}


/// Get the last position of the planner
/// This is also the target position of the last command that was sent with
/// setTarget / setTargetNew / setTargetNewExt
/// Note this isn't the position of the hardware right now, use getStepperPosition for that.
/// If the pipeline buffer is empty, then getPlannerPosition == getStepperPosition
const Point getPlannerPosition() {
	Point p;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		p = Point(planner_position[X_AXIS], planner_position[Y_AXIS], planner_position[Z_AXIS],
			  planner_position[A_AXIS], planner_position[B_AXIS] );

		//Subtract out the toolhead offset
		for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
			p[i] -= (*tool_offsets)[i];
	}
	return p;
}


/// Get current position

#ifndef SIMULATOR

const Point getStepperPosition() {
	uint8_t active_toolhead;
	int32_t position[STEPPER_COUNT];

	st_get_position(&position[X_AXIS], &position[Y_AXIS], &position[Z_AXIS], &position[A_AXIS], &position[B_AXIS], &active_toolhead);

	active_toolhead %= 2;	//Safeguard, shouldn't be needed

	//Because all targets have a toolhead offset added to them, we need to undo that here.
	//Also, because the toollhead can change, we need to use the active_toolhead from the hardware position
	//and can't use toolIndex
	Point *gp_tool_offsets;

	if ( active_toolhead == 1 )	gp_tool_offsets = &tolerance_offset_T1;
	else				gp_tool_offsets = &tolerance_offset_T0;

	//Subtract out the toolhead offset
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		position[i] -= (*gp_tool_offsets)[i];

	Point p = Point(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[A_AXIS], position[B_AXIS]);
	return p;
}

#else

const Point getStepperPosition() {
	Point p = Point(0,0,0,0,0);
	return p;
}

#endif


void setHoldZ(bool holdZ_in) {
	holdZ = holdZ_in;
}


void setTarget(const Point& target, int32_t dda_interval) {
	//Add on the tool offsets
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		planner_target[i] = target[i] + (*tool_offsets)[i];

#ifdef CLIP_Z_AXIS
	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);
#endif

	//Calculate the maximum steps of any axis and store in planner_master_steps
	//Also calculate the step deltas (planner_steps[i]) at the same time.
	int32_t max_delta = 0;
	planner_master_steps_index = 0;
	for (int i = 0; i < STEPPER_COUNT; i++) {
		planner_steps[i] = labs(planner_target[i] - planner_position[i]);

		if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
			max_delta = planner_steps[i];
		}
	}
	planner_master_steps = (uint32_t)max_delta;

	if ( planner_master_steps == 0 ) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	//dda_rate is the number of dda steps per second for the master axis
	uint32_t dda_rate = (uint32_t)(1000000 / dda_interval);

	plan_buffer_line(0, dda_rate, toolIndex, false, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize) is_running = true;
	else                                          is_running = false;
}


void setTargetNew(const Point& target, int32_t us, uint8_t relative) {
	//Add on the tool offsets and convert relative moves into absolute moves
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		planner_target[i] = target[i] + (*tool_offsets)[i];

		if ((relative & (1 << i)) != 0) {
			planner_target[i] = planner_position[i] + planner_target[i];
		}
	}

#ifdef CLIP_Z_AXIS
	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);
#endif

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
        int32_t max_delta = 0;
	planner_master_steps_index = 0;
        for (int i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = labs(planner_target[i] - planner_position[i]);

                if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
                        max_delta = planner_steps[i];
		}
        }
        planner_master_steps = (uint32_t)max_delta;

	if ( planner_master_steps == 0 ) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	int32_t  dda_interval	= us / max_delta;

	//dda_rate is the number of dda steps per second for the master axis
	uint32_t dda_rate	= (uint32_t)(1000000 / dda_interval);

	plan_buffer_line(0, dda_rate, toolIndex, false, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Dda_rate is the number of dda steps per second for the master axis

void setTargetNewExt(const Point& target, int32_t dda_rate, uint8_t relative, float distance, int16_t feedrateMult64) {
	//Add on the tool offsets and convert relative moves into absolute moves
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		planner_target[i] = target[i] + (*tool_offsets)[i];

		if ((relative & (1 << i)) != 0) {
			planner_target[i] = planner_position[i] + planner_target[i];
		}
	}

#ifdef CLIP_Z_AXIS
	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);
#endif

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
        int32_t max_delta = 0;
        planner_master_steps_index = 0;
        for (int i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = planner_target[i] - planner_position[i];
		int32_t abs_planner_steps = labs(planner_steps[i]);
		if (abs_planner_steps <= 0x7fff)
		     delta_mm[i] = FPMULT2(ITOFP(planner_steps[i]), axis_steps_per_unit_inverse[i]);
		else
		      // This typically only happens for LONG Z axis moves
		      // As such it typically happens three times per print
		     delta_mm[i] = FTOFP((float)planner_steps[i] * FPTOF(axis_steps_per_unit_inverse[i]));
                planner_steps[i] = abs_planner_steps;

                if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
                        max_delta = planner_steps[i];
		}
        }
        planner_master_steps = (uint32_t)max_delta;

	if (( planner_master_steps == 0 ) || ( distance == 0.0 )) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	//Handle distance
	planner_distance = FTOFP(distance);

	//Handle feedrate
	FPTYPE feedrate = 0;

	if ( acceleration ) {
		feedrate = ITOFP((int32_t)feedrateMult64);

		//Feed rate was multiplied by 64 before it was sent, undo
#ifdef FIXED
			feedrate >>= 6;
#else
			feedrate /= 64.0;
#endif
	}

	plan_buffer_line(feedrate, dda_rate, toolIndex, acceleration && segmentAccelState, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Step positions for homing.  We shift by >> 1 so that we can add
//tool_offsets without overflow
#define POSITIVE_HOME_POSITION ((INT32_MAX - 1) >> 1)
#define NEGATIVE_HOME_POSITION ((INT32_MIN + 1) >> 1)

/// Start homing

void startHoming(const bool maximums, const uint8_t axes_enabled, const uint32_t us_per_step) {
	setSegmentAccelState(false);

	Point target = getStepperPosition();

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
                if ((axes_enabled & (1<<i)) == 0) {
			axis_homing[i] = false;
		} else {
	 		target[i] = (maximums) ? POSITIVE_HOME_POSITION : NEGATIVE_HOME_POSITION;
			axis_homing[i] = true;
			stepperAxis[i].hasHomed = true;
                }
        }

	setTarget(target, us_per_step);

        is_homing = true;
}


/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
        if (index < STEPPER_COUNT) {
		stepperAxisSetEnabled(index, enable);
        }
}


/// Returns a bit mask for all axes enabled
uint8_t allAxesEnabled(void) {
	return axesEnabled;
}


/// set digital potentiometer for stepper axis
void setAxisPotValue(uint8_t index, uint8_t value){
	if (index < STEPPER_COUNT) {
		digi_pots[index].setPotValue(value);
	}
}


/// get the digital potentiometer for stepper axis
uint8_t getAxisPotValue(uint8_t index){
#ifndef SIMULATOR
	if (index < STEPPER_COUNT) {
		return digi_pots[index].getPotValue();
	}
#endif
	return 0;
}

/// Reset the digital potentiometer for stepper axis to the stored eeprom value
void resetAxisPot(uint8_t index) {
#ifndef SIMULATOR
	if (index < STEPPER_COUNT) {
		digi_pots[index].resetPots();
	}
#endif
}

/// Toggle segment acceleration on or off
/// Note this is also off if acceleration variable is not set
void setSegmentAccelState(bool state) {
     segmentAccelState = state;
}


void changeToolIndex(uint8_t tool) {
	toolIndex = tool;

	//Just in case as toolIndex is used to index into arrays
	//so we need a sane value here.
	toolIndex %= 2;

	if(toolIndex == 1)	tool_offsets = &tolerance_offset_T1;
	else			tool_offsets = &tolerance_offset_T0;
}


// endstop status bits: (7-0) : | N/A | N/A | z max | z min | y max | y min | x max | x min |

uint8_t getEndstopStatus() {
	uint8_t status = 0;

	status |= stepperAxisIsAtMaximum(Z_AXIS) << 5;
	status |= stepperAxisIsAtMinimum(Z_AXIS) << 4;
	status |= stepperAxisIsAtMaximum(Y_AXIS) << 3;
	status |= stepperAxisIsAtMinimum(Y_AXIS) << 2;
	status |= stepperAxisIsAtMaximum(X_AXIS) << 1;
	status |= stepperAxisIsAtMinimum(X_AXIS) << 0;

	return status;
}


// Enables and disables deprime
// Deprime is always disabled if acceleration is switch off in the eeprom

void deprimeEnable(bool enable) {
	st_deprime_enable(acceleration && enable);
}


void runSteppersSlice() {
#if 0
#ifdef DEBUG_VALUE
	uint8_t bufferUsed = movesplanned();

	//Set bit 1 of the debug leds to the amount of items left in the acceleration pipeline
	//2 or less items left, and we switch foo off, otherwise on
	if ( bufferUsed < 3 ) DEBUG_VALUE(0x00);
	else                  DEBUG_VALUE(0x01);
#endif
#endif

        //Set the stepper interrupt timer
#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
        debug_onscreen2 = debugTimer;
#endif
}


void doStepperInterrupt() {
#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
                DEBUG_TIMER_START;
#endif

	//is_running is determined when a buffer item is added, however
	//if st_interrupt deletes a buffer item, then is_running must have changed
	//and now be false, so we set it here
	if ( st_interrupt() ) is_running = false;

	//If we're homing, there's a few possibilities:
	//1. The homing is still running on one of the axis
	//2. The homing on all axis has stopped running, in which case, the block
	//is dead, so we delete it and sync up the planner position to the stepper position
	//Homing blocks aren't automatically deleted by st_interrupt because they are set to
	//positions of INT32_MAX/MIN.
	if ( is_homing ) {
		is_homing = false;
	
		//Are we still homing on one of the axis?
		for (uint8_t i = 0; i <= Z_AXIS; i++)
			is_homing |= axis_homing[i];

		//If we've finished homing, stop the stepper subsystem
		//and sync
		if ( ! is_homing ) {
			//Delete all blocks (should only be 1 homing block) and sync
			//planner position to stepper position
			quickStop();

			setSegmentAccelState(acceleration);
		}
	}

#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
        DEBUG_TIMER_FINISH;
        debugTimer = DEBUG_TIMER_TCTIMER_USI;
#endif
}


void doExtruderInterrupt() {
	st_extruder_interrupt();
}

}
