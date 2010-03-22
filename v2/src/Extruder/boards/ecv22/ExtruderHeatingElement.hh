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

#ifndef BOARDS_ECV22_EXTRUDER_HEATING_ELEMENT_HH_
#define BOARDS_ECV22_EXTRUDER_HEATING_ELEMENT_HH_

#include <stdint.h>
#include "HeatingElement.hh"

class ExtruderHeatingElement : public HeatingElement {
public:
	void init();
	void setHeatingElement(uint8_t value);
};

#endif // BOARDS_ECV22_HEATING_ELEMENT_HH_
