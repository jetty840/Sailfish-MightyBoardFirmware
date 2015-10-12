#ifdef HAS_RGB_LED

/*
 * Copyright 2015 by Dan Newman <dan.newman@mtbaldy.us>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * The Azteeg X3 has the four suitable outputs -- outputs not at +5V --
 * on four different timers!
 *
 *   AVR Pin / Timer, Counter / Arduino Pin
 *   PG5     / TIMER0, OC0B   / D4
 *   PE3     / TIMER3, OC3A   / D5
 *   PH3     / TIMER4, OC4A   / D6
 *   PB5     / TIMER1, OC1A   / D11
 *
 * This makes it impractical to use hardware PWM to control the LEDs
 * since there simply are not three spare timers.   If three of the
 * four pins were on the same timer (e.g., TIMER3 or 4) then it would
 * be practical.  But as is, they are across different timers, some
 * of which need to be used for other functions *and* at different
 * frequencies.  So, we're stuck with doing software PWM with a spare
 * timer.  Unfortunately, the spare timer is also a high priority timer,
 * Timer 1.  So we need to be careful and not have it do too much work.
 */

#include "Compat.hh"
#include "RGB_LED.hh"
#include "Configuration.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

namespace RGB_LED {

static uint8_t pwm_counter;
static uint8_t pwm_r_top_value;
static uint8_t pwm_g_top_value;
static uint8_t pwm_b_top_value;

void init() {
     // Set digital I/O pins as outputs
     RGB_LED_DDR_R |= RGB_LED_MASK_R;
     RGB_LED_DDR_G |= RGB_LED_MASK_G;
     RGB_LED_DDR_B |= RGB_LED_MASK_B;

     // Set the default color
     setDefaultColor();
}

/*
 * For simplicity, we'd like to do
 *
 *   PIN.setValue(pwm_counter < counter_top_value)
 *
 * That keeps the LED off all the time when the counter_top_value is 0.
 * Thus there is no flickering of an LED which should be OFF.  However,
 * there is then flickering when the LED should be on 100%,
 *
 *   PIN.setValue([0,254] < 255) --> TRUE
 *   PIN.setValue(255 < 255) --> FALSE
 *
 * However, that slight flicker off should be okay and not perceptible.
 */

// We do not allow full 0 - 255 resolution for each LED.   Instead we cut
// it down by 2^(RGB_RES).  That allows us to use a low PWM frequency with
// the lights and have a brief on/off period.  If we used a low PWM frequency
// (e.g., 2 - 5 KHz) but also had 0 - 255 resolution, the blinking in many cases
// would be noticeable.  We can either reduce the resolution or increase the
// frequency.  But since we have to use an actual software interrupt, we want
// to keep the frequency down so as to not steal cycles from more useful
// things.

#define RGB_RES 4

ISR(TIMER1_COMPA_vect) {
     if (++pwm_counter > (0xFF >> RGB_RES))
	  pwm_counter = 0;

#if RGB_LED_INVERTED
     if ( pwm_counter < pwm_r_top_value) {
	  // LED on (electrical OFF)
	  RGB_LED_PORT_R &= ~(RGB_LED_MASK_R);
     }
     else {
	  // LED off (electrical ON)
	  RGB_LED_PORT_R |= RGB_LED_MASK_R;
     }

     if ( pwm_counter < pwm_g_top_value) {
	  // LED on (electrical OFF)
	  RGB_LED_PORT_G &= ~(RGB_LED_MASK_G);
     }
     else {
	  // LED off (electrical ON)
	  RGB_LED_PORT_G |= RGB_LED_MASK_G;
     }

     if ( pwm_counter < pwm_b_top_value) {
	  // LED on (electrical OFF)
	  RGB_LED_PORT_B &= ~(RGB_LED_MASK_B);
     }
     else {
	  // LED off (electrical ON)
	  RGB_LED_PORT_B |= RGB_LED_MASK_B;
     }
#else
     if ( pwm_counter < pwm_r_top_value) {
	  // LED on (electrical ON)
	  RGB_LED_PORT_R |= RGB_LED_MASK_R;
     }
     else {
	  // LED off (electrical OFF)
	  RGB_LED_PORT_R &= ~(RGB_LED_MASK_R);
     }

     if ( pwm_counter < pwm_g_top_value) {
	  // LED on (electrical ON)
	  RGB_LED_PORT_G |= RGB_LED_MASK_G;
     }
     else {
	  // LED off (electrical OFF)
	  RGB_LED_PORT_G &= ~(RGB_LED_MASK_G);
     }

     if ( pwm_counter < pwm_b_top_value) {
	  // LED on (electrical ON)
	  RGB_LED_PORT_B |= RGB_LED_MASK_B;
     }
     else {
	  // LED off (electrical OFF)
	  RGB_LED_PORT_B &= ~(RGB_LED_MASK_B);
     }
#endif
}

void errorSequence() {
     setColor(255, 0, 0);
     // setLEDBlink(12);
}

void setDefaultColor(uint8_t LEDColor) {

	if (LEDColor == 0xff) LEDColor = eeprom::getColor();

     // blink rate has to be set first in order for color to register,
     // so set blink before each color
     uint8_t r = 0x00, g = 0x00, b = 0x00;

     switch(LEDColor) {
     default:
     case LED_DEFAULT_OFF:    break;
     case LED_DEFAULT_WHITE:  r = g = b = 0xFF;   break;
     case LED_DEFAULT_BLUE:   b = 0xFF;           break;
     case LED_DEFAULT_RED:    r = 0xFF;           break;
     case LED_DEFAULT_GREEN:  g = 0xFF;           break;
     case LED_DEFAULT_ORANGE: r = 0xFF; g = 0x80; break;
     case LED_DEFAULT_PINK:   r = b = 0xFF;       break;
     case LED_DEFAULT_PURPLE: r = 0x7F; b = 0xFF; break;
     case LED_DEFAULT_CUSTOM:
     {
	  uint32_t CustomColor = eeprom::getEeprom32(
	       eeprom_offsets::LED_STRIP_SETTINGS +
	       blink_eeprom_offsets::CUSTOM_COLOR_OFFSET,
	       0xFFFFFFFF);

	  r = CustomColor >> 24;
	  g = CustomColor >> 16;
	  b = CustomColor >>  8;
	  break;
     }
     }
     setColor(r, g, b);
}


// set LED color and store to EEPROM "custom" color area
void setCustomColor(uint8_t red, uint8_t green, uint8_t blue) {
     eeprom::setCustomColor(red, green, blue);
     setColor(red, green, blue);
}


void setColor(uint8_t r, uint8_t g, uint8_t b) {
     pwm_r_top_value = r >> RGB_RES;
     pwm_g_top_value = g >> RGB_RES;
     pwm_b_top_value = b >> RGB_RES;

     // Turn LEDs off
#if RGB_LED_INVERTED
     RGB_LED_PORT_R |= RGB_LED_MASK_R;
     RGB_LED_PORT_G |= RGB_LED_MASK_G;
     RGB_LED_PORT_B |= RGB_LED_MASK_B;
#else
     RGB_LED_PORT_R &= ~(RGB_LED_MASK_R);
     RGB_LED_PORT_G &= ~(RGB_LED_MASK_G);
     RGB_LED_PORT_B &= ~(RGB_LED_MASK_B);
#endif

#define SOLID_ON_OFF(x) ( ((x) == 0) || ((x) == (0xFF >> (RGB_RES))) )

     // Special case situations in which the LEDs are either 100% on or off
     if ( SOLID_ON_OFF(pwm_r_top_value) &&
	  SOLID_ON_OFF(pwm_g_top_value) &&
	  SOLID_ON_OFF(pwm_b_top_value) ) {

	  // Each LED is either 100% on or 100% off
	  // PWM is not needed; just latch them into the correct state
	  // ... so dispense with PWM... this allows us to support the
	  // common case of the LED's all on (WHITE) and not having to
	  // use an interrupt to allow that.

	  // Disable Timer 1
	  TCCR1A = 0;
	  TCCR1B = 0;
	  TCCR1C = 0;
	  OCR1A  = 0;
	  OCR1B  = 0;
	  OCR1C  = 0;
	  TIMSK1 = 0;

	  // Turn some LEDs ON
#if RGB_LED_INVERTED
	  if (pwm_r_top_value) RGB_LED_PORT_R &= ~(RGB_LED_MASK_R);
	  if (pwm_g_top_value) RGB_LED_PORT_G &= ~(RGB_LED_MASK_G);
	  if (pwm_b_top_value) RGB_LED_PORT_B &= ~(RGB_LED_MASK_B);
#else
	  if (pwm_r_top_value) RGB_LED_PORT_R |= RGB_LED_MASK_R;
	  if (pwm_g_top_value) RGB_LED_PORT_G |= RGB_LED_MASK_G;
	  if (pwm_b_top_value) RGB_LED_PORT_B |= RGB_LED_MASK_B;
#endif
     }
     else if ( (r != 0) || (g != 0) || (b != 0) ) {

	  // PWM is needed to achieve the desired color

	  // Enable Timer 1
	  TCCR1A = 0;     // Pins disconnected
	  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC; 1/64 prescaler -> 250 KHz
	  TCCR1C = 0;
	  OCR1A  = 125;  // 250 KHz / 125 -> 5 KHz
	  OCR1B  = 0;
	  OCR1C  = 0;
	  TCNT1  = 0;
	  TIMSK1 = (1 << OCIE1A);
     }

#undef SOLID_ON_OFF
}
}

#endif
