/*
 * AnalogPin.hh
 *
 *  Created on: Mar 1, 2010
 *      Author: phooky
 */

#ifndef ANALOGPIN_HH_
#define ANALOGPIN_HH_

#include <stdint.h>

void initAnalogPins(uint8_t bitmask);

void startAnalogRead(uint8_t pin, volatile uint16_t* destination);


#endif /* ANALOGPIN_HH_ */
