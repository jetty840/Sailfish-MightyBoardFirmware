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

#ifndef UART_HH_
#define UART_HH_

#include "Packet.hh"
#include "Configuration.hh"
#include <stdint.h>

// TODO: Move to UART class
/// Communication mode selection
enum communication_mode {
    RS232,          ///< Act as an asynchronous, full duplex RS232 transciever
    RS485           ///< Act as an asynchronous, half duplex RS485 transciever
};


/// The UART object implements the serial communication protocol using one
/// or two hardware UART libraries.
///
/// UARTs, when constructed, start off disabled.
/// They begin receiving data only after an enable(true)
/// call is made.  beginSend() calls will send completed
/// packets.
///
/// Porting notes:
/// The current implementation supports one UART on the atmega168/328, and two UARTs
/// on the atmega644 and atmega1280/2560. The code will need to be updated to support
/// new architectures.
/// \ingroup HardwareLibraries
class UART {
private:
    static UART hostUART;       ///< The controller accepts commands from the host UART

#if HAS_SLAVE_UART
    static UART slaveUART;      ///< The controller can forward commands to the slave UART
#endif

public:
    /// Get a reference to the host UART
    /// \return hostUART instance, which should act as a slave to a computer (or motherboard)
    static UART& getHostUART() { return hostUART; }

#if HAS_SLAVE_UART
    /// Get a reference to the slave UART
    /// \return slaveUART instance, which should act as a master to one or more slave toolheads.
    static UART& getSlaveUART() { return slaveUART; }
#endif

private:
        /// Create an instance of the given UART controller
        /// \param[in] index hardware index of the UART to initialize
        /// \param[in] mode Either #RS232 or #RS485.
        UART(uint8_t index, communication_mode mode);

        /// Initialize the serial configuration. Must be called once at boot.
        void init_serial();

        /// Send a byte of data over the serial line.
        /// \param[in] data Data byte to send
        inline void send_byte(char data);

        const communication_mode mode_;     ///< Communication mode we are speaking
        const uint8_t index_;               ///< Hardware UART index
        volatile bool enabled_;             ///< True if the hardware is currently enabled

public:
        InPacket in;                        ///< Input packet
        OutPacket out;                      ///< Output packet

        /// Begin sending the data located in the #out packet.
        void beginSend();

        /// Enable or disable the serial port.
        /// \param[in] true to enable the serial port, false to disable it.
	void enable(bool enabled);

        /// Reset the UART to a listening state.  This is important for
        /// RS485-based comms.
        void reset();
};

#endif // UART_HH_
