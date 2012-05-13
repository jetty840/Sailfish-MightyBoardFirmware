#ifndef PROTOCOLDOCUMENTATION_HH
#define PROTOCOLDOCUMENTATION_HH

/// \addtogroup Commands Serial command protocol
///
/// This document describes the protocol by which 3rd and 4th generation RepRap
/// electronics communicate with their host machine, as well as the protocol
/// by which the RepRap host communicates with its subsystems.  The same simple
/// packet protocol is used for both purposes.
///
/// <h1>Where to find implementations of this protocol</h1>
/// <h2>Firmware</h2>
/// The default firmware for the default generation 3 electronics can be found on GitHub at:
/// http://github.com/makerbot/G3Firmware.
///
/// <h2>Host software</h2>
/// Currently the only software suite implementing this protocol is ReplicatorG.  It can be found on GitHub at:
/// http://github.com/makerbot/ReplicatorG.
///
/// <h1>Network Overview</h1>
/// The RepRap Generation 3 electronics set consists of several hardware components:
/// -# A single <b>Master Controller</b> which controls the 3-axis steppers, communicates with a host PC, supports a storage card and controls a set of toolheads.
/// -# A set of <b>Stepper Drivers</b> which drive the steppers based on signals from the master controller.  The communications between the drivers and the master controller is outside of the scope of this document.
/// -# A number of <b>Tool Controllers</b> which control extruders, cutters, frostruders, etc.
///
/// The two communications channels covered by this document are:
/// -# The <b>Host Network</b>, between a host computer and the master controller
/// -# The <b>Slave Network</b>, between the master controller and one or more tool controllers.
///
///
/// The host network is generally implemented over a standard TTL-level RS232 serial connection.  The slave network is implemented as an RS485 serial bus driven by SN75176A transceivers.
///
/// <h1>Packet Protocol</h1>
/// <h2>Protocol Overview</h2>
/// Each network has a single master: in the case of the host network, this is the host computer, and in the case of the slave network, this is the master controller.  All network communications are initiated by the network master; a slave node can never initiate a data transfer.
///
/// Data is sent over the network as a series of simple packets.  Packets are variable-length, with a maximum payload size of 32 bytes.
///
/// Each network transaction consists of at least two packets: a master packet, followed by a response packet.  Every packet from a master must be responded to.
///
/// Commands will be sent in packets. All commands are query/response.  The master in each pair will always initiate communications, never the slave.  All packets are synchronous; they will wait for a response from the client before sending the next packet.  The firmware will continue rendering buffered commands while receiving new commands and replying to them.
///
/// <h2>Timeouts</h2>
/// Packets must be responded to promptly.  No command should ever block.  If a query would require more than the timeout period to respond to, it must be recast as a poll-driven operation.
///
/// All communications, both host-mb and mb-toolboard, are at 38400bps.  It should take approximately 1/3rd ms. to transmit one byte at those speeds.  The maximum
/// packet size of 32+3 bytes should take no more than 12ms to transmit.  We establish a 20ms. window from the reception of a start byte until packet completion.  If a packet is not completed within this window, it is considered to have timed out.
///
/// It is expected that there will be a lag between the completion of a command packet and the beginning of a response packet.  This may include a round-trip request to a toolhead, for example.  This window is expected to be 36ms. at the most.  Again, if the first byte of the response packet is not received by the time 36ms. has passed, the packet is presumed to have timed out.
/// (missing image)
/// <h2>Handling Packet Failures</h2>
/// If a packet has timed out, the host or board should treat the entire packet transaction as void.  It should:
/// - Return its packet reception state machine to a ready state.
/// - Presume that no action has been taken on the transaction
/// - Attempt to resend the packet, if it was a host packet.
///
/// <h2>Command Buffering</h2>
/// To ensure smooth motion, as well as to support print queueing, we'll want certain commands to be queued in a buffer.  This means we won't get immediate feedback from any queued command.  To this end we will break commands down into two categories: action commands that are put in the command buffer, and query commands that require an immediate response.  In order to make it simple to differentiate the commands on the firmware side, we will break them up into two sets: commands numbered 0-127 will be query commands, and commands numbered 128-255 will be action commands to be put into the buffer.  The firmware can then simply look at the highest bit to determine which type of packet it is.
///
/// Every packet gets precisely one response packet.  Query commands must be sent in their own packet.  Only action or query commands can be sent in a single packet.  The first byte of the command payload will determine the nature of the entire packet.  Thus, you cannot mix query and action commands in a single packet!
///
/// <h2>Command Types</h2>
/// <table>
///  <tr>
///   <th>Range</th>
///   <th>Type</th>
///   <th>Description</th>
///  </tr>
///  <tr>
///   <td>0-127</td>
///   <td>Query Commands</td>
///   <td>Commands that can be executed immediately, and thus do not need to be queued.  Generally return pre-calculated information and should be fast.</td>
///  </tr>
///  <tr>
///   <td>128-255</td>
///   <td>Action Commands</td>
///   <td>Commands that must be executed one at a time, and in a particular order.  These commands should be buffered for smooth operation.  Many of there commands may take up a large amount of time to complete.</td>
///  </tr>
/// </table>
///
/// <h2>Packet structure</h2>
/// Command packets are sent from the Master to the Slave.  This can either be from the Host CPU to the Master uC or from the Master uC to the Slave uC.  It consists of a two-byte header, a variable-length payload, and a 1 byte CRC footer.
///
/// The packet consists of:
///
/// <table>
///  <tr>
///   <th>Index</th>
///   <th>Name</th>
///   <th>Details</th>
///  </tr>
///  <tr>
///   <td>0</td>
///   <td>Start byte</td>
///   <td>This byte always has the value 0xD5 and is used to ensure synchronization.</td>
///  </tr>
///  <tr>
///   <td>1</td>
///   <td>Length</td>
///   <td>This byte indicates the length of the payload (excluding the header and CRC) in bytes.</td>
///  </tr>
///  <tr>
///   <td>2+</td>
///   <td>Payload</td>
///   <td>The packet payload.  This is appended to the buffer and either immediately executed and removed (if a query type) or buffered for execution (if an action type).</td>
///  </tr>
///  <tr>
///   <td>Length+2</td>
///   <td>CRC</td>
///   <td>This is the 8-bit iButton/Maxim CRC of the payload.</td>
///  </tr>
/// </table>
/// Command length is implicit in the command structure; no explicit separator is needed.
///
/// <h2>Command structure</h2>
///
/// <h3>Host Commands</h3>
/// The payload of a packet consists of one command.  Each command contains a header, as follows:
///
/// <table>
///  <tr>
///   <th>Index</th>
///   <th>Name</th>
///   <th>Details</th>
///  </tr>
///  <tr>
///   <td>0</td>
///   <td>Command</td>
///   <td>The command code to send to the device.</td>
///  </tr>
///  <tr>
///   <td>1-N</td>
///   <td>Arguments</td>
///   <td>Arguments to this command (details below as 'command payload').</td>
///  </tr>
/// </table>
///
/// <h3>Slave Commands</h3>
/// The payload of a packet consists of one command.  Each command contains a header, as follows:
///
/// <table>
///  <tr>
///   <th>Index</th>
///   <th>Name</th>
///   <th>Details</th>
///  </tr>
///  <tr>
///   <td>0</td>
///   <td>Slave ID</td>
///   <td>The ID of the slave device being addressed.  The value 127 represents any available device; see below.</td>
///  </tr>
///  <tr>
///   <td>1</td>
///   <td>Command</td>
///   <td>The command code to send to the device.</td>
///  </tr>
///  <tr>
///   <td>2-N</td>
///   <td>Arguments</td>
///   <td>Arguments to this command (details below as 'command payload').</td>
///  </tr>
/// </table>
///
/// <h2>Slave IDs</h2>
/// The slave ID is the ID number of a toolhead.  A toolhead may only respond to commands that are directed at its ID.  If the packet is corrupt, the slave should *not* respond with an error message to avoid collisions. Valid Slave IDs are 0 - 126.
///
/// The exception to this is the slave ID 127.  This represents any listening device.  The address 127 should only be used when setting the ID of a slave.
///
/// <h2>Response Packets</h2>
/// Response packets look just like command packets.  The only difference is the payload is guaranteed to contain a response code as the first byte, as described below.  The only exception is certain debugging packets, which will specifically indicate such in their description.
///
/// <h2>Response Packet Payload Structure</h2>
///
/// <table>
///  <tr>
///   <th>Index</th>
///   <th>Name</th>
///   <th>Details</th>
///  </tr>
///  <tr>
///   <td>0</td>
///   <td>Response Code</td>
///   <td>This is a standard response code that is included with all packets and is described below.</td>
///  </tr>
///  <tr>
///   <td>1-N</td>
///   <td>Response Data</td>
///   <td>This contains response-specific data.  Length is implicit based on the command being responded-to.</td>
///  </tr>
/// </table>
///
/// If the command was a non-request command, the response data is always empty.
///
/// <h2>Response Code Values</h2>
/// Note: These are changed starting in version 2.92. Previously, they were 0x0 - 0x7.
/// <table>
///  <tr>
///   <th>Response Code</th>
///   <th>Interpretation</th>
///  </tr>
///  <tr>
///   <td>0x80</td>
///   <td>Generic error, packet discarded.</td>
///  </tr>
///  <tr>
///   <td>0x81</td>
///   <td>Success.</td>
///  </tr>
///  <tr>
///   <td>0x82</td>
///   <td>Action buffer overflow, entire packet discarded.</td>
///  </tr>
///  <tr>
///   <td>0x83</td>
///   <td>CRC mismatch, packet discarded.</td>
///  </tr>
///  <tr>
///   <td>0x84</td>
///   <td>Query packet too big, packet discarded.</td>
///  </tr>
///  <tr>
///   <td>0x85</td>
///   <td>Command not supported/recognized.</td>
///  </tr>
///  <tr>
///   <td>0x86</td>
///   <td>Success; expect more packets.  Used when a single reponse packet cannot contain the entire message to be retrieved.</td>
///  </tr>
///  <tr>
///   <td>0x87</td>
///   <td>Downstream timeout (for example, a toolhead timed out).</td>
///  </tr>
/// </table>
///
/// <h2>Data Formats</h2>
/// The protocol is a byte-oriented protocol.  Payloads may contain various information in a variety of formats, but will generally be limited to the formats described below.  Multi-byte datatypes will always be transmitted in Little-endian mode.  Remember, this means that the least significant byte ("littlest") byte is sent first!  For a more in-depth discussionof little-endian storage, see: http://en.wikipedia.org/wiki/Endianness#Little-endian
///
/// <h2>Various Data Types</h2>
/// <table>
///  <tr>
///   <th>Type</th>
///   <th>Range</th>
///  </tr>
///  <tr>
///   <td>uint8</td>
///   <td>0 to 255</td>
///  </tr>
///  <tr>
///   <td>uint16</td>
///   <td>0 to 65,535</td>
///  </tr>
///  <tr>
///   <td>int16</td>
///   <td>-32767 to 32,767</td>
///  </tr>
///  <tr>
///   <td>uint32</td>
///   <td>0 to 4,294,967,295</td>
///  </tr>
///  <tr>
///   <td>int32</td>
///   <td>-2,147,483,647 to 2,147,483,647</td>
///  </tr>
/// </table>
///
/// <h2>Test Commands</h2>
/// The command codes of the form 0xFX and 0x7X are reserved for diagnostic test packets.
/// The firmware is not guaranteed to implement any of these operations.
///
/// (section omitted)
///


/// \mainpage
/// <h2>Introduction</h2>
/// The G3 firmware suite is machine control firmware for MakerBot and similar 3d printers.
/// The firmware is basically a simple RTOS, which uses a round-robin scheduler to service
/// all of the different modules that are on a board (using their runSlice() routine). In
/// addition to this scheduler, boards also implement a high-frequency, interrupt driven
/// control loop for high priority tasks ( ExtruderBoard::doInterrupt() or
/// Motherboard::doInterrupt() ). Finally, some hardware accelerators (such as the USART
/// serial hardware) also use interrupts for control. There aren't any hard and fast rules
/// for how much time any particular task should take, other than that it shouldn't
/// interfere with printing (which means that the host serial buffer should stay full during
/// a print, and the StepperInterface shouldn't be starved for new movement commands). Any
/// help defining this more precisely would be welcome.
///
/// <h2>Libraries</h2>
/// Support for different control systems/hardware are implemented as C++ classes. There
/// are two main kinds- #SoftwareLibraries, which don't rely on any particular hardware
/// peripherals, and #HardwareLibraries, which require that the microcontroller implements
/// a specific feature (such as a timer or UART). Libraries should not use singletons or
/// static members if possible, to keep the build system simple to maintain (instead of
/// conditionally including some object files, all can be included, and the linker can
/// figure out which parts to include). Instead, instances of the library class should be
/// included in the board class (ExtruderBoard or Motherboard), and instantiated from there.
///
/// Libraries should have these entry points (note that current nomenclature is not consistant):
/// <table>
///  <tr>
///   <th>Function name</th>
///   <th>Description</th>
///  </tr>
///  <tr>
///   <td>init()</td>
///   <td>Place setup code here instead of in the class constructor, so that the module can completely re-initialize itself during a soft reset.</td>
///  </tr>
///  <tr>
///   <td>runSlice()</td>
///   <td>Non-realtime maintanence code, anything which needs to happen but may take a while to run (ie, updating an LCD screen)</td>
///  </tr>
///  <tr>
///   <td>doInterrupt()</td>
///   <td>Interrupt code, anything that must happen often but cannot take much processor time (ie, scanning a button array)</td>
///  </tr>
/// </table>
///
///
/// <h2>Supported Hardware</h2>
///
/// <table>
///  <tr>
///   <th>Electronics version</th>
///   <th>Motherboard name</th>
///   <th>Extruder name</th>
///   <th>Used in</th>
///  </tr>
///  <tr>
///   <td>Generation 3</td>
///   <td>RepRap Motherboard v1.2</td>
///   <td>Extruder Controller v2.2</td>
///   <td>MakerBot Cupcake</td>
///  </tr>
///  <tr>
///   <td>Generation 4</td>
///   <td>Motherboard v2.4+</td>
///   <td>Extruder Controller v3.4</td>
///   <td>Makerbot Thing-O-Matic</td>
///  </tr>
///  <tr>
///   <td>MightyBoard</td>
///   <td>MightyBoard</td>
///   <td>Embedded MightyBoard Extruder</td>
///   <td>Makerbot Replicator</td>
///  </tr>
//// </table>


/// <table>
///  <tr>
///   <th>Index</th>
///   <th>Name</th>
///   <th>Details</th>
///  </tr>
///  <tr>
///   <td></td>
///   <td></td>
///   <td></td>
///  </tr>
///  <tr>
///   <td></td>
///   <td></td>
///   <td></td>
///  </tr>
///  <tr>
///   <td></td>
///   <td></td>
///   <td></td>
///  </tr>
/// </table>


#endif // PROTOCOLDOCUMENTATION_HH
