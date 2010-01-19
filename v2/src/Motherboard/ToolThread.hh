#ifndef TOOL_THREAD_HH_
#define TOOL_THREAD_HH_

#include "Packet.hh"

void runToolSlice();

// Queue a tool transaction.  The in packet will be reset, and the out packet
// should already be filled with data for the toolhead.
void queueToolTransaction(OutPacket* out, InPacket* in);

#endif // TOOL_THREAD_HH_
