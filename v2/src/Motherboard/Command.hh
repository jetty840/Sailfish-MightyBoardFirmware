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

#ifndef COMMAND_HH_
#define COMMAND_HH_

#include <stdint.h>

namespace command {

/**
 * Reset the entire command queue.  Clears out any remaining queued
 * commands.
 */
void reset();

/**
 * Run the command thread slice.
 */
void runCommandSlice();

/**
 * If the parameter is true, pause all further command processing
 * (without altering the queue).  If false, resume command processing.
 */
void pause(bool pause);

/**
 * Returns true if command processing is currently suspended.
 */
bool isPaused();

/**
 * Return the remaining space in the command buffer, in bytes.
 */
uint16_t getRemainingCapacity();

/**
 * Returns true if command queue is empty.
 */
bool isEmpty();

/**
 * Push a byte onto the command buffer.
 */
void push(uint8_t byte);

}

#endif // COMMAND_HH_
