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

#ifndef BOARDS_RRMBV12_PSU_HH_
#define BOARDS_RRMBV12_PSU_HH_

#include "Pin.hh"

/// Simple wrapper class for encapsulating the PSU functionality
/// \ingroup SoftwareLibraries
class PSU {
private:
        Pin psu_pin;    ///< Power supply enable pin
public:
        PSU(const Pin& psu);

	/// Initialize the PSU
	void init();

	/// Turn the PSU on or off
        /// \param[in] on True to turn the PSU on, False to turn it off.
	void turnOn(bool on);
};

#endif // BOARDS_RRMBV12_PSU_HH_
