/*
 * DebugPacketProcessor.hh
 *
 *  Created on: Dec 9, 2009
 *      Author: phooky
 */

#ifndef UTIL_DEBUGPACKETPROCESSOR_HH_
#define UTIL_DEBUGPACKETPROCESSOR_HH_

#include "Packet.hh"

/// Identify a debug packet, and process it.  If the packet is a debug
/// packet, return true, indicating that no further processing should
/// be done.  Otherwise, processing of this packet should drop through
/// to the next processing level.
bool processDebugPacket(const InPacket& from_host, OutPacket& to_host);


#endif // UTIL_DEBUGPACKETPROCESSOR_HH_
