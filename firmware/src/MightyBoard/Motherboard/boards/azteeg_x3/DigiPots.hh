/*
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

namespace DigiPots {

     void init();

     /// set default values for i2c pots
     void resetPot(uint8_t axis);

     /// set i2c pot to specified value (0 - 255 valid)
     void setPotValue(uint8_t axis, const uint8_t val);

     /// returns the last pot value set
     uint8_t getPotValue(uint8_t axis);
};

#endif // DIGIPOTS_HH_
