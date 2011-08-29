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
}

void StepperAxis::doInterrupt(const int32_t intervals) {
        counter += delta;
        if (counter >= 0) {
                interface->setDirection(direction);
                counter -= intervals;
                if (direction) {
                        if (!interface->isAtMaximum()) interface->step(true);
                        position++;
                } else {
                        if (!interface->isAtMinimum()) interface->step(true);
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
                if (direction) {
                        if (!interface->isAtMaximum()) {
                                interface->step(true);
                        } else {
                                return false;
                        }
                        position++;
                } else {
                        if (!interface->isAtMinimum()) {
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
