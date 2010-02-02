/*
 * CommandThread.hh
 *
 *  Created on: Dec 28, 2009
 *      Author: phooky
 */

#ifndef COMMANDTHREAD_HH_
#define COMMANDTHREAD_HH_

void resetCommands();

void runCommandSlice();

void pauseCommandThread(bool pause);

bool isCommandThreadPaused();

#endif /* COMMANDTHREAD_HH_ */
