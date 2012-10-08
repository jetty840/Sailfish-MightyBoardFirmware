/*
  StepperAccel.hh - stepper motor driver: executes motion plans of planner.c using the stepper motors
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

#ifndef STEPPERACCEL_HH
#define STEPPERACCEL_HH 

#include <inttypes.h>
#include "StepperAxis.hh"
#include "StepperAccelPlanner.hh"

#define MAX_STEP_FREQUENCY 40000 // Max step frequency for Ultimaker (5000 pps / half step)

//Enables the debug timer.  The timer can detected upto 4ms before overflowing.
//Example usage:
//	DEBUG_TIMER_START;
//	**** DO SOMETHING ****
//	DEBUG_TIMER_FINISH;
//	debug_onscreen1 = DEBUG_TIMER_TCTIMER_CYCLES;

#define DEBUG_TIMER

#if  defined(DEBUG_TIMER) && defined(TCCR5A)
	#define DEBUG_TIMER_START TCCR5A = 0x00;TCCR5B=0x01;TCNT5 = 0
	#define DEBUG_TIMER_FINISH debugTimer = TCNT5
	#define DEBUG_TIMER_TCTIMER_CYCLES ((float)debugTimer)
	#define DEBUG_TIMER_TCTIMER_US ((float)debugTimer / 16.0)	//16 = cpu frequency / 1000000
	#define DEBUG_TIMER_TCTIMER_USI (debugTimer / 16)   		//16 = cpu frequency / 1000000
	extern uint16_t debugTimer;
#else
	#define DEBUG_TIMER_START
	#define DEBUG_TIMER_FINISH
	#define DEBUG_TIMER_TCTIMER_CYCLES 0
	#define DEBUG_TIMER_TCTIMER_US 0.0
	#define DEBUG_TIMER_TCTIMER_USI 0
#endif


//If defined, the speed lookup table is used to calculate the timer
//otherwise, the timer is calculated with a divide.

// Don't use the lookup table when simulating -- simulator doesn't
// bother to provide a pgm mem library emulator
#ifndef SIMULATOR
	#define LOOKUP_TABLE_TIMER
#endif

#ifndef CRITICAL_SECTION_START
	#define CRITICAL_SECTION_START  unsigned char _sreg = SREG; cli();
	#define CRITICAL_SECTION_END    SREG = _sreg;
#endif //CRITICAL_SECTION_START

// Initialize and start the stepper motor subsystem
void st_init();

// Returns true is there are no buffered steps to be executed
bool st_empty();

// Set current position in steps
void st_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b);
void st_set_e_position(const int32_t &a, const int32_t &b);

// Get current position in steps
void st_get_position(int32_t *x, int32_t *y, int32_t *z, int32_t *a, int32_t *b, uint8_t *active_toolhead);

// Returns true if we deleted an item in the pipeline buffer
bool st_interrupt();

void st_extruder_interrupt();

void quickStop();
  

extern block_t	*current_block;  // A pointer to the block currently being traced
extern int16_t	extruder_deprime_steps[EXTRUDERS];
extern bool	extrude_when_negative[EXTRUDERS];
extern float	extruder_only_max_feedrate[EXTRUDERS];

//Enables and disables deprime
extern void st_deprime_enable(bool enable);

#endif
