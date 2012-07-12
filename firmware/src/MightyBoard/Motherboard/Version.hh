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

#ifndef VERSION_HH_
#define VERSION_HH_

#include <stdint.h>

#ifndef VERSION
#error "Version not defined! Please define the version number for this build."
#else
const uint16_t firmware_version = VERSION;
#endif

#ifndef VERSION_INTERNAL
const uint16_t internal_version = 0;
#else
const uint16_t internal_version = VERSION_INTERNAL;
#endif


#ifndef BUILD_NAME
const char* const build_name = "Makerbot";
#else
const char* const build_name = BUILD_NAME;
#endif

#endif // VERSION_HH_
