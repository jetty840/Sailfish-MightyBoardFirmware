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
/// elapsed.  Resolution presently 100 microseconds
///
/// Timeouts must be checked before the maximum timeout length to remain valid
/// After a timeout has elapsed, it can not go back to a valid state without being explicitly reset.

#define TIMEOUT_FLAGS_ACTIVE  0x01
#define TIMEOUT_FLAGS_ELAPSED 0x02

class Timeout {

private:
        uint8_t flags;
        uint8_t my_wrap;
	micros_t end_time_micros;

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
        bool isActive() const { return 0 != ( flags & TIMEOUT_FLAGS_ACTIVE ); }

        /// Stop the current timeout.
        void abort() { flags &= ~( TIMEOUT_FLAGS_ACTIVE ); }

        /// Clear the timeout so it can be used again
        void clear() { flags &= ~( TIMEOUT_FLAGS_ELAPSED ); }
};

#endif // TIMEOUT_HH_
