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

}

#endif // TOOL_HH_
