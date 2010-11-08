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

/***
 * There are three fundamental ways to initiate a tool interaction.  They are:
 * * Passthrough queries.  These are initiated by the host.  The controller passes
 *   the query along to the tool, and the response is copied back to the host.
 * * Controller queries.  These are initiated by the controller.  They generally
 *   are not directly returned to the host.
 * * Queued commands.  The responses from these queries are generally discarded.
 *   At this time, the command thread blocks until these commands return.
 *
 * Because the tool can only process one transaction at a time, and we don't
 * want to block on tool transactions, we introduce the concept of a tool lock.
 * Once the lock is acquired, it must be explicitly released by the holder.
 *
 */
namespace tool {

/**
 * Run the tool maintenance timeslice.  Checks for tool command timeouts,
 * etc.
 */
void runToolSlice();

/**
 * Get the tool interaction lock.  Returns true if the lock has been successfully
 * acquired, false otherwise.
 */
bool getLock();
/**
 * Release the tool interaction lock, letting some other part of the system
 * interact with the toolheads.
 */
void releaseLock();

/**
 * Start a transaction.  Assumes that the caller has acquired the lock and
 * filled the output packet.
 */
void startTransaction();
/**
 * Check if the transaction is completed.
 */
bool isTransactionDone();

/**
 * Get the output packet (to the tool).
 */
OutPacket& getOutPacket();
/**
 * Get the input packet (from the tool).
 */
InPacket& getInPacket();

/**
 * Immediately reset the tool.  Returns true if
 * tool responded to reset; false otherwise.
 */
bool reset();

extern uint8_t tool_index;
}

#endif // TOOL_HH_
