/*
 * Copyright 2011 by Alison Leonard	 <alison@makerbot.com>
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
 */
 
 #include "Piezo.hh"
 #include "Configuration.hh"
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <util/delay.h>
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>
 
 namespace Piezo{

Pin BuzzPin = BUZZER_PIN;
Timeout piezoTimeout;
bool ToneOn = false;
bool playing = false;
uint8_t queueLength = 0;
uint32_t toggleCount = 0;
uint32_t toggle_time =  0;
uint32_t lastMicros = 0;
bool toggle = false;
const static uint8_t TONE_QUEUE_SIZE = 20;
//TRICKY: ^ Wwas 40, but that was causing bugs, failures. Lowered to 20 for shipping

// TODO change to one buffer of sound structs
uint16_t frequency_buf[TONE_QUEUE_SIZE];
uint16_t duration_buf[TONE_QUEUE_SIZE];
CircularBuffer16 frequencies(TONE_QUEUE_SIZE, frequency_buf);
CircularBuffer16 durations(TONE_QUEUE_SIZE, duration_buf);
 
 // call this sequence on startup
 void startUpTone()
 {		
	 ///song 3
	 setTone(NOTE_A7, 288);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_CS8, 188);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_D8, 433);
 }

// call this sequence at the end of prints
 void doneTone( )// Ta-da!
 {

	 setTone(NOTE_A5, 188);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_A6, 188);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_B6, 188);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_C7, 188);
	 setTone(NOTE_CS7, 188);
	 setTone(NOTE_0, 188);
	 setTone(NOTE_A7, 333);

 }
 bool isPlaying(){
	return playing;
 }



 // call this sequence on error
 void errorTone(uint8_t iterations)
 {
	 for(int i = 0; i < iterations; i++)
	 {
		setTone(NOTE_B4, 300);
		setTone(NOTE_A4, 300);
	}
 }
 
 // allow queuing of tones so that multiple tones can be called sequentially
 // without waiting for each to finish
 void queueTone(uint16_t frequency, uint16_t duration)
 {
	 if(frequencies.getRemainingCapacity() > 0 ) {
		 frequencies.push(frequency);
		 durations.push(duration);
	 }
	 //FUTURE: set an error blink here.
 }
 
 // derived from arduino Tone library
 void setTone(uint16_t frequency, uint16_t duration)
{
	 /// if we are already playing, cache the next tone
	if( ToneOn ) {
		queueTone(frequency,duration);
		return;
	}

    if(eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS + buzz_eeprom_offsets::BASIC_BUZZ_OFFSET,1) == 0)
        return;

	playing = true;
	ToneOn = true;
	BuzzPin.setValue(false);
    BuzzPin.setDirection(true);
		
	// 8 MHZ base clock
	uint32_t fCPU = 8000000;
	// scan through prescalars to find the best fit
      uint32_t ocr = fCPU / frequency / 2 - 1;
      toggle_time = ocr;
      uint8_t prescalarbits = 0b001;  
     
      if (ocr > 255)
      {
        ocr = fCPU / frequency / 2 / 8 - 1;
        prescalarbits = 0b010;  

        if (ocr > 255)
        {
          ocr = fCPU / frequency / 2 / 64 - 1;
          prescalarbits = 0b011;

          if (ocr > 255)
          {
            ocr = fCPU / frequency / 2 / 256 - 1;
            prescalarbits = 0b100;
            
            if (ocr > 255)
            {
              // can't do any better than /1024
              ocr = fCPU / frequency / 2 / 1024 - 1;
              prescalarbits = 0b101;
            }
          }
        }
      }
      TCCR0B = 0b00001000 + prescalarbits; //set prescaler to desired value
      
      OCR0A = ocr & 0xFF;
      OCR0B = ocr & 0xFF; //set pwm frequency
      TIMSK0 = 0b00000010; //turn compA interrupt on
      toggleCount = 2L * frequency * duration / 1000L;
      lastMicros = 0;
}

void doInterrupt()
{			  
	if (toggleCount != 0)
  {
    // toggle the pin
    toggle = !toggle;
    BuzzPin.setValue(toggle);

    if (toggleCount > 0)
      toggleCount--;
  }
 // clear after interval
  else
  {
    TIMSK0 = 0;
    OCR0B = 0;
    OCR0A = 0;
    BuzzPin.setValue(false);  // keep pin low after stop
    ToneOn = false;
    if(frequencies.isEmpty() == false)
		setTone(frequencies.pop(), durations.pop());
	else
		playing = false;
   }
}
}



