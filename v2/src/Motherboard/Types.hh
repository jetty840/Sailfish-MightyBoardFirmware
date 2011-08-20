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

/// Class that represents an N-dimensional point, where N is the number of
/// stepper axes present in the system. Can support 3 or 5 axes.
class Point {
private:
        int32_t coordinates[AXIS_COUNT];        ///< n-dimensional coordinate
public:
        /// Default point constructor
	Point() {}

        /// Construct a point with the given cooridnates. Coordinates are in
        /// stepper steps.
        /// \param[in] x X axis position
        /// \param[in] y Y axis position
        /// \param[in] z Z axis position
        /// \param[in] a (if supported) A axis position
        /// \param[in] b (if supported) B axis position
        Point(int32_t x, int32_t y, int32_t z, int32_t a, int32_t b) {
		coordinates[0] = x;
		coordinates[1] = y;
		coordinates[2] = z;
#if AXIS_COUNT > 3
		coordinates[3] = a;
		coordinates[4] = b;
#endif
	}

        // TODO: Can this be removed by giving the 5-dimensional function
        //       some initial values?
        /// Construct a point with the given cooridnates. Coordinates are in
        /// stepper steps. If used on a 5-dimesional system, the A and B
        /// axes are set to zero.
        /// \param[in] x X axis position
        /// \param[in] y Y axis position
        /// \param[in] z Z axis position
	Point(int32_t x, int32_t y, int32_t z) {
		coordinates[0] = x;
		coordinates[1] = y;
		coordinates[2] = z;
#if AXIS_COUNT > 3
		coordinates[3] = 0;
		coordinates[4] = 0;
#endif
	}

        /// Constant array accessor.
        /// \param[in] index Axis to look up
        /// \return Axis position, in steps
        const int32_t& operator[](unsigned int index) const {
            return coordinates[index];
        }

        /// Array accessor.
        /// \param[in] index Axis to look up
        /// \return Reference to the variable containing the axis' position.
        int32_t& operator[](unsigned int index) {
            return coordinates[index];
        }

} __attribute__ ((__packed__));


#endif // TYPES_HH_
