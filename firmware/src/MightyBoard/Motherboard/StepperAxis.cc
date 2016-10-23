/*
 * Copyright August 10, 2010 by Jetty
 *
 * The purpose of these defines is to speed up stepper pin access whilst
 * maintaining minimal code and hardware abstraction.
 *
 * I/O functions based heavily on StepperPorts by Alison Leonard,
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

#include "Compat.hh"

#include "StepperAxis.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"

#if !defined(SIMULATOR)
//Optimize this better, maybe load defaults from progmem, x_min/max could combine invert_endstop/invert_axis into 1
//110 bytes

#if defined(ZYYX_3D_PRINTER) && defined(PSTOP_SUPPORT)
static StepperIOPort pstop_port = Z_STEPPER_MAX;
#define PSTOP_AXIS 2
#define PSTOP_AXIS_END maximum
#else
#define PSTOP_AXIS 0
static StepperIOPort pstop_port = X_STEPPER_MIN;
#define PSTOP_AXIS_END minimum
#endif

#if defined(PSTOP_SUPPORT)
#if defined(ZYYX_3D_PRINTER)
uint8_t pstop_enabled = 1;  // Enabled by default
uint8_t pstop_value   = 1;  // Inverted P-Stop switch
#else
uint8_t pstop_enabled = 0;  // Disabled by default
uint8_t pstop_value;        // Doesn't require static initialization when disabled
#endif
#endif

struct StepperAxisPorts stepperAxisPorts[STEPPER_COUNT] = {
#if defined(ZYYX_3D_PRINTER)
	{ X_STEPPER_STEP, X_STEPPER_DIR, X_STEPPER_ENABLE, X_STEPPER_MIN, X_STEPPER_MAX },
	{ Y_STEPPER_STEP, Y_STEPPER_DIR, Y_STEPPER_ENABLE, Y_STEPPER_MIN, Y_STEPPER_MAX },
#if defined(PSTOP_SUPPORT)
	{ Z_STEPPER_STEP, Z_STEPPER_DIR, Z_STEPPER_ENABLE, Z_STEPPER_MIN, STEPPER_NULL },
#else
	{ Z_STEPPER_STEP, Z_STEPPER_DIR, Z_STEPPER_ENABLE, Z_STEPPER_MIN, Z_STEPPER_MAX },
#endif
	{ A_STEPPER_STEP, A_STEPPER_DIR, A_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	},
#if EXTRUDERS > 1
	{ B_STEPPER_STEP, B_STEPPER_DIR, B_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	}
#endif
#else
#if defined(PSTOP_SUPPORT)
	{ X_STEPPER_STEP, X_STEPPER_DIR, X_STEPPER_ENABLE, STEPPER_NULL,  X_STEPPER_MAX },
#else
	{ X_STEPPER_STEP, X_STEPPER_DIR, X_STEPPER_ENABLE, X_STEPPER_MIN, X_STEPPER_MAX },
#endif
	{ Y_STEPPER_STEP, Y_STEPPER_DIR, Y_STEPPER_ENABLE, Y_STEPPER_MIN, Y_STEPPER_MAX },
	{ Z_STEPPER_STEP, Z_STEPPER_DIR, Z_STEPPER_ENABLE, Z_STEPPER_MIN, Z_STEPPER_MAX },
	{ A_STEPPER_STEP, A_STEPPER_DIR, A_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	},
#if EXTRUDERS > 1
	{ B_STEPPER_STEP, B_STEPPER_DIR, B_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	}
#endif
#endif // ZYYX_3D_PRINTER
};

#endif // !SIMULATOR

struct StepperAxis stepperAxis[STEPPER_COUNT];

volatile int32_t dda_position[STEPPER_COUNT];
volatile bool    axis_homing[STEPPER_COUNT];
volatile int16_t e_steps[EXTRUDERS];
volatile uint8_t axesEnabled;			//Planner axis enabled
volatile uint8_t axesHardwareEnabled;		//Hardware axis enabled

/// Initialize a stepper axis
void stepperAxisInit(bool hard_reset) {
	uint8_t axes_invert = 0, endstops_invert = 0;
	if ( hard_reset ) {
		//Load the defaults
		axes_invert	= eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION, 0);
		endstops_invert = eeprom::getEeprom8(eeprom_offsets::ENDSTOP_INVERSION, 0);
#if defined(PSTOP_SUPPORT)
		if ( !pstop_enabled ) {
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.port  = pstop_port.port;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.iport = pstop_port.iport;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.pin   = pstop_port.pin;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.ddr   = pstop_port.ddr;
		}
		else {
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.port  = 0;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.iport = 0;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.pin   = 0;
			stepperAxisPorts[PSTOP_AXIS].PSTOP_AXIS_END.ddr   = 0;
		}
#if defined(PSTOP_2_SUPPORT)
		extrusion_seen[0] = false;
#if EXTRUDERS > 1
		extrusion_seen[1] = false;
#endif
#endif
#endif
	}

	//Initialize the stepper pins
	bool endstops_present = (endstops_invert & (1<<7)) != 0;
	for (uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		if ( hard_reset ) {
			//Setup axis inversion, endstop inversion and steps per mm from the values
			//stored in eeprom
			// If endstops are not present, then we consider them inverted, since they will
			// always register as high (pulled up).
			stepperAxis[i].invert_endstop = !endstops_present || ((endstops_invert & (1<<i)) != 0);
			stepperAxis[i].invert_axis = (axes_invert & (1<<i)) != 0;

			stepperAxis[i].steps_per_mm = (float)eeprom::getEeprom32(eeprom_offsets::AXIS_STEPS_PER_MM + i * sizeof(uint32_t),
								   	         replicator_axis_steps_per_mm::axis_steps_per_mm[i]) / 1000000.0;

			stepperAxis[i].max_feedrate = FTOFP((float)eeprom::getEeprom32(eeprom_offsets::AXIS_MAX_FEEDRATES + i * sizeof(uint32_t),
										       replicator_axis_max_feedrates::axis_max_feedrates[i]) / 60.0);

			// max jogging speed for an axis is the min count of microseconds per step
			// min us/step = (1000000 us/s) / [ (max mm/s) * (axis steps/mm) ]
			int32_t f = (int32_t)(stepperAxis[i].steps_per_mm * FPTOF(stepperAxis[i].max_feedrate));
			stepperAxis[i].min_interval = f ? 1000000 / f : 500;

			//Read the axis lengths in
			int32_t length = (int32_t)((float)eeprom::getEeprom32(eeprom_offsets::AXIS_LENGTHS + i * sizeof(uint32_t), replicator_axis_lengths::axis_lengths[i]) *
						    stepperAxis[i].steps_per_mm);
			int32_t *axisMin = &stepperAxis[i].min_axis_steps_limit;
			int32_t *axisMax = &stepperAxis[i].max_axis_steps_limit;

			switch(i) {
			case X_AXIS:
			case Y_AXIS:
				//Half the axis in either direction around the center point
				*axisMax = length / 2;
				*axisMin = - (*axisMax);
				break;
			case Z_AXIS:
				// ***** WARNING *****
				// The following assumes the Z home offset is close to zero.  Thus
				// there's an implicit assumption that Z min homing is done.  If Z max
				// homing is done instead, then #define Z_HOME_MAX
#ifndef Z_HOME_MAX
				//Z is special, as 0 as at the top, so min is 0, and max = length - Z Home Offset
				*axisMax = length - (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + i * sizeof(uint32_t), 0);
#else
				//We home to Z max and so axis min = 0 and axis max is Z Home Position
				*axisMax = (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + i * sizeof(uint32_t), 0);
#endif
				*axisMin = 0;
				break;
			case A_AXIS:
			case B_AXIS:
				*axisMax = length;
				*axisMin = - length;
				break;
			}

			//Setup the pins
			STEPPER_IOPORT_SET_DIRECTION(stepperAxisPorts[i].dir, true);
			STEPPER_IOPORT_SET_DIRECTION(stepperAxisPorts[i].step, true);

			//Enable is active low
			STEPPER_IOPORT_WRITE(stepperAxisPorts[i].enable, true);
			STEPPER_IOPORT_SET_DIRECTION(stepperAxisPorts[i].enable, true);

			// Setup minimum and maximum ports for input
			// Use pullup pins to avoid triggering when using inverted endstops
			if ( ! STEPPER_IOPORT_NULL(stepperAxisPorts[i].maximum) ) {
				STEPPER_IOPORT_SET_DIRECTION(stepperAxisPorts[i].maximum, false);
				STEPPER_IOPORT_WRITE(stepperAxisPorts[i].maximum, stepperAxis[i].invert_endstop);
			}

			if ( ! STEPPER_IOPORT_NULL(stepperAxisPorts[i].minimum) ) {
				STEPPER_IOPORT_SET_DIRECTION(stepperAxisPorts[i].minimum, false);
				STEPPER_IOPORT_WRITE(stepperAxisPorts[i].minimum, stepperAxis[i].invert_endstop);
			}

			//We reset this here because we don't want an abort to lose track of positioning
			dda_position[i]	= 0;

			stepperAxis[i].hasHomed		 = false;
        		stepperAxis[i].hasDefinePosition = false;
		}

		//Setup the higher level stuff functionality / create the ddas
		axis_homing[i]				= false;
		stepperAxis[i].dda.eAxis		= (i >= A_AXIS) ? true : false;
		stepperAxis[i].dda.counter		= 0;
		stepperAxis[i].dda.direction		= 1;
		stepperAxis[i].dda.stepperDir		= false;
		stepperAxis[i].dda.master		= false;
		stepperAxis[i].dda.master_steps		= 0;
		stepperAxis[i].dda.steps_completed	= 0;
		stepperAxis[i].dda.steps		= 0;
	}

	if ( hard_reset ) {
		axesEnabled = 0;
		axesHardwareEnabled = 0;
#if defined(PSTOP_SUPPORT)
#if defined(PSTOP_PORT)
		// PSTOP port is input and ensure pull up resistor is deactivated
                if ( pstop_enabled ) {
			PSTOP_PORT.setDirection(false);
			PSTOP_PORT.setValue(false);
		}
#endif
#if defined(PSTOP2_PORT)
		PSTOP2_PORT.setDirection(false);
		PSTOP2_PORT.setValue(true);
#endif
#endif
	}

	for (uint8_t i = 0; i < EXTRUDERS; i ++ )
		e_steps[i] = 0;
}

/// Returns the steps per mm for the given axis
float stepperAxisStepsPerMM(uint8_t axis)
{
        return stepperAxis[axis].steps_per_mm;
}

/// Convert steps to mm's, as accurate as floating point is
float stepperAxisStepsToMM(int32_t steps, uint8_t axis) {
        return (float)steps / stepperAxis[axis].steps_per_mm;
}

//Convert mm's to steps for the given axis
//Accurate to 1/1000 mm
int32_t stepperAxisMMToSteps(float mm, uint8_t axis) {
        //Multiply mm by 1000 to avoid floating point errors
        int64_t intmm = (int64_t)(mm * 1000.0);

        //Calculate the number of steps
        int64_t ret = intmm * (int64_t)stepperAxis[axis].steps_per_mm;

        ret /= 1000;

        return (int32_t)ret;
}
