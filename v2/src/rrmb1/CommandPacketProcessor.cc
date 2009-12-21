#include "CommandPacketProcessor.hh"
#include "CommandQueue.hh"

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) != 0) {
			// Is queueable command.
			// Is there room on the queue?
			const uint8_t command_length = from_host.getLength();
			if (command_buffer.getRemainingCapacity() >= command_length) {
				// Append command to buffer
				for (int i = 0; i < command_length; i++) {
					command_buffer.push(from_host.read8(i));
				}
				to_host.append8(RC_OK);
			} else {
				to_host.append8(RC_BUFFER_OVERFLOW);
			}
			return true;
		}
	}
	return false;
}
