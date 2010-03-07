/*
 * ExtruderMotor.hh
 *
 *  Created on: Mar 7, 2010
 *      Author: phooky
 */

#ifndef EXTRUDERMOTOR_HH_
#define EXTRUDERMOTOR_HH_

#include <stdint.h>

void initExtruderMotor();

// 0 = stop
// + = forward direction
// - = negative direction
// Valid range: -255 through 255
void setExtruderMotor(int16_t speed);

#endif /* EXTRUDERMOTOR_HH_ */
