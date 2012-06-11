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

#include "PSU.hh"

PSU::PSU(const Pin& psu) :
    psu_pin(psu)
{
}

void PSU::init() {
        psu_pin.setDirection(true);
        turnOn(true);
}

void PSU::turnOn(bool on) {
	// PSU pin is pulled low to turn on power supply
        psu_pin.setValue(!on);
}
