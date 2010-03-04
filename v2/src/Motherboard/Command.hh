/*
 * CommandThread.hh
 *
 *  Created on: Dec 28, 2009
 *      Author: phooky
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

#endif /* COMMAND_HH_ */
