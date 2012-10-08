/*
 * Copyright August 10, 2010 by Jetty
 * 
 * The purpose of these defines is to speed up stepper pin access whilst
 * maintaining minimal code and hardware abstraction.
 * 
 * IO functions based heavily on StepperPorts by Alison Leonard,
 * which was based heavily on fastio.h in Marlin
 * by Triffid_Hunter and modified by Kliment
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

#ifndef STEPPER_AXIS_HH_
#define STEPPER_AXIS_HH_

#include "Configuration.hh"
#ifdef SIMULATOR
#include "Simulator.hh"
#endif

enum AxisEnum {
        X_AXIS=0,
        Y_AXIS,
        Z_AXIS,
        A_AXIS,
        B_AXIS
};

#ifndef SIMULATOR

#define  FORCE_INLINE __attribute__((always_inline)) inline

#include "Motherboard.hh"

/// write a pin
#define STEPPER_IOPORT_WRITE(IOPORT, v)		do {								\
							if (v)	_SFR_MEM8(IOPORT.port) |=  _BV(IOPORT.pin);	\
							else	_SFR_MEM8(IOPORT.port) &= ~_BV(IOPORT.pin);	\
						} while(0)

/// read a pin
#define STEPPER_IOPORT_READ(IOPORT)		((bool)(_SFR_MEM8(IOPORT.iport) & _BV(IOPORT.pin)))

/// set pin as input
#define	STEPPER_IOPORT_SET_DIRECTION(IOPORT, v)	do {								\
							if (v)	_SFR_MEM8(IOPORT.ddr) |=  _BV(IOPORT.pin);	\
							else	_SFR_MEM8(IOPORT.ddr) &= ~_BV(IOPORT.pin);	\
						} while(0)

/// set pin as input
#define	STEPPER_IOPORT_SET_INPUT(IOPORT)	do {							\
							_SFR_MEM8(IOPORT.ddr) &= ~_BV(IOPORT.pin);	\
						} while(0)

/// set pin as output
#define	STEPPER_IOPORT_SET_OUTPUT(IOPORT)	do {							\
							_SFR_MEM8(IOPORT.ddr) |=  _BV(IOPORT.pin);	\
						} while(0)

/// True if port is not defined
#define STEPPER_IOPORT_NULL(IOPORT)		(IOPORT.port == 0)

#else

#define STEPPER_IOPORT_WRITE(IOPORT, v)
#define STEPPER_IOPORT_READ(IOPORT) (uint8_t)0x00
#define	STEPPER_IOPORT_SET_DIRECTION(IOPORT, v)
#define	STEPPER_IOPORT_SET_INPUT(IOPORT)
#define	STEPPER_IOPORT_SET_OUTPUT(IOPORT)
#define STEPPER_IOPORT_NULL(IOPORT) (true)

#endif

struct StepperIOPort {
	uint16_t port;
	uint16_t iport;
	uint8_t pin;
	uint16_t ddr;
};

#define STEPPER_NULL	{ 0, 0, 0, 0 }

struct dda {
        bool    master;         //True if this is the master steps axis
        int32_t master_steps;   //The number of steps for the master axis
        bool    eAxis;          //True if this is the e axis
        char    direction;      //Direction of the dda, 1 = forward, -1 = backwards
        bool    stepperDir;     //The direction the stepper gets sent in
	bool	enabled;	//True if this dda is enabled, 0 if target is reached or
				//this axis isn't moving. (Z and 1 extruder frequently don't move)
				//This variable acts to speed up processing.

        int32_t counter;                //Used for the dda counter
        int32_t steps_completed;        //Number of steps completed
        int32_t steps;                  //Number of steps we need to execute for this axis
};

/// The reason we don't put StepperAxisPorts and StepperAxis in the same structure,
/// is because we need to initialize the port information, but none of the rest,
/// so we save space by using this approach
struct StepperAxisPorts {
	StepperIOPort step, dir, enable, minimum, maximum;
};

struct StepperAxis {
	bool invert_endstop;
	bool invert_axis;
	float steps_per_mm;
	float max_feedrate;
	bool hasHomed;		//True if this axis has homed
	bool hasDefinePosition;	//True if this axis has had a definePosition
	int32_t min_axis_steps_limit;
	int32_t max_axis_steps_limit;
	struct dda dda;
};

extern struct StepperAxisPorts	stepperAxisPorts[STEPPER_COUNT];
extern struct StepperAxis 	stepperAxis[STEPPER_COUNT];


extern volatile int32_t dda_position[STEPPER_COUNT];
extern volatile int16_t e_steps[EXTRUDERS];
extern volatile bool    axis_homing[STEPPER_COUNT];
extern volatile uint8_t axesEnabled;			//Planner axis enabled
extern volatile uint8_t axesHardwareEnabled;		//Hardware axis enabled


/// Set the direction of the next step
FORCE_INLINE void stepperAxisSetDirection(uint8_t axis, bool forward) {
	STEPPER_IOPORT_WRITE(stepperAxisPorts[axis].dir, (stepperAxis[axis].invert_axis) ? (! forward) : forward);
}
	
/// Step

///***** SHOULD THIS BE REALLY false, true
FORCE_INLINE void stepperAxisStep(uint8_t axis, bool value) {
	STEPPER_IOPORT_WRITE(stepperAxisPorts[axis].step, value);
}

/// The A3982 steper driver chip has an inverted enable
FORCE_INLINE void stepperAxisSetHardwareEnabled(uint8_t axis, bool enabled ) {
	if (enabled)	axesHardwareEnabled |=  _BV(axis);
	else		axesHardwareEnabled &= ~_BV(axis);

	STEPPER_IOPORT_WRITE(stepperAxisPorts[axis].enable, (! enabled));
}

FORCE_INLINE void stepperAxisSetHardwareEnabledToMatch(uint8_t desiredAxesEnabled) {
	if ( desiredAxesEnabled == axesHardwareEnabled )	return;
	uint8_t bitMask;
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ )	{
		bitMask = _BV(i);
		if (( desiredAxesEnabled & bitMask ) != ( axesHardwareEnabled & bitMask ))
			stepperAxisSetHardwareEnabled(i, (bool)(desiredAxesEnabled & bitMask));
	}
}

//Because the planner is populated before the enable axis needs to happen, we
//have a software axis enable and a hardware axis enable.  The
//software axis enable routinely should be called
//(stepperAxisSetHardwareEnabled is called automatically by the Stepper driver)
FORCE_INLINE void stepperAxisSetEnabled(uint8_t axis, bool enabled) {
	if (enabled)	axesEnabled |=  _BV(axis);
	else		axesEnabled &= ~_BV(axis);
}

/// Returns true if we're at a maximum endstop
FORCE_INLINE bool stepperAxisIsAtMaximum(uint8_t axis) {
	return (STEPPER_IOPORT_NULL(stepperAxisPorts[axis].maximum)) ? false : (STEPPER_IOPORT_READ(stepperAxisPorts[axis].maximum) ^ stepperAxis[axis].invert_endstop);
}

/// Returns true if we're at a minimum endstop
FORCE_INLINE bool stepperAxisIsAtMinimum(uint8_t axis) {
	return (STEPPER_IOPORT_NULL(stepperAxisPorts[axis].minimum)) ? false : (STEPPER_IOPORT_READ(stepperAxisPorts[axis].minimum) ^ stepperAxis[axis].invert_endstop);
}

/// Makes a step, but checks if an endstop is triggered first, if it is, the
/// step is abandoned and "true" is returned.
FORCE_INLINE void stepperAxisStepWithEndstopCheck(uint8_t axis, bool direction) {
	if (( (direction)   && (! stepperAxisIsAtMaximum(axis))) ||
	    ( (! direction) && (! stepperAxisIsAtMinimum(axis))))
		stepperAxisStep(axis, true);
	else	axis_homing[axis] = false;
}

/// DDA

#define DDA_IND stepperAxis[ind].dda

FORCE_INLINE void stepperAxis_dda_reset(uint8_t ind, bool master, int32_t master_steps, bool direction, int32_t steps)
{
	DDA_IND.enabled		      = (steps != 0 );

	if ( ! DDA_IND.enabled ) return;	//If we're not enabled, we don't calculate the rest.

	DDA_IND.counter  = master_steps >> 1;

#ifdef OVERSAMPLED_DDA
        DDA_IND.counter  = - (DDA_IND.counter << OVERSAMPLED_DDA);
#else
        DDA_IND.counter  = - DDA_IND.counter;
#endif

        DDA_IND.master                = master;
#ifdef OVERSAMPLED_DDA
        DDA_IND.master_steps          = master_steps << OVERSAMPLED_DDA;
#else
        DDA_IND.master_steps          = master_steps;
#endif
        DDA_IND.steps         = steps;
        DDA_IND.direction     = (direction) ? -1 : 1;
        DDA_IND.stepperDir    = (direction) ? false : true;

        DDA_IND.steps_completed = 0;
}

FORCE_INLINE void stepperAxis_dda_shift_phase16(uint8_t ind, int16_t phase)
{
#ifdef OVERSAMPLED_DDA
        DDA_IND.counter += phase << OVERSAMPLED_DDA;
#else
        DDA_IND.counter += phase;
#endif
}

FORCE_INLINE void stepperAxis_dda_shift_phase32(uint8_t ind, int32_t phase)
{
#ifdef OVERSAMPLED_DDA
        DDA_IND.counter += phase << OVERSAMPLED_DDA;
#else
        DDA_IND.counter += phase;
#endif
}

FORCE_INLINE void stepperAxis_dda_step(uint8_t ind)
{
	if ( ! DDA_IND.enabled )	return;

	DDA_IND.counter += DDA_IND.steps;
	if (( DDA_IND.counter > 0 ) && ( DDA_IND.steps_completed < DDA_IND.steps ))
	{
		DDA_IND.counter -= DDA_IND.master_steps;

#ifdef JKN_ADVANCE
               	if ( DDA_IND.eAxis ) {
#ifndef SIMULATOR
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
			//This generates a warning: array subscript is below array bounds [-Warray-bounds]
			//However we override this warning because anything below A_AXIS can never be
			//an eAxis, and to test would require extra cycles and we don't need to
			e_steps[ind-A_AXIS] += DDA_IND.direction;
#ifndef SIMULATOR
#pragma GCC diagnostic pop
#endif
		}
		else
		{
#endif
			stepperAxisSetDirection(ind, DDA_IND.stepperDir );
			stepperAxisStepWithEndstopCheck(ind, DDA_IND.stepperDir);
			stepperAxisStep(ind, false);
#ifdef JKN_ADVANCE
		}
#endif

		dda_position[ind] += DDA_IND.direction;
		DDA_IND.steps_completed ++;
	}
}

/// Clips an axis to the minimum step limit.  It returns target if it doesn't require clipping,
/// and min_axis_steps_limit if it does
FORCE_INLINE int32_t stepperAxis_clip_to_min(uint8_t axis, int32_t target)
{
	return ( target < stepperAxis[axis].min_axis_steps_limit ) ? stepperAxis[axis].min_axis_steps_limit : target;
}

/// Clips an axis to the maximum step limit.  It returns target if it doesn't require clipping,
/// and max_axis_steps_limit if it does
FORCE_INLINE int32_t stepperAxis_clip_to_max(uint8_t axis, int32_t target)
{
	return ( target > stepperAxis[axis].max_axis_steps_limit ) ? stepperAxis[axis].max_axis_steps_limit : target;
}

extern void stepperAxisInit(bool hard_reset);
extern float stepperAxisStepsPerMM(uint8_t axis);
extern float stepperAxisStepsToMM(int32_t steps, uint8_t axis);
extern int32_t stepperAxisMMToSteps(float mm, uint8_t axis);

#endif // STEPPER_AXIS_HH_
