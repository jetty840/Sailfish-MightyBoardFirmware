/**
 * TrueRandom - A true random number generator for Arduino.
 *
 * Copyright (c) 2010 Peter Knight, Tinker.it! All rights reserved.
 * 
 * Edited to use the ATMega1280 and allow for any analog Pin: alison@makerbot.com
 */

#ifndef TrueRandom_h
#define TrueRandom_h

#include <inttypes.h>
class TrueRandom
{
  public:
	TrueRandom(uint8_t pin);
    int16_t rand();
    int32_t random();
    int32_t random(int32_t howBig);
    int32_t random(int32_t howsmall, int32_t how);
    int16_t randomBit(void);
    uint8_t randomByte(void);
    void memfill(uint8_t* location, int16_t size);
    void mac(uint8_t* macLocation);
    void uuid(uint8_t* uuidLocation);
  private:
	uint8_t analog_pin;
    int16_t randomBitRaw(void);
    int16_t randomBitRaw2(void);
};
#endif
