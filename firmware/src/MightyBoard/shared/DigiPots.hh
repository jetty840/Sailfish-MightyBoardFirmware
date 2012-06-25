/*
* Copyright 2010 by Alison Leonard	 <alison@makerbot.com>
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

#ifndef DIGIPOTS_HH_
#define DIGIPOTS_HH_

#include <Pin.hh>




/// The StepperInterface module represents a connection to a single stepper controller.
/// \ingroup SoftwareLibraries
class DigiPots {
public:
	/// Default constructor
	DigiPots() {}
	DigiPots(
		const Pin& pot,
		const uint16_t &eeprom_base_in,
		const uint8_t vref_meausure_pin_in);
public:
		/// Initialize the pins for the interface
		/// \param[in] idx Stepper index that this interface refers to (used to look up
		///                it's settings in the EEPROM)
	void init(uint8_t idx);

	const Pin pot_pin;                ///< Pin (output) data line for i2c pot interface
	uint8_t vref_measure_pin;	  ///< Analog Pin number for vref measurement
	
	uint8_t digi_pot_max;

	uint16_t eeprom_base;             ///< Base address to read EEPROM configuration from
	uint16_t eeprom_pot_offset;       ///< Base address for the digi pot eeprom locations

public:
	
	/// set default values for i2c pots
	void resetPots();

	/// set i2c pot to specified value (0-127 valid)
	void setPotValue(const uint8_t val);
	
	/// get max vref from digi pot output
	void CalibrateVref();

};

#endif // DIGIPOTS_HH_
