#include "StepperAxis.hh"

StepperAxis::StepperAxis() :
    interface(0) {
}

StepperAxis::StepperAxis(StepperInterface& stepper_interface) :
        interface(&stepper_interface) {
        reset();
}

void StepperAxis::setTarget(const int32_t target_in,
                            bool relative) {
        target = target_in;
        if (relative) {
                delta = target;
        } else {
                delta = target - position;
        }
        direction = true;
        if (delta != 0) {
                interface->setEnabled(true);
        }
        if (delta < 0) {
                delta = -delta;
                direction = false;
        }
}

void StepperAxis::setHoming(const bool direction_in) {
        direction = direction_in;
        interface->setEnabled(true);
        delta = 1;
}

void StepperAxis::definePosition(const int32_t position_in) {
        position = position_in;
}

void StepperAxis::enableStepper(bool enable) {
        interface->setEnabled(enable);
}

void StepperAxis::reset() {
        position = 0;
        minimum = 0;
        maximum = 0;
        target = 0;
        counter = 0;
        delta = 0;
#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
        endstop_play = ENDSTOP_DEFAULT_PLAY;
        endstop_status = ESS_UNKNOWN;
#endif //SINGLE_SWITCH_ENDSTOPS
}

bool StepperAxis::checkEndstop(const bool isHoming) {
#if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1)
  bool hit_endstop = direction ? interface->isAtMaximum() : interface->isAtMinimum();
  // We must move at least ENDSTOP_DEBOUNCE from where we hit the endstop before we declare traveling
  if (hit_endstop || ((endstop_play < ENDSTOP_DEFAULT_PLAY - ENDSTOP_DEBOUNCE) && endstop_status != ESS_TRAVELING)) {
    // Did we *just* hit the endstop?
    if (endstop_status == ESS_TRAVELING || (isHoming && endstop_status == ESS_UNKNOWN)) {
      endstop_play   = ENDSTOP_DEFAULT_PLAY;
      if (isHoming?direction:prev_direction)
	endstop_status = ESS_AT_MAXIMUM;
      else
	endstop_status = ESS_AT_MINIMUM;
      
      // OR, are we traveling away from the endstop we just hit and still have play...
    } else if ((direction && endstop_status != ESS_AT_MAXIMUM) || (!direction && endstop_status != ESS_AT_MINIMUM)) {
      if (endstop_play > 0) {
	--endstop_play;
	hit_endstop = false; // pretend this never happened...
      } else {
	// we ran out of play, so we must be ramming into the side, switch directions
	// endstop_status = !direction ? ESS_AT_MAXIMUM : ESS_AT_MINIMUM;
	// endstop_play   = ENDSTOP_DEFAULT_PLAY;
      }
    }
    // otherwise we hit the endstop
    
    // but if we didn't hit an endstop, clear the status
  } else {
    endstop_status = ESS_TRAVELING;
    if (!isHoming) {
      endstop_play   = ENDSTOP_DEFAULT_PLAY;
    }
  }
  prev_direction = direction;
  return hit_endstop;
#else
  return direction ? interface->isAtMaximum() : interface->isAtMinimum();
#endif
}

void StepperAxis::doInterrupt(const int32_t intervals) {
        counter += delta;
        if (counter >= 0) {
                interface->setDirection(direction);
                counter -= intervals;
                bool hit_endstop = checkEndstop(false);
                if (direction) {
                        if (!hit_endstop) interface->step(true);
                        position++;
                } else {
                        if (!hit_endstop) interface->step(true);
                        position--;
                }
                interface->step(false);
        }
}


bool StepperAxis::doHoming(const int32_t intervals) {
        if (delta == 0) return false;
        counter += delta;
        if (counter >= 0) {
                interface->setDirection(direction);
                counter -= intervals;
                bool hit_endstop = checkEndstop(true);
                if (direction) {
                        if (!hit_endstop) {
                                interface->step(true);
                        } else {
                                return false;
                        }
                        position++;
                } else {
                        if (!hit_endstop) {
                                interface->step(true);
                        } else {
                                return false;
                        }
                        position--;
                }
                interface->step(false);
        }
        return true;
}
