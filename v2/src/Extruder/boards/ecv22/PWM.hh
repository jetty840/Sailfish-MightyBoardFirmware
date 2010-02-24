/*
 * PWM.hh
 *
 * Low-frequency manual 8-bit PWM.
 * The PWM cycle runs at ~100Hz.
 *
 *  Created on: Feb 23, 2010
 *      Author: phooky
 */

#ifndef PWM_HH_
#define PWM_HH_

#include "AvrPort.hh"

void startPWM();

// Setting a value of 0xff or 0 will remove the pin from the pwm table.
void setPWM(Pin pin, uint8_t value);

#endif /* PWM_HH_ */
