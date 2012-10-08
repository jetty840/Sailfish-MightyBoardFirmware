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

#include "StepperAxis.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"

#ifndef SIMULATOR
//Optimize this better, maybe load defaults from progmem, x_min/max could combine invert_endstop/invert_axis into 1
//110 bytes
struct StepperAxisPorts stepperAxisPorts[STEPPER_COUNT] = {
	{ X_STEPPER_STEP, X_STEPPER_DIR, X_STEPPER_ENABLE, X_STEPPER_MIN, X_STEPPER_MAX },
	{ Y_STEPPER_STEP, Y_STEPPER_DIR, Y_STEPPER_ENABLE, Y_STEPPER_MIN, Y_STEPPER_MAX },
	{ Z_STEPPER_STEP, Z_STEPPER_DIR, Z_STEPPER_ENABLE, Z_STEPPER_MIN, Z_STEPPER_MAX },
	{ A_STEPPER_STEP, A_STEPPER_DIR, A_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	},
	{ B_STEPPER_STEP, B_STEPPER_DIR, B_STEPPER_ENABLE, STEPPER_NULL,  STEPPER_NULL	}
};
#endif

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
	}

	//Initialize the stepper pins
	for (uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		if ( hard_reset ) {
			//Setup axis inversion, endstop inversion and steps per mm from the values
			//stored in eeprom
			bool endstops_present = (endstops_invert & (1<<7)) != 0;	

			// If endstops are not present, then we consider them inverted, since they will
			// always register as high (pulled up).
			stepperAxis[i].invert_endstop = !endstops_present || ((endstops_invert & (1<<i)) != 0);
			stepperAxis[i].invert_axis = (axes_invert & (1<<i)) != 0;

			stepperAxis[i].steps_per_mm = (float)eeprom::getEeprom32(eeprom_offsets::AXIS_STEPS_PER_MM + i * sizeof(uint32_t),
								   	         replicator_axis_steps_per_mm::axis_steps_per_mm[i]) / 1000000.0;

			stepperAxis[i].max_feedrate = (float)eeprom::getEeprom32(eeprom_offsets::AXIS_MAX_FEEDRATES + i * sizeof(uint32_t),
                                                                   		 replicator_axis_max_feedrates::axis_max_feedrates[i]) / 60.0;

			//Read the axis lengths in
                	int32_t length = (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_LENGTHS + i * sizeof(uint32_t), replicator_axis_lengths::axis_lengths[i]);
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
                                	//Z is special, as 0 as at the top, so min is 0, and max = length - Z Home Offset
                                	*axisMax = length - (int32_t)eeprom::getEeprom32(eeprom_offsets::AXIS_HOME_POSITIONS_STEPS + i * sizeof(uint32_t), 0);
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
