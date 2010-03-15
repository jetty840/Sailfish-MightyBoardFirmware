#ifndef STEPPERS_HH_
#define STEPPERS_HH_

#include "Configuration.hh"
#include <stdint.h>
#include "AvrPort.hh"
#include "Command.hh"

#include "Types.hh"
#include "Motherboard.hh"

namespace steppers {

/// Initialize the stepper subsystem.
void init(Motherboard& motherboard);
/// Returns true if the stepper subsystem is running.  If the
/// stepper subsystem is idle, returns false.  Will return true
/// if the system is running but paused.
bool isRunning();
/// Abort the current motion and set the stepper subsystem to
/// the not-running state.
void abort();
/// Enable/disable the given axis.
void enableAxis(uint8_t which, bool enable);
/// Set current target
void setTarget(const Point& target, int32_t dda_interval);
/// Define current position as given point
void definePosition(const Point& position);
/// Handle interrupt.  Return true if still moving to target; false
/// if target has been reached.
bool doInterrupt();
/// Get current position
const Point getPosition();
};

#endif /* STEPPERS_HH_ */
