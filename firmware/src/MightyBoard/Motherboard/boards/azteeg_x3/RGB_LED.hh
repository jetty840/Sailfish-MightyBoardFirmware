/*
 * Copyright 2015 by Dan Newman <dan.newman@mtbaldy.us>
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

#ifndef RGB_LED_HH
#define RGB_LED_HH

#include "Types.hh"

#define SET_COLOR(r,g,b,c) RGB_LED::setColor((r),(g),(b))

namespace RGB_LED {
 void init();
 void errorSequence();
 void setColor(uint8_t red, uint8_t green, uint8_t blue);
 void setDefaultColor(uint8_t c = 0xff);
 void setCustomColor(uint8_t red, uint8_t green, uint8_t blue);
}
#endif
