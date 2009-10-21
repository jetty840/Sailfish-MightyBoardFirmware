#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdint.h>

//this is the version of our host software
extern unsigned int host_version;

//are we paused?
extern bool is_machine_paused;
extern bool is_machine_aborted;

//how many queued commands have we processed?
//this will be used to keep track of our current progress.
extern unsigned long finishedPoints;
extern uint8_t commandMode;

#endif // VARIABLES_H
