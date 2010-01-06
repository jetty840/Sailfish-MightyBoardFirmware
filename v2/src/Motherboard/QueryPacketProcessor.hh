/*
 * QueryPacketProcessor.hh
 *
 *  Created on: Dec 9, 2009
 *      Author: phooky
 */

#ifndef UTIL_QUERYPACKETPROCESSOR_HH_
#define UTIL_QUERYPACKETPROCESSOR_HH_

#include "Packet.hh"

/// Identify a query packet, and process it.  If the packet is a recognized
/// command, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processQueryPacket(const InPacket& from_host, OutPacket& to_host);

#endif // UTIL_QUERYPACKETPROCESSOR_HH_
