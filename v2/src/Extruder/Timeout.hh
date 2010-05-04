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

#ifndef TIMEOUT_HH_
#define TIMEOUT_HH_

#include <stdint.h>
#include "Types.hh"

/// Timeout objects maintain timestamps and check the universal clock to figure out when they've
/// elapsed.  Resolution is at best that of the system interval.  Maximum timeout length is
/// 2147483648 microseconds.
/// Timeouts much be checked every 1073741824 microseconds to remain valid.
/// After a timeout has elapsed, it can not go back to a valid state without being explicitly reset.
class Timeout {
private:
	bool active;
	bool elapsed;
	micros_t start_stamp_micros;
	micros_t duration_micros;
public:
	Timeout();
	void start(micros_t duration_micros);
	bool hasElapsed();
	bool isActive() const { return active; }
	void abort();
};

#endif // TIMEOUT_HH_
