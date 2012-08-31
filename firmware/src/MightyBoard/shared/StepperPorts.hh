/*
 * Copyright 2010 by Alison Leonard	 <alison@makerbot.com>
 * 
 * The purpose of these defines is to speed up stepper pin access
 * 
 * IO functions based heavily on fastio.h in Marlin
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


//#define STEP0_PORT 		PORTG
//#define STEP0_PIN 		PING1


/*
#define X_STEP_PORT		PORTF	//step 
#define X_STEP_PIN		PINF1
#define X_STEP_DDR		DDRF
#define X_DIR_PORT		PORTF	//dir
#define X_DIR_PIN		PINF0
#define X_DIR_DDR		DDRF
#define X_ENABLE_PORT	PORTF	//enable
#define X_ENABLE_IPORT	PINF
#define X_ENABLE_PIN	PINF2
#define X_ENABLE_DDR	DDRF
#define X_MIN_IPORT		PINL	//min
#define X_MIN_PORT		PORTL
#define X_MIN_PIN		PINL0
#define X_MIN_DDR		DDRL
#define X_MAX_IPORT		PINL	//max
#define X_MAX_PORT		PORTL
#define X_MAX_PIN		PINL1
#define X_MAX_DDR		DDRL

#define Y_STEP_PORT		PORTF	//step 
#define Y_STEP_PIN		PINF5
#define Y_STEP_DDR		DDRF
#define Y_DIR_PORT		PORTF	//dir
#define Y_DIR_PIN		PINF4
#define Y_DIR_DDR		DDRF
#define Y_ENABLE_PORT	PORTF	//enable
#define Y_ENABLE_IPORT	PINF
#define Y_ENABLE_PIN	PINF6
#define Y_ENABLE_DDR	DDRF
#define Y_MIN_IPORT		PINL	//min
#define Y_MIN_PORT		PORTL
#define Y_MIN_PIN		PINL2
#define Y_MIN_DDR		DDRL
#define Y_MAX_IPORT		PINL	//max
#define Y_MAX_PORT		PORTL
#define Y_MAX_PIN		PINL3
#define Y_MAX_DDR		DDRL

#define Z_STEP_PORT		PORTK	//step 
#define Z_STEP_PIN		PINK1
#define Z_STEP_DDR		DDRK
#define Z_DIR_PORT		PORTK	//dir
#define Z_DIR_PIN		PINK0
#define Z_DIR_DDR		DDRK
#define Z_ENABLE_PORT	PORTK	//enable
#define Z_ENABLE_IPORT	PINK	
#define Z_ENABLE_PIN	PINK2
#define Z_ENABLE_DDR	DDRK
#define Z_MIN_IPORT		PINL	//min
#define Z_MIN_PORT		PORTL
#define Z_MIN_PIN		PINL6
#define Z_MIN_DDR		DDRL
#define Z_MAX_IPORT		PINL	//max
#define Z_MAX_PORT		PORTL
#define Z_MAX_PIN		PINL7
#define Z_MAX_DDR		DDRL


#define A_STEP_PORT		PORTA	//step 
#define A_STEP_PIN		PINA3
#define A_STEP_DDR		DDRA
#define A_DIR_PORT		PORTA	//dir
#define A_DIR_PIN		PINA2
#define A_DIR_DDR		DDRA
#define A_ENABLE_PORT	PORTA	//enable
#define A_ENABLE_IPORT	PINA	//enable
#define A_ENABLE_PIN	PINA4
#define A_ENABLE_DDR	DDRA

#define B_STEP_PORT		PORTA	//step 
#define B_STEP_PIN		PINA7
#define B_STEP_DDR		DDRA
#define B_DIR_PORT		PORTA	//dir
#define B_DIR_PIN		PINA6
#define B_DIR_DDR		DDRA
#define B_ENABLE_IPORT	PING	//enable
#define B_ENABLE_PORT	PORTG	//enable
#define B_ENABLE_PIN	PING2
#define B_ENABLE_DDR	DDRG
*/

#include "Configuration.hh" 

/// we map the stepper pins to arbitrary number for use in the sped up pin-access functions
#define X_DIR			0
#define X_STEP			1
#define X_ENABLE		2
#define X_MAX			3
#define X_MIN			4

#define Y_DIR			10
#define Y_STEP			11
#define Y_ENABLE		12
#define Y_MAX			13
#define Y_MIN			14

#define Z_DIR			20
#define Z_STEP			21
#define Z_ENABLE		22	
#define Z_MAX			23
#define Z_MIN			24

#define A_DIR			30		
#define A_STEP			31
#define A_ENABLE		32

#define B_DIR			40		
#define B_STEP			41
#define B_ENABLE		41

/// write a pin
#define _WRITE(IO, v)  do { if (v) {IO ## _PORT |= _BV(IO ## _PIN); } else {IO ## _PORT &= ~_BV(IO ## _PIN); }; } while (0)

/// read a pin
#define _READ(IO) ((bool)(IO ## _IPORT & _BV(IO ## _PIN)))

/// set pin as input
#define	_SET_DIRECTION(IO, v) do { if (v) {IO ## _DDR |=  _BV(IO ## _PIN); } else {IO ## _DDR &= ~_BV(IO ## _PIN); };} while (0)

/// set pin as input
#define	_SET_INPUT(IO) do {IO ## _DDR &= ~_BV(IO ## _PIN); } while (0)
/// set pin as output
#define	_SET_OUTPUT(IO) do {IO ## _DDR |=  _BV(IO ## _PIN); } while (0)

