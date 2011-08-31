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

#ifndef TOOL_HH_
#define TOOL_HH_

#include "Packet.hh"

// TODO: Make this into a class.

/// There are three fundamental ways to initiate a tool interaction.  They are:
/// <ul>
/// <li> Passthrough queries.  These are initiated by the host.  The controller passes
///      the query along to the tool, and the response is copied back to the host.</li>
/// <li> Controller queries.  These are initiated by the controller.  They generally
///      are not directly returned to the host.</li>
/// <li> Queued commands.  The responses from these queries are generally discarded.
///     At this time, the command thread blocks until these commands return.</li>
/// </ul>
///
/// Because the tool can only process one transaction at a time, and we don't
/// want to block on tool transactions, the tool lock must be acquired before using
/// the tool interface. Once the lock is acquired, it must be explicitly released by
/// the holder.
namespace tool {

/// Run the tool maintenance timeslice.  Checks for tool command timeouts, etc.
void runToolSlice();

/// Get the tool interaction lock.
/// \return True if the lock has been successfully acquired, false otherwise.
bool getLock();

/// Release the tool interaction lock, letting some other part of the system
/// interact with the toolheads.
void releaseLock();

/// Start a transaction.  Assumes that the caller has acquired the lock and
/// filled the output packet.
void startTransaction();

/// Check if the transaction is completed.
/// \return True if the transaction is complete.
bool isTransactionDone();

/// Get the output packet
/// \return Reference to the output packet, which the host should fill with a query
///         to send to the tool.
OutPacket& getOutPacket();

/// Get the input packet
/// \return Reference to the input packet, which should contain a query respose from
///         the tool, which the host should then inspect.
InPacket& getInPacket();

/// Get the total number of packets that were attempted to be sent to a tool. This can
/// be used to assess communication quality with the tool.
/// \return Total number of packets sent over the rs485 line to the tool
uint32_t getSentPacketCount();

/// Get the total number of packets that failed to get a response from a tool. This can
/// be used to assess communication quality with the tool.
/// \return Total number of packets sent to the tool, that failed to generate a response.
uint32_t getPacketFailureCount();

/// Get the total packet retries attempted. The host will attempt to retry a packet 5 times
/// before giving up, so this number will increase by 5 for every failed packet.
/// \return Total number of packet retries attempted.
uint32_t getRetryCount();

/// Get the total number of received bytes that were discarded as noise.
/// \return Total number of noise bytes received.
uint32_t getNoiseByteCount();

/// Attempt to reset the tool by sending it a reset packet
/// \return True if the extruder responded to the reset request.
bool reset();

/// Attempt to test the tool by sending it a series of pings
/// \return True if the test succeeded; false otherwise.
bool test();

/// Set the toolhead index
/// \param index Index (0-127) of the toolhead to communicate with
void setCurrentToolheadIndex(uint8_t index);

/// Get the current toolhead index
/// \return Index of the current toolhead.
uint8_t getCurrentToolheadIndex();

}

#endif // TOOL_HH_
