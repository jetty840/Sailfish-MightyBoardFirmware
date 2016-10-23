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

#include "Compat.hh"
#include "Model.hh"
#include "MachineId.hh"

#ifndef SIMULATOR

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "StepperAxis.hh"
#include <stdint.h>
#include <util/delay.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "StepperAccelPlanner.hh"
#include "stdio.h"

#else

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "StepperAxis.hh"
#include <stdint.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "StepperAccelPlanner.hh"

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

#if defined(AUTO_LEVEL)
#include "SkewTilt.hh"
#endif

#ifdef DEBUG_ONSCREEN
	volatile float debug_onscreen1 = 0.0, debug_onscreen2 = 0.0;
#endif

#ifdef TOOL_CHANGE_SEPARATE_MOVE
float tolerance_offset_distance;
int16_t tolerance_feedrate_x64;
#endif

namespace steppers {

uint8_t alterSpeed = 0x00;
FPTYPE speedFactor = KCONSTANT_1;

#if defined(DIGIPOT_SUPPORT)

void initPots() {
     // set digipots to stored default values
     DigiPots::init();
     for ( int i = 0; i < STEPPER_COUNT; i++ )
	  DigiPots::resetPot(i);
}

#define INITPOTS  initPots()

#else

#define INITPOTS

#endif

volatile bool is_running;
volatile bool is_homing;
bool acceleration = true;
uint8_t plannerMaxBufferSize;
FPTYPE axis_steps_per_unit_inverse[STEPPER_COUNT];

// Some gcode is loaded with enable/disable extruder commands. E.g., before each travel-only move.
// This seems okay for 1.75 mm filament extruders.  However, it is problematic for 3mm filament
// extruders: when the stepper motor is disabled, too much filament backs out owing to the high
// melt chamber pressure and the free-wheeling pinch gear.  To combat this, the firmware has an
// option to leave the extruder stepper motors engaged throughout an entire build, ignoring any
// gcode / s3g command to disable the extruder stepper motors.
bool extruder_hold[EXTRUDERS]; // True if the extruders should not be disabled during printing

// Segments are accelerated when segmentAccelState is true; unaccelerated otherwise
static bool segmentAccelState = true;

static Point tolerance_offset_T0;
static Point tolerance_offset_T1;
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


// The following simple routine loads the toolhead offsets.  The systems
// used for the offsets have changed over time much to the confusion and
// consternation of users.  And, to compound the problem it appears that
// an auto-conversion of the firmware's 5.x to 6.x format was done
// incorrectly.
//
// Changes such as this unfortunately make it difficult for third-party
// tools (e.g., slicers) to support MBI's hardware: who wants to keep on
// chasing a moving target?
//
// Brief history of the toolhead offsets.
//
// RepG 29 (11 December 2011; MBI 5.x firmwares)
// ---------------------------------------------
//
//   1. Each toolhead moved half the ideal offset in gcode
//
//      G10 P1 X-16.5  ( X coordinates += -16.5 mm after a G54 command issued )
//      G10 P2 X+16.5  ( X coordinates += +16.5 mm after a G55 command issued )
//
//   2. No extra handling in the firmware
//
//   This was likely done to kick start experimentation of dualstrusion.
//   Folks using Thing-o-Matics with no modifications to the 3.1 firmware.
//
// RepG 33, 34 (27 February, 13 March 2012; MBI 5.x firmwares)
// -----------------------------------------------------------
//
//   1. Each tool head moved half the ideal offset in gcode
//
//      G10 P1 X-16.5  ( X coordinates += -16.5 mm after a G54 command issued )
//      G10 P2 X+16.5  ( X coordinates += +16.5 mm after a G55 command issued )
//
//   2. In firmware, deviations from ideal are stored in EEPROM
//
//   3. In firmware, deviations split between each toolhead
//
//      Tool 0 in use: X coordinates += + x-deviation / 2
//      Tool 1 in use: X coordinates += - x-deviation / 2
//
//   So, when moving to X = 0, the actual position is
//
//      Tool 0 in use:  -16.5 mm + x-deviation / 2
//      Tool 1 in use:  +16.5 mm - x-deviation / 2
//
//   Difference is then
//
//      total-X-offset = 33.0 mm - x-deviation
//
//   Consequently,
//
//      x-deviation > 0 ==> nozzles are closer together than the ideal 33.0 mm
//      x-deviation < 0 ==> nozzles are farther appart than the ideal 33.0 mm
//
// RepG 37, 39 (22 June, 3 October 2012; MBI 5.5 firmware)
// -------------------------------------------------------
//
//   1. Tool 1 moved the entire ideal offset in gcode
//
//      G10 P1 X0     ( X coordinates unchanged after a G54 command issued )
//      G10 P2 X33.0  ( X coordinates += 33.0 mm after a G55 command issued )
//
//   2. In firmware, deviations from ideal are stored in EEPROM
//
//   3. In firmware, deviations split between each toolhead
//
//      Tool 0 in use: X coordinates += + x-deviation / 2
//      Tool 1 in use: X coordinates += - x-deviation / 2
//
//   So, when moving to X = 0, the actual position is
//
//      Tool 0 in use:    0.0 mm + x-deviation / 2
//      Tool 1 in use:  +33.0 mm - x-deviation / 2
//
//   Difference is then
//
//      total-X-offset = 33.0 mm - x-deviation
//
//   Consequently,
//
//      x-deviation > 0 ==> nozzles are closer together than the ideal 33.0 mm
//      x-deviation < 0 ==> nozzles are farther appart than the ideal 33.0 mm
//
// RepG 40 (9 November 2012; MBI 6.0 firmware)
// -------------------------------------------
// *** MBI introduces an error in their 6.x firmware for handling of old
// *** EEPROM toolhead offsets.
//
//   1. Gcode no longer tracks toolhead offsets: the G10, G54, and G55
//      commands are no longer used.
//
//   2. In firmware, the total offsets are stored in EEPROM
//
//   3. In firmware, the total offset is put onto Tool 1
//
//      Tool 0 in use: X coordinates remain unchanged
//      Tool 1 in use: X coordinates += total-X-offset'
//
//   So, when moving to X = 0, the actual position is
//
//      Tool 0 in use:  0.0 mm
//      Tool 1 in use:  0.0 mm + total-X-offset'
//
//   The difference is the total-X-offset'.
//
//   Fine so far.  However, MBI included in the 6.x (and now 7.x) firmware
//   code to automatically convert the 5.x "x-deviation" stored in EEPROM
//   to a "total-X-offset'" stored in EEPROM.  They used this formula,
//
//      total-X-offset' = 33.0 mm + x-deviation
//
//   Unfortunately, that's wrong the wrong formula!  As per the analyses
//   above of what the x-deviation stored in EEPROM means,
//
//      x-deviation > 0 ==> nozzles are closer than 33.0 mm
//      x-deviation < 0 ==> nozzles are farther appart than 33.0 mm
//
//   Thus, the correct translation to a total-X-offset is
//
//      total-X-offset' = 33.0 mm - x-deviation
//
//   And that's exactly the form we arrived at in the prior analysis.
//   Additionally, this can be seen when doing dualstrusion prints with
//   MBI's 5.5 and then 6.2 firmware.  Dan happens to have nozzles with
//   significant offsets in both X and Y ( ~0.6 mm for both ).  A very
//   nicely aligned print done with 5.5 then comes out with visible
//   misalignment with 6.2 and its incorrect auto-conversion of the tool
//   head offsets.  The sign error in the conversion gives a result off
//   by twice the offset or ~1.2 mm.  That's visually significant.
//
//   Minor correction of the 6.2 firmware to compute the total-X-offset'
//   correctly resulted in correctly aligned prints.
//
// Sailfish therefore implements the correct conversion.  Also, unlike
// the MBI firmware, it doesn't change the offsets stored in EEPROM.  It
// just converts the offsets at run time as necessary.

void loadToleranceOffsets() {

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Force all toolhead offsets to 0
		// Note that when the bot's tool count is set to 1, RepG won't display the
		//   toolhead offsets.  It then becomes possible for a bot operator to not
		//   realize that s/he has non-zero toolhead offsets set.  RepG won't show
		//   them but they're there and effect behavior unless we force the offsets
		//   to zero when the tool count is != 2.

		for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
			tolerance_offset_T0[i] = 0;
			tolerance_offset_T1[i] = 0;
		}
	}

#ifndef SIMULATOR
#if EXTRUDERS > 1
	// get toolhead offsets for dual extruder units
	if ( !eeprom::isSingleTool() ) {

		// ~4 mm expressed in units of steps
		int32_t fourMM = ((int32_t)stepperAxisStepsPerMM(0)) << 2;

		// The X Toolhead offset in units of steps
		int32_t xToolheadOffset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 0, TOOLHEAD_OFFSET_X));
		int32_t yToolheadOffset = (int32_t)(eeprom::getEeprom32(eeprom_offsets::TOOLHEAD_OFFSET_SETTINGS + 4, TOOLHEAD_OFFSET_Y));

#ifdef TOOLHEAD_OFFSET_SYSTEM
		// See which toolhead offset system is used
		uint8_t toolhead_system =
			eeprom::getEeprom8(eeprom_offsets::TOOLHEAD_OFFSET_SYSTEM,
					   DEFAULT_TOOLHEAD_OFFSET_SYSTEM);
		if ( toolhead_system == 0 ) {

			// OLD SYSTEM: stored offset is the deviation from the
			//    ideal offset of 33.0 or 35.0 mm.

			// See if the stored offset is > 4 mm.  If so, then it's
			//    likely meant for the new system.  If so, convert it
			//    to the old system.

			if ( abs(xToolheadOffset) > fourMM ) {
				xToolheadOffset =
					(int32_t)stepperAxisMMToSteps(TOOLHEAD_OFFSET_X, 0) - xToolheadOffset;
				yToolheadOffset = -yToolheadOffset;
			}

			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				// In the RepG 39 and earlier system
				//
				//    T0[i] = +1/2 offset[i]
				//    T1[i] = -1/2 offset[i]
				//
				// MOREOVER, the offset is the deviation from the ideal offset
				tolerance_offset_T0[0] =  xToolheadOffset / 2;
				tolerance_offset_T1[0] = -tolerance_offset_T0[0];
				tolerance_offset_T0[1] =  yToolheadOffset / 2;
				tolerance_offset_T1[1] = -tolerance_offset_T0[1];
			}
		}
		else {
#endif // TOOLHEAD_OFFSET_SYSTEM
			// NEW SYSTEM: stored offset is the actual offset (33 or 35 mm)

			// See if the stored offset is < 4.0 mm.  If so, then it's
			//    likely meant for the old system.  If so, convert it
			//    to the new system.

			if ( abs(xToolheadOffset) <= fourMM ) {
				xToolheadOffset = TOOLHEAD_OFFSET_X - xToolheadOffset;
				yToolheadOffset = -yToolheadOffset;
			}

			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				// In the RepG 40 and later system
				//
				//    T0[i] = 0
				//    T1[i] = offset[i]
				//
				// The offset is the full offset, not the deviation from the offset
				tolerance_offset_T1[0] = xToolheadOffset;
				tolerance_offset_T1[1] = yToolheadOffset;
			}
#ifdef TOOLHEAD_OFFSET_SYSTEM
		}
#endif
	}
#endif // EXTRUDERS > 1
#ifdef TOOL_CHANGE_SEPARATE_MOVE
	tolerance_offset_distance = sqrt((float)(xToolheadOffset*xToolheadOffset +
											 yToolheadOffset*yToolheadOffset));
	tolerance_feedrate_x64 = (stepperAxis[X_AXIS].max_feedrate <=
							  stepperAxis[Y_AXIS].max_feedrate) ?
		stepperAxis[X_AXIS].max_feedrate : stepperAxis[Y_AXIS].max_feedrate;
	tolerance_feedrate_x64 = FPTOI16(FPMULT2(tolerance_feedrate_x64, KCONSTANT_64));
#endif // TOOL_CHANGE_SEPARATE_MOVE
#endif // !defined(SIMULATOR)
}

void reset() {
	stepperAxisInit(false);
	INITPOTS;

	// must be after stepperAxisInit() so that stepperAxisStepsPerMM() functions correctly
	// must be after stepperAxisInit() so that steppers[].max_feedrate has been set
	loadToleranceOffsets();

	// must be after loadToleranceOffsets() so that the toolhead offsets are at hand
	changeToolIndex(0);

	// If acceleration has not been initialized before (i.e. last time we ran we were an earlier firmware),
	// then we initialize the acceleration eeprom settings here
	uint8_t accelerationStatus = eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::DEFAULTS_FLAG, 0xFF);
	if (accelerationStatus !=  _BV(ACCELERATION_INIT_BIT)) {
		eeprom::setDefaultsAcceleration();
	}

        //Get the acceleration settings
	uint8_t accel = eeprom::getEeprom8(eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::ACCELERATION_ACTIVE, 0) & 0x01;
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
	#define NAC2(LOCATION) eeprom_offsets::ACCELERATION_SETTINGS + acceleration_eeprom_offsets::LOCATION
	#define NAC2_2(LOCATION) eeprom_offsets::ACCELERATION2_SETTINGS + acceleration2_eeprom_offsets::LOCATION
	#define AC2(LOCATION,INT16INDEX) NAC2(LOCATION) + sizeof(uint16_t) * INT16INDEX
	#define AC2_2(LOCATION,INT16INDEX) NAC2_2(LOCATION) + sizeof(uint16_t) * INT16INDEX

	// Set max acceleration in units/s^2 for print moves
	// X,Y,Z,A,B maximum start speed for accelerated moves.
	// A,B default values are good for skeinforge 40+, for older versions raise them a lot.
	max_acceleration_units_per_sq_second[X_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,0), DEFAULT_MAX_ACCELERATION_AXIS_X);
	max_acceleration_units_per_sq_second[Y_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,1), DEFAULT_MAX_ACCELERATION_AXIS_Y);
	max_acceleration_units_per_sq_second[Z_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,2), DEFAULT_MAX_ACCELERATION_AXIS_Z);
	max_acceleration_units_per_sq_second[A_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,3), DEFAULT_MAX_ACCELERATION_AXIS_A);
#if EXTRUDERS > 1
	max_acceleration_units_per_sq_second[B_AXIS] = (uint32_t)eeprom::getEeprom16(AC2(MAX_ACCELERATION_AXIS,4), DEFAULT_MAX_ACCELERATION_AXIS_B);
#endif

	for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
		// Limit the max accelerations so that the calculation of block->acceleration & JKN Advance K2
		// can be performed without overflow issues
		if (max_acceleration_units_per_sq_second[i] > (uint32_t)((float)0xFFFFF / stepperAxisStepsPerMM(i)))
		     max_acceleration_units_per_sq_second[i] = (uint32_t)((float)0xFFFFF / stepperAxisStepsPerMM(i));
		axis_steps_per_sqr_second[i] = (uint32_t)((float)max_acceleration_units_per_sq_second[i] * stepperAxisStepsPerMM(i));
		axis_accel_step_cutoff[i] = (uint32_t)0xffffffff / axis_steps_per_sqr_second[i];
	}

#ifdef OLD_ACCEL_LIMITS
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
#endif

	//Number of steps when priming or deprime the extruder
	extruder_deprime_steps[0]    = (int16_t)eeprom::getEeprom16(AC2_2(EXTRUDER_DEPRIME_STEPS,0), DEFAULT_EXTRUDER_DEPRIME_STEPS_A);
#if EXTRUDERS > 1
	extruder_deprime_steps[1]    = (int16_t)eeprom::getEeprom16(AC2_2(EXTRUDER_DEPRIME_STEPS,1), DEFAULT_EXTRUDER_DEPRIME_STEPS_B);
#endif
	extruder_deprime_travel      = 1 == (eeprom::getEeprom8(eeprom_offsets::EXTRUDER_DEPRIME_ON_TRAVEL,
								DEFAULT_EXTRUDER_DEPRIME_ON_TRAVEL));

	//Maximum speed change
	max_speed_change[X_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,0), DEFAULT_MAX_SPEED_CHANGE_X));
	max_speed_change[Y_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,1), DEFAULT_MAX_SPEED_CHANGE_Y));
	max_speed_change[Z_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,2), DEFAULT_MAX_SPEED_CHANGE_Z));
	max_speed_change[A_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,3), DEFAULT_MAX_SPEED_CHANGE_A));
#if EXTRUDERS > 1
	max_speed_change[B_AXIS]  = FTOFP((float)eeprom::getEeprom16(AC2(MAX_SPEED_CHANGE,4), DEFAULT_MAX_SPEED_CHANGE_B));
#endif

#ifdef DEBUG_SLOW_MOTION
	max_speed_change[X_AXIS]  = FTOFP((float)1);
	max_speed_change[Y_AXIS]  = FTOFP((float)1);
	max_speed_change[Z_AXIS]  = FTOFP((float)0.15);
	max_speed_change[A_AXIS]  = FTOFP((float)1);
#if EXTRUDERS > 1
	max_speed_change[B_AXIS]  = FTOFP((float)1);
#endif
#endif

#ifdef FIXED
	smallest_max_speed_change = max_speed_change[Z_AXIS];
	for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
		if ( max_speed_change[i] < smallest_max_speed_change )
			smallest_max_speed_change = max_speed_change[i];
	}
#endif

	FPTYPE advanceK         = FTOFP((float)eeprom::getEeprom32(NAC2_2(JKN_ADVANCE_K),  DEFAULT_JKN_ADVANCE_K)         / 100000.0);
	FPTYPE advanceK2        = FTOFP((float)eeprom::getEeprom32(NAC2_2(JKN_ADVANCE_K2), DEFAULT_JKN_ADVANCE_K2)        / 100000.0);

	minimumSegmentTime = FTOFP((float)ACCELERATION_MIN_SEGMENT_TIME);

	// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
	// of the buffer and all stops. This should not be much greater than zero and should only be changed
	// if unwanted behavior is observed on a user's machine when running at very slow speeds.
	minimumPlannerSpeed = FTOFP((float)ACCELERATION_MIN_PLANNER_SPEED);

	if ( eeprom::getEeprom8(NAC2_2(SLOWDOWN_FLAG), DEFAULT_SLOWDOWN_FLAG) ) {
		slowdown_limit = (int)ACCELERATION_SLOWDOWN_LIMIT;
		if ( slowdown_limit > (BLOCK_BUFFER_SIZE / 2))  slowdown_limit = 0;
	}
	else	slowdown_limit = 0;

	//Clockwise extruder
	extrude_when_negative[0] = ACCELERATION_EXTRUDE_WHEN_NEGATIVE_A;
#if EXTRUDERS > 1
	extrude_when_negative[1] = ACCELERATION_EXTRUDE_WHEN_NEGATIVE_B;
#endif

	//These max feedrates limit the speed the extruder can move at when
	//it's been advanced, primed/deprimed and depressurized
	//It acts as an overall speed governer for the A/B axis
	//The values are obtained via the RepG xml and are updated on connection
	//with RepG if they're different than stored.  These values are in mm per
	//min, we divide by 60 here to get mm / sec.
	extruder_only_max_feedrate[0] = FPTOF(stepperAxis[A_AXIS].max_feedrate);
#if EXTRUDERS > 1
	extruder_only_max_feedrate[1] = FPTOF(stepperAxis[B_AXIS].max_feedrate);
#endif

	// Some gcode is loaded with enable/disable extruder commands. E.g., before each travel-only move.
	// This seems okay for 1.75 mm filament extruders.  However, it is problematic for 3mm filament
	// extruders: when the stepper motor is disabled, too much filament backs out owing to the high
	// melt chamber pressure and the free-wheeling pinch gear.  To combat this, the firmware has an
	// option to leave the extruder stepper motors engaged throughout an entire build, ignoring any
	// gcode / s3g command to disable the extruder stepper motors.
	extruder_hold[0] = ((eeprom::getEeprom8(eeprom_offsets::EXTRUDER_HOLD, DEFAULT_EXTRUDER_HOLD)) != 0);
#if EXTRUDERS > 1
	extruder_hold[1] = extruder_hold[0];
#endif

#ifdef PLANNER_OFF
	plannerMaxBufferSize = 1;
#else
	plannerMaxBufferSize = BLOCK_BUFFER_SIZE - 1;
#endif

	alterSpeed  = 0x00;
	speedFactor = KCONSTANT_1;

#if defined(PSTOP_SUPPORT) && defined(PSTOP_ZMIN_LEVEL) && defined(AUTO_LEVEL)
	command::max_zprobe_hits = (uint8_t)eeprom::getEeprom8(
	     eeprom_offsets::ALEVEL_MAX_ZPROBE_HITS,
	     ALEVEL_MAX_ZPROBE_HITS_DEFAULT);
#endif

	// Z holding indicates that when the Z axis is not in
	// motion, the machine should continue to power the stepper
	// coil to ensure that the Z stage does not shift.
	// Bit 7 of the AXIS_INVERSION eeprom setting
	// indicates whether or not to use z holding;
	// the bit is active low. (0 means use z holding,
	// 1 means turn it off.)
	uint8_t axis_invert = eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION, 0);
	bool hold_z = (axis_invert & (1<<7)) == 0;

	plan_init(advanceK, advanceK2, hold_z);		//Initialize planner
	st_init();					//Initialize stepper accel
}

//public:
void init() {
	is_running = false;
	is_homing = false;

	stepperAxisInit(true);
	DEBUG_VALUE(DEBUG_STEPPERS | 0x02);

	INITPOTS;
	DEBUG_VALUE(DEBUG_STEPPERS | 0x03);

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

Point removeOffsets(const Point &position) {
    Point p = position;
    for ( uint8_t i = 0; i < STEPPER_COUNT; i++ )
	p[i] -= (*tool_offsets)[i];

    return p;
}

/// Define current position as given point
void definePosition(const Point& position_in, bool home) {
	Point position_offset = position_in;

	for ( uint8_t i = 0; i < STEPPER_COUNT; i++ ) {
		stepperAxis[i].hasDefinePosition = true;

		//Add the toolhead offset
		if ( !home ) position_offset[i] += (*tool_offsets)[i];
	}

	plan_set_position(STEPPERS_(position_offset[X_AXIS],
								position_offset[Y_AXIS],
								position_offset[Z_AXIS],
								position_offset[A_AXIS],
								position_offset[B_AXIS]));
}


/// Get the last position of the planner
/// This is also the target position of the last command that was sent with
/// setTarget / setTargetNew / setTargetNewExt
/// Note this isn't the position of the hardware right now, use getStepperPosition for that.
/// If the pipeline buffer is empty, then getPlannerPosition == getStepperPosition
const Point getPlannerPosition() {
	Point p;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		p = Point(STEPPERS_(planner_position[X_AXIS],
							planner_position[Y_AXIS],
							planner_position[Z_AXIS],
							planner_position[A_AXIS],
							planner_position[B_AXIS]));
	}

	// Subtract out the toolhead offset
	for ( uint8_t i = 0; i < STEPPER_COUNT; i++ )
	     p[i] -= (*tool_offsets)[i];

#if defined(AUTO_LEVEL)
	// This needs to be done after removing the toolhead offsets
	if ( skew_active ) p[Z_AXIS] -= skew((int32_t *)&p.coordinates[0]);
#endif

	return p;
}


/// Get current position

#ifndef SIMULATOR

const Point getStepperPosition(uint8_t *toolIndex) {
	uint8_t active_toolhead;
	int32_t position[STEPPER_COUNT];

	st_get_position(STEPPERS_(&position[X_AXIS],
							  &position[Y_AXIS],
							  &position[Z_AXIS],
							  &position[A_AXIS],
							  &position[B_AXIS]),
					&active_toolhead);

	active_toolhead %= 2;	//Safeguard, shouldn't be needed
	*toolIndex = active_toolhead;

	//Because all targets have a toolhead offset added to them, we need to undo that here.
	//Also, because the toollhead can change, we need to use the active_toolhead from the hardware position
	//and can't use toolIndex
	Point *gp_tool_offsets;

	if ( active_toolhead == 1 )	gp_tool_offsets = &tolerance_offset_T1;
	else				gp_tool_offsets = &tolerance_offset_T0;

	//Subtract out the toolhead offset
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )
		position[i] -= (*gp_tool_offsets)[i];

#if defined(AUTO_LEVEL)
	// Down in stepper space, where this position came from, the skew has
	// been applied.  Thus we need to remove it.
	// This needs to be done after removing the toolhead offsets
	if ( skew_active ) position[Z_AXIS] -= skew(position);
#endif
	Point p = Point(STEPPERS_(position[X_AXIS], position[Y_AXIS],
							  position[Z_AXIS], position[A_AXIS],
							  position[B_AXIS]));

	return p;
}

#else

const Point getStepperPosition(uint8_t *toolIndex) {
	Point p = Point(0,0,0,0,0);
	*toolIndex = 0;
	return p;
}

#endif


void setTargetNew(const Point& target, int32_t dda_interval, int32_t us, uint8_t relative) {
	// Convert relative coordinates into absolute coordinates
	for ( uint8_t i = 0; i < STEPPER_COUNT; i++ ) {
	     planner_target[i] = target[i];
	     if ( (relative & (1 << i)) != 0 )
		  planner_target[i] += planner_position[i];
	}

#if defined(AUTO_LEVEL)
	// Apply the skew before the toolhead offsets
	// The skew transform is computed using coordinates which have had
	// the offsets removed
	if ( skew_active ) planner_target[Z_AXIS] += skew(planner_target);
#endif

	// Add on the toolhead offsets
	planner_target[X_AXIS] += (*tool_offsets)[X_AXIS];
	planner_target[Y_AXIS] += (*tool_offsets)[Y_AXIS];

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
	int32_t max_delta = 0;
	planner_master_steps_index = 0;
	planner_axes = 0;
#if !defined(CORE_XY) && !defined(CORE_XYZ)
        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = labs(planner_target[i] - planner_position[i]);
		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
        }
#elif defined(CORE_XYZ)
	// If us != 0, force recalc since max_delta may change
	//  Note that when jogging or homing, the passed value for
	//  us is zero.  HOWEVER, for those motions, max_delta
	//  doesn't change since one of X-steps or Y-steps is zero
	if ( us ) dda_interval = 0;

	int32_t delta_x = planner_target[X_AXIS] - planner_position[X_AXIS];
	int32_t delta_y = planner_target[Y_AXIS] - planner_position[Y_AXIS];
	int32_t delta_z = planner_target[Z_AXIS] - planner_position[Z_AXIS];

	delta_ab[X_AXIS] = delta_z + delta_y + delta_x;
	delta_ab[Y_AXIS] = delta_z + delta_y - delta_x;
	delta_ab[Z_AXIS] = delta_z - delta_y - delta_x;

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
	        if ( i <= Z_AXIS ) planner_steps[i] = labs(delta_ab[i]);
	        else planner_steps[i] = labs(planner_target[i] - planner_position[i]);
		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
        }
#else
	// If us != 0, force recalc since max_delta may change
	//  Note that when jogging or homing, the passed value for
	//  us is zero.  HOWEVER, for those motions, max_delta
	//  doesn't change since one of X-steps or Y-steps is zero
	if ( us ) dda_interval = 0;

	int32_t delta_x = planner_target[X_AXIS] - planner_position[X_AXIS];
	int32_t delta_y = planner_target[Y_AXIS] - planner_position[Y_AXIS];

	delta_ab[X_AXIS] = delta_x + delta_y;
	delta_ab[Y_AXIS] = delta_x - delta_y;

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
	        if ( i <= Y_AXIS ) planner_steps[i] = labs(delta_ab[i]);
	        else planner_steps[i] = labs(planner_target[i] - planner_position[i]);
		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
        }
#endif
	if ( !planner_axes )
	     // No motion along any axes
	     return;

        planner_master_steps = (uint32_t)max_delta;

	if ( planner_master_steps == 0 ) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	if ( dda_interval == 0 )
		dda_interval = us / max_delta;

	//dda_rate is the number of dda steps per second for the master axis
	uint32_t dda_rate = (uint32_t)(1000000 / dda_interval);

	plan_buffer_line(0, dda_rate, toolIndex, false, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Dda_rate is the number of dda steps per second for the master axis

void setTargetNewExt(const Point& target, int32_t dda_rate, uint8_t relative, float distance, int16_t feedrateMult64) {
	// Convert relative coordinates into absolute coordinates
	for ( uint8_t i = 0; i < STEPPER_COUNT; i++ ) {
	     planner_target[i] = target[i];
	     if ( (relative & (1 << i)) != 0 )
		  planner_target[i] += planner_position[i];
	}

#if defined(AUTO_LEVEL)
	// Apply the skew before the toolhead offsets
	// The skew transform is computed using coordinates which have had
	// the offsets removed
	if ( skew_active ) planner_target[Z_AXIS] += skew(planner_target);
#endif

	// Now add in the toolhead offsets
	planner_target[X_AXIS] += (*tool_offsets)[X_AXIS];
	planner_target[Y_AXIS] += (*tool_offsets)[Y_AXIS];

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
        int32_t max_delta = 0;
        planner_master_steps_index = 0;
	planner_axes = 0;
#if !defined(CORE_XY) && !defined(CORE_XYZ)
        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = planner_target[i] - planner_position[i];
		int32_t abs_planner_steps = labs(planner_steps[i]);
		if ( abs_planner_steps <= 0x7fff )
		     delta_mm[i] = FPMULT2(ITOFP(planner_steps[i]), axis_steps_per_unit_inverse[i]);
		else
		      // This typically only happens for LONG Z axis moves
		      // As such it typically happens three times per print
		     delta_mm[i] = FTOFP((float)planner_steps[i] * FPTOF(axis_steps_per_unit_inverse[i]));
                planner_steps[i] = abs_planner_steps;

		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
        }
#elif defined(CORE_XYZ)
	int32_t delta_x = planner_target[X_AXIS] - planner_position[X_AXIS];
	int32_t delta_y = planner_target[Y_AXIS] - planner_position[Y_AXIS];
	int32_t delta_z = planner_target[Z_AXIS] - planner_position[Z_AXIS];

	delta_ab[X_AXIS] = delta_z + delta_y + delta_x;
	delta_ab[Y_AXIS] = delta_z + delta_y - delta_x;
	delta_ab[Z_AXIS] = delta_z - delta_y - delta_x;

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
	        if ( i <= Z_AXIS ) planner_steps[i] = delta_ab[i];
                else planner_steps[i] = planner_target[i] - planner_position[i];
		int32_t abs_planner_steps = labs(planner_steps[i]);
		if ( abs_planner_steps <= 0x7fff )
		     delta_mm[i] = FPMULT2(ITOFP(planner_steps[i]), axis_steps_per_unit_inverse[i]);
		else
		      // This typically only happens for LONG Z axis moves
		      // As such it typically happens three times per print
		     delta_mm[i] = FTOFP((float)planner_steps[i] * FPTOF(axis_steps_per_unit_inverse[i]));
                planner_steps[i] = abs_planner_steps;
		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
	}
#else
	int32_t delta_x = planner_target[X_AXIS] - planner_position[X_AXIS];
	int32_t delta_y = planner_target[Y_AXIS] - planner_position[Y_AXIS];

	delta_ab[X_AXIS] = delta_x + delta_y;
	delta_ab[Y_AXIS] = delta_x - delta_y;

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
	        if ( i <= Y_AXIS ) planner_steps[i] = delta_ab[i];
                else planner_steps[i] = planner_target[i] - planner_position[i];
		int32_t abs_planner_steps = labs(planner_steps[i]);
		if ( abs_planner_steps <= 0x7fff )
		     delta_mm[i] = FPMULT2(ITOFP(planner_steps[i]), axis_steps_per_unit_inverse[i]);
		else
		      // This typically only happens for LONG Z axis moves
		      // As such it typically happens three times per print
		     delta_mm[i] = FTOFP((float)planner_steps[i] * FPTOF(axis_steps_per_unit_inverse[i]));
                planner_steps[i] = abs_planner_steps;
		if ( planner_steps[i] ) {
		     planner_axes |= 1 << i;
		     if ( planner_steps[i] > max_delta ) {
			  planner_master_steps_index = i;
			  max_delta = planner_steps[i];
		     }
		}
        }
#endif

	if ( !planner_axes )
	     // No steps; nothing to do
	     return;

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

#if defined(CORE_XY) || defined(CORE_XYZ)
	feedrate = ITOFP((int32_t)feedrateMult64);

	//Feed rate was multiplied by 64 before it was sent, undo
#ifdef FIXED
	feedrate >>= 6;
#else
	feedrate /= 64.0;
#endif

	// We need to recompute the dda rate
	//  Problem is that it can easily overflow FPTYPE and the
	//  checks to prevent this are themselves computationally
	//  expensive.

#ifdef FIXED
	if ( planner_master_steps < 0x7fff ) {
	     FPTYPE tmp = FPDIV(ITOFP((int32_t)planner_master_steps), planner_distance);

	     // 181 is a tad smaller than sqrt(0x7fff)
	     if ( feedrate > KCONSTANT_181 || tmp > KCONSTANT_181 )
		  dda_rate = (int32_t)((float)feedrateMult64 * FPTOF(tmp)) >> 6;
	     else
		  dda_rate = FPTOI(FPMULT2(feedrate, tmp));
	}
	else
	     dda_rate = (int32_t)((float)feedrateMult64 * (float)planner_master_steps / distance) >> 6;
#else
	dda_rate = (int32_t)((float)feedrateMult64 * (float)planner_master_steps / distance) >> 6;
#endif
	if ( dda_rate > 0x7fff ) dda_rate = 0x7fff;
#endif

	if ( acceleration ) {

#if !defined(CORE_XY) || !defined(CORE_XYZ)
		feedrate = ITOFP((int32_t)feedrateMult64);

		//Feed rate was multiplied by 64 before it was sent, undo
#ifdef FIXED
		feedrate >>= 6;
#else
		feedrate /= 64.0;
#endif
#endif

		if ( relative & 0x80 ) {
#ifdef FIXED
			feedrate = FPMULT2(feedrate, speedFactor);
			dda_rate = (int32_t)((float)dda_rate * FPTOF(speedFactor));
#else
		        feedrate *= speedFactor;
			dda_rate = (int32_t)((float)dda_rate * speedFactor);
#endif // FIXED
		}
	}

	plan_buffer_line(feedrate, dda_rate, toolIndex,
			 acceleration && segmentAccelState, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Step positions for homing.  We shift by >> 1 so that we can add
//tool_offsets without overflow
#if !defined(CORE_XY) && !defined(CORE_XY_STEPPER) && !defined(CORE_XYZ)
#define POSITIVE_HOME_POSITION ((INT32_MAX - 1) >> 1)
#define NEGATIVE_HOME_POSITION ((INT32_MIN + 1) >> 1)
#else
// We will be adding two of these so cut in half what we normally use
//   and half that again to be conservative
#define POSITIVE_HOME_POSITION ((INT32_MAX - 1) >> 3)
#define NEGATIVE_HOME_POSITION ((INT32_MIN + 1) >> 3)
#endif

/// Start homing

void startHoming(const bool maximums, const uint8_t axes_enabled, uint32_t us_per_step) {
	setSegmentAccelState(false);
	uint8_t dummy;
	Point target = getStepperPosition(&dummy);

        for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
                if ((axes_enabled & (1<<i)) == 0) {
			axis_homing[i] = false;
		} else {
	 		target[i] = (maximums) ? POSITIVE_HOME_POSITION : NEGATIVE_HOME_POSITION;
			axis_homing[i] = true;
			stepperAxis[i].hasHomed = true;
			if ( us_per_step < (uint32_t)stepperAxis_minInterval(i) )
			     us_per_step = (uint32_t)stepperAxis_minInterval(i);
                }
        }

	setTargetNew(target, us_per_step, 0, 0);

        is_homing = true;
}


/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
	if (index < STEPPER_COUNT) {
		stepperAxisSetEnabled(index, enable);
		if ( !enable ) {
			stepperAxis[index].hasDefinePosition = false;
			stepperAxis[index].hasHomed = false;
		}
	}
}

void enableAxes(uint8_t axes, bool enable) {
	for (uint8_t i = 0; i < STEPPER_COUNT; i++)
		if ( (axes & _BV(i)) != 0 )
			stepperAxisSetEnabled(i, enable);
}

/// Returns a bit mask for all axes enabled
uint8_t allAxesEnabled(void) {
	return axesEnabled;
}


#if !defined(SIMULATOR) && defined(DIGIPOT_SUPPORT)

/// set digital potentiometer for stepper axis
void setAxisPotValue(uint8_t index, uint8_t value) {
     if (index < STEPPER_COUNT)
	  DigiPots::setPotValue(index, value);
}


/// get the digital potentiometer for stepper axis
uint8_t getAxisPotValue(uint8_t index) {
     if (index < STEPPER_COUNT)
	  return DigiPots::getPotValue(index);
     return 0;
}

/// Reset the digital potentiometer for stepper axis to the stored eeprom value
void resetAxisPot(uint8_t index) {
     if (index < STEPPER_COUNT)
	  DigiPots::resetPot(index);
}

#else

void setAxisPotValue(uint8_t index, uint8_t value) { return; }
uint8_t getAxisPotValue(uint8_t index) { return 0; }
void resetAxisPot(uint8_t index) { return; }

#endif

/// Toggle segment acceleration on or off
/// Note this is also off if acceleration variable is not set
void setSegmentAccelState(bool state) {
     segmentAccelState = state;
}


void changeToolIndex(uint8_t tool) {
#if EXTRUDERS == 1
	(void)tool;
#else
     toolIndex = tool % 2;
     tool_offsets = ( toolIndex == 1 ) ?
		 &tolerance_offset_T1 : &tolerance_offset_T0;

#if 0
     // Queue a move to effect the change

     if ( toolIndex != oldIndex ) {

	  // If it weren't for auto-leveling we could just
	  // use (0,0,0,0,0) and do a relative move.

	  int32_t interval = stepperAxis_minInterval(X_AXIS);
	  if ( interval < 500 ) interval = 500;

#if !defined(AUTO_LEVEL)
	  // Relative move of (0, 0, 0, 0, 0)
	  Point target = Point();
	  setTargetNew(target, interval, 0, 0xFF);
#else
	  // planner_position[Z] is the *skewed* Z position.
	  //   We need to convert back to the gcode coordinate space by
	  //   removing the skew.  Then, when we call setTargetNew(),
	  //   it can add it back in causing a zero displacement move in Z.

	  Point target = Point(
		  STEPPERS_(planner_position[X_AXIS],
					planner_position[Y_AXIS],
					planner_position[Z_AXIS] - skew(planner_position),
					planner_position[A_AXIS],
					planner_position[B_AXIS]));
	  // Absolute move
	  setTargetNew(target, interval, 0, 0);
#endif // !defined(AUTO_LEVEL)
     }
#endif // 0
#endif // EXTRUDERS == 1
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
	if ( bufferUsed < 3 ) DEBUG_VALUE(DEBUG_STEPPERS | 0x00);
	else                  DEBUG_VALUE(DEBUG_STEPPERS | 0x01);
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
