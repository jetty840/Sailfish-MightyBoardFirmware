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

#ifndef ERRORS_HH_
#define ERRORS_HH_

///
/// Canonical definition of blink codes for error conditions.
///
/// Explicit definitions instead of an enum to make it easy
/// to lookup blink codes until we write the doc.

#define NO_ERROR					0
#define ERR_SLAVE_PACKET_TIMEOUT	1
#define ERR_SLAVE_LOCK_TIMEOUT		2
#define ERR_SLAVE_PACKET_MISC		3
#define ERR_HOST_PACKET_TIMEOUT		4
#define ERR_HOST_PACKET_MISC		5
#define ERR_WDT_TIMEOUT				6

#define ERR_HOST_TRUNCATED_CMD		8

#endif /* ERRORS_HH_ */
