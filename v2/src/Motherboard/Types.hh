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

#ifndef TYPES_HH_
#define TYPES_HH_

#include "Configuration.hh"

typedef uint32_t micros_t;

#define AXIS_COUNT STEPPER_COUNT

class Point {
private:
	int32_t coordinates[AXIS_COUNT];
public:
	Point() {}
	Point(int32_t x, int32_t y, int32_t z, int32_t a, int32_t b) {
		coordinates[0] = x;
		coordinates[1] = y;
		coordinates[2] = z;
#if AXIS_COUNT > 3
		coordinates[3] = a;
		coordinates[4] = b;
#endif
	}
	Point(int32_t x, int32_t y, int32_t z) {
		coordinates[0] = x;
		coordinates[1] = y;
		coordinates[2] = z;
#if AXIS_COUNT > 3
		coordinates[3] = 0;
		coordinates[4] = 0;
#endif
	}
	const int32_t& operator[](unsigned int index) const { return coordinates[index]; }
} __attribute__ ((__packed__));


#endif // TYPES_HH_
