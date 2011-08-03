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

#ifndef HEATINGELEMENT_HH_
#define HEATINGELEMENT_HH_

#include <stdint.h>

/// The heating element interface is a standard interface used to communicate with
/// things that can control
/// \ingroup SoftwareLibraries
class HeatingElement {
public:
        /// Set the output of the heating element
        /// \param[in] value Duty cycle to run the heating element at, 0 - 255.
        ///                  Note that some implementations (relay driven DC motors)
        ///                  may not support this, and will interpret this as
        ///                  a binary on/off command instead.
        virtual void setHeatingElement(uint8_t value) =0;
};

#endif // HEATINGELEMENT_HH_
