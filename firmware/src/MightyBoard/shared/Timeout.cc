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

#include "Timeout.hh"
#include "Configuration.hh"
#include "Motherboard.hh"

// MBI used a technique which handled wrap around of the clock timer
// but required a minimum of two uint32_t values per Timeout object.
// Since there's a lot of Timeout objets, a significant amount of
// SRAM can be saved by using a different technique.
//
// MBI's technique was
//
//   start(duration):
//     saved_clock = current_clock
//     saved_duration = duration
//
//   check_timeout:
//     delta = current_clock - saved_clock
//     if ( delta >= saved_duration ) timer-has-timed-out
//
// With the above technique two values are stored and the edge case of
//    1. current_clock = 0xffff fff0
//    2. duration = 0x08
//    3. current_clock wraps to 0x0000 nnnn
//    4. check still works as values wrap appropriately when doing the math
//
// New technique
//
//   start(duration):
//      timeout = current_clock + duration
//
//   check_timeout:
//     if ( timeout <= current_clock ) timer-has-timed-out
//
// The above will fail in the above cited edge case.
// There's two solutions
//   A. Account for clock wraps, or
//   B. Make the getCentaMicros() timer take a long time to time out.
//
// As regards A, MBI was counting microseconds with a 100 microsecond
// resolution.  Their counter would overflow in 71 minutes and 34
// seconds.  They kept their timeout values in microseconds as well.
// A better approach is to keep the counter in units of 100 microseconds
// and convert all timeouts from microseconds to hundreds of microseconds.
// Then the counter won't overflow for 100 * (71.58 seconds) or 119.3 hours
//
// As regards B, we can do that as well with an 8bit wrap counter.  That
// extends us out a further factor of 255 to 3.47 years!
// 

Timeout::Timeout() : flags(0) { }

void Timeout::start(micros_t duration_micros_in) {
     flags = TIMEOUT_FLAGS_ACTIVE;
     end_time_micros = (duration_micros_in / 100) + Motherboard::getBoard().getCurrentCentaMicros(&my_wrap);
}

bool Timeout::hasElapsed() {
     if ( flags == TIMEOUT_FLAGS_ACTIVE ) {
	  uint8_t wrap;
	  if ( ( end_time_micros <= Motherboard::getBoard().getCurrentCentaMicros(&wrap) ) ||
	       ( my_wrap < wrap ) )
	       flags = TIMEOUT_FLAGS_ELAPSED;
     }
     return 0 != (flags & TIMEOUT_FLAGS_ELAPSED);
}
