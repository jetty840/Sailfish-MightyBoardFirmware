/**
 * TrueRandom - A true random number generator for Arduino.
 *
 * Copyright (c) 2010 Peter Knight, Tinker.it! All rights reserved.
 * 
 * Edited to use the ATMega1280 and allow for any analog Pin: alison@makerbot.com
 * 
 */

#include <avr/io.h>
#include "TrueRandom.hh"

TrueRandom::TrueRandom(uint8_t pin){

  // Analog pins are on ports F and K
	if (pin < 8) {
			DDRF &= ~(_BV(pin));
			PORTF &= ~(_BV(pin));
			// clear ADC Channel bit selecting upper 8 ADCs
			ADCSRB &= ~0b01000;
	}
	else{
		pin -= 8;
		DDRK &= ~(_BV(pin));
		PORTK &= ~(_BV(pin));
		// set ADC Channel bit selecting upper 8 ADCs
		ADCSRB |= 0b01000;
	}
	
	analog_pin = pin;
}

int16_t TrueRandom::randomBitRaw(void) {
  uint8_t copyAdmux, copyAdcsra, copyAdcsrb, copyPort, copyDdr;
  uint16_t i;
  uint8_t bit;
  volatile uint8_t dummy;
  
  // Store all the registers we'll be playing with
  copyAdmux = ADMUX;
  copyAdcsra = ADCSRA;
  copyAdcsrb = ADCSRB;
  
  /// we're writing out the port changes using if statments instead of using a pointer the port
  /// because using pointers will make the port operations much slower, which will destroy the functionality 
  /// of this library, as it relies on fast port manipulation
  if(analog_pin  < 8){
	  copyPort = PORTF;
	  copyDdr = DDRF;
  }else{
	  copyPort = PORTK;
	  copyDdr = DDRK;		
  }

  // select ADC Channel and connect AREF to AVCC
  ADMUX = 0b01000000 + analog_pin;
  
#if F_CPU > 16000000
  // ADC is enabled, divide by 32 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);
#elif F_CPU > 8000000
  // ADC is enabled, divide by 16 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS2);
#else
  // ADC is enabled, divide by 8 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
#endif

  // Autotriggering disabled
  ADCSRB = 0;

 if(analog_pin  < 8){
	  // Pull AnalogPin to ground
	  PORTF &=~_BV(analog_pin);
	  DDRF |= _BV(analog_pin);
	  // Release AnalogPin, apply internal pullup
	  DDRF &= ~_BV(analog_pin);
	  PORTF |= _BV(analog_pin);
	  // Immediately start a sample conversion on Analog0
	  ADCSRA |= _BV(ADSC);
	  // Wait for conversion to complete
	  while (ADCSRA & _BV(ADSC)) PORTF ^= _BV(analog_pin);
  } else{
  	 // Pull AnalogPin to ground
	  PORTK &=~_BV(analog_pin);
	  DDRK |= _BV(analog_pin);
	  // Release AnalogPin, apply internal pullup
	  DDRK &= ~_BV(analog_pin);
	  PORTK |= _BV(analog_pin);
	  // Immediately start a sample conversion on Analog0
	  ADCSRA |= _BV(ADSC);
	  // Wait for conversion to complete
	  while (ADCSRA & _BV(ADSC)) PORTK ^= _BV(analog_pin);
  }
  
  // Xor least significant bits together
  bit = ADCL;
  // We're ignoring the high bits, but we have to read them before the next conversion
  dummy = ADCH;

  // Restore register states
  ADMUX = copyAdmux;
  ADCSRA = copyAdcsra;
  ADCSRB = copyAdcsrb;
 
 if(analog_pin < 8){
  PORTF = copyPort;
  DDRF = copyDdr;
 }else{
  PORTK = copyPort;
  DDRK = copyDdr;
 }

  return bit & 1;
}

int16_t TrueRandom::randomBitRaw2(void) {
  // Software whiten bits using Von Neumann algorithm
  //
  // von Neumann, John (1951). "Various techniques used in connection
  // with random digits". National Bureau of Standards Applied Math Series
  // 12:36.
  //
  for(;;) {
    int16_t a = randomBitRaw() | (randomBitRaw()<<1);
    if (a==1) return 0; // 1 to 0 transition: log a zero bit
    if (a==2) return 1; // 0 to 1 transition: log a one bit
    // For other cases, try again.
  }
}

int16_t TrueRandom::randomBit(void) {
  // Software whiten bits using Von Neumann algorithm
  //
  // von Neumann, John (1951). "Various techniques used in connection
  // with random digits". National Bureau of Standards Applied Math Series
  // 12:36.
  //
  for(;;) {
    int16_t a = randomBitRaw2() | (randomBitRaw2()<<1);
    if (a==1) return 0; // 1 to 0 transition: log a zero bit
    if (a==2) return 1; // 0 to 1 transition: log a one bit
    // For other cases, try again.
  }
}

uint8_t TrueRandom::randomByte(void) {
  uint8_t result;
  uint8_t i;
  result = 0;
  for (i=8; i--;) result += result + randomBit();
  return result;
}

int16_t TrueRandom::rand() {
  int16_t result;
  uint8_t i;
  result = 0;
  for (i=15; i--;) result += result + randomBit();
  return result;
}

int32_t TrueRandom::random() {
  int32_t result;
  uint8_t i;
  result = 0;
  for (i=31; i--;) result += result + randomBit();
  return result;
}

int32_t TrueRandom::random(int32_t howBig) {
  int32_t randomValue;
  int32_t maxRandomValue;
  int32_t topBit;
  int32_t bitPosition;
  
  if (!howBig) return 0;
  randomValue = 0;
  if (howBig & (howBig-1)) {
    // Range is not a power of 2 - use slow method
    topBit = howBig-1;
    topBit |= topBit>>1;
    topBit |= topBit>>2;
    topBit |= topBit>>4;
    topBit |= topBit>>8;
    topBit |= topBit>>16;
    topBit = (topBit+1) >> 1;

    bitPosition = topBit;
    do {
      // Generate the next bit of the result
      if (randomBit()) randomValue |= bitPosition;

      // Check if bit 
      if (randomValue >= howBig) {
        // Number is over the top limit - start again.
        randomValue = 0;
        bitPosition = topBit;
      } else {
        // Repeat for next bit
        bitPosition >>= 1;
      }
    } while (bitPosition);
  } else {
    // Special case, howBig is a power of 2
    bitPosition = howBig >> 1;
    while (bitPosition) {
      if (randomBit()) randomValue |= bitPosition;
      bitPosition >>= 1;
    }
  }
  return randomValue;
}

int32_t TrueRandom::random(int32_t howSmall, int32_t howBig) {
  if (howSmall >= howBig) return howSmall;
  int32_t diff = howBig - howSmall;
  return TrueRandom::random(diff) + howSmall;
}

void TrueRandom::memfill(uint8_t* location, int16_t size) {
  for (;size--;) *location++ = randomByte();
}

void TrueRandom::mac(uint8_t* macLocation) {
  memfill((uint8_t*)macLocation,6);
}

void TrueRandom::uuid(uint8_t* uuidLocation) {
  // Generate a Version 4 UUID according to RFC4122
  memfill((uint8_t*)uuidLocation,16);
  // Although the UUID contains 128 bits, only 122 of those are random.
  // The other 6 bits are fixed, to indicate a version number.
  uuidLocation[6] = 0x40 | (0x0F & uuidLocation[6]); 
  uuidLocation[8] = 0x80 | (0x3F & uuidLocation[8]);
}

