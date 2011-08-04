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
 */

#ifndef ANALOG_PIN_HH_
#define ANALOG_PIN_HH_

#include <stdint.h>

/// Porting notes:
/// This needs to be ported to each processor architecture.

/// Initialize a hardware pin to work in analog mode.
/// \param [in] Analog input number (0-7 on the Atmega168).
void initAnalogPin(uint8_t pin);


/// Initialize an asynchronous analog read from the specified input. The pin must first be
/// placed into analog input mode by a call to #initAnalogPin(). The ADC will be set to
/// interrupt mode,
/// \param [in] pin Analog input number that should be read (processor-specific)
/// \param [out] destination Address to store the result of the analog read.
/// \param [out] finished This flag will be set to true once the analog read has been
///              completed, and the output is stored in destination.
bool startAnalogRead(uint8_t pin, volatile int16_t* destination, volatile bool* finished);

#endif /* ANALOG_PIN_HH_ */
