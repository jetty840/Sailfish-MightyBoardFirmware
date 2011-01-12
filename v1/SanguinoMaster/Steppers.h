#ifndef _STEPPERS_H
#define _STEPPERS_H

#include <stdint.h>
#include "Datatypes.h"

/// Initialize the stepper driver state.
void init_steppers();
void disable_steppers();
void enable_steppers(bool x, bool y, bool z);
void disable_steppers(bool x, bool y, bool z);
int8_t get_endstop_states();
void enable_needed_steppers();

void queue_absolute_point(long x, long y, long z, unsigned long micros);
/// Move the steppers until the minimum endstop is triggered on the
/// specified axes.  All axes are moved at the same rate.
/// \param find_x find the minimum position for the X axis
/// \param find_y find the minimum position for the Y axis
/// \param find_z find the minimum position for the Z axis
/// \param step_delay delay between steps in microseconds
/// \param timeout_seconds seconds to wait before giving up
void seek_minimums(bool find_x, bool find_y, bool find_z, unsigned long step_delay, unsigned int timeout_seconds);
/// Move the axis one step in towards the minimum.  If the min endstop is
/// triggered, back up until it is only triggered by a single step.
/// \return true if the minimum endstop is triggered.
void seek_maximums(bool find_x, bool find_y, bool find_z, unsigned long step_delay, unsigned int timeout_seconds);
bool is_point_buffer_empty();
bool point_buffer_has_room(uint8_t size);
bool at_target();
void wait_until_target_reached();

void set_position(const LongPoint& pos);
const LongPoint get_position();

void set_range(const LongPoint& range);
const LongPoint get_range();


void resume_stepping();
void pause_stepping();
 
#endif // _STEPPERS_H
