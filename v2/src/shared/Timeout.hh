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

/// The timeout object is a simple 1-shot timer, which can be used to schedule a low-frequency
/// event such as polling a temperature sensor. To start a timeout, call the #start() function with
/// the minimum number of microseconds to wait before
///
/// Timeout objects maintain timestamps and check the universal clock to figure out when they've
/// elapsed.  Resolution is at best that of the system interval.  Maximum timeout length is
/// 2147483648 microseconds.
/// Timeouts much be checked every 1073741824 microseconds to remain valid.
/// After a timeout has elapsed, it can not go back to a valid state without being explicitly reset.
/// \ingroup SoftwareLibraries
class Timeout {
private:
        bool active;                    ///< True if the timeout object is actively counting down.
        bool elapsed;                   ///< True if the timeout object has elapsed.

        //TODO: Instead of storing start and duration, precompute and store the elapse time.
	micros_t start_stamp_micros;
	micros_t duration_micros;
public:
        /// Instantiate a new timeout object.
	Timeout();

        /// Start a new timeout cycle that will elapse after the given amount of time.
        /// \param [in] duration_micros Microseconds until the timeout cycle should elapse.
	void start(micros_t duration_micros);

        /// Test whether the current timeout cycle has elapsed. When called, this function will
        /// compare the system time to the calculated time that the timeout should expire, and
        /// if it has, the timer is marked as elapsed and not active.
        /// \return True if the timeout has elapsed.
	bool hasElapsed();

        ///
        /// \return True if the timeout is still running.
	bool isActive() const { return active; }

        /// Stop the current timeout.
	void abort();
};

#endif // TIMEOUT_HH_
