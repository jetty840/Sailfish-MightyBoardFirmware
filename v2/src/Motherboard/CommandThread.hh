/*
 * CommandThread.hh
 *
 *  Created on: Dec 28, 2009
 *      Author: phooky
 */

#ifndef COMMANDTHREAD_HH_
#define COMMANDTHREAD_HH_

/// We're moving to a fixed-length interval.  This gives us approximately 4000
/// instructions per interval, and ~4000 steps/second.
#define INTERVAL_IN_MICROSECONDS 512

/// The command thread is driven by the timer1 interrupt and occurs every 32 microseconds.
/// It is responsible for driving the steppers and processing new commands from the head
/// of the command queue.
void startCommandThread();
void stopCommandThread();
bool isCommandThreadPaused();
void pauseCommandThread(bool paused = true);

#endif /* COMMANDTHREAD_HH_ */
