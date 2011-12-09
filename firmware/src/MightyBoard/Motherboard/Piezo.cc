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
 
 namespace Piezo{
	
Pin BuzzPin = BUZZER_PIN;
Timeout piezoTimeout;
bool ToneOn = false;
uint8_t queueLength = 0;
uint16_t toggleCount = 0;
bool toggle = false;

// TODO change to one buffer of sound structs	
uint16_t frequency_buf[TONE_QUEUE_SIZE];
uint16_t duration_buf[TONE_QUEUE_SIZE];
CircularBuffer16 frequencies(TONE_QUEUE_SIZE, frequency_buf);
CircularBuffer16 durations(TONE_QUEUE_SIZE, duration_buf);
 
 // call this sequence on startup
 void startUpTone()
 {
 	setTone(NOTE_G4, 1000); //392
 	setTone(NOTE_B0, 500); //31
 	setTone(NOTE_C8, 500); //4186
 }
 
 // call this sequence on error
 void errorTone()
 {
 	setTone(NOTE_B2, 300);
 	setTone(NOTE_A2, 300);
 }
 
 // allow queuing of tones so that multiple tones can be called sequentially
 // without waiting for each to finish
 void queueTone(uint16_t frequency, uint16_t duration)
 {
 	queueLength++;
 	frequencies.push(frequency);
 	durations.push(duration);
 }
 
 // derived from arduino Tone library
 void setTone(uint16_t frequency, uint16_t duration)
{
	if(ToneOn)
	{
		queueTone(frequency,duration);
//		DEBUG_PIN1.setValue(true);
		return;
	}
	BuzzPin.setValue(false);
	BuzzPin.setDirection(true);
		
	// 8 MHZ base clock
	uint32_t fCPU = 8000000;
	// scan through prescalars to find the best fit
      uint32_t ocr = fCPU / frequency / 2 - 1;
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
      TCCR0B = prescalarbits; //0b00001000 + prescalarbits; //set prescaler to desired value
      
      OCR0B = ocr & 0xFF; //set pwm frequency
 //     TCCR0A |= 0b00100000; //turn PWM output on
      TIMSK0 = 0b00000100;
      toggleCount = 2L * frequency * duration / 1000;
      
      ToneOn = true;
      //piezoTimeout.start(duration*1000L);
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
  else
  {
    TIMSK0 = 0;
    ToneOn = false;
    BuzzPin.setValue(false);  // keep pin low after stop
    if(queueLength > 0)
	{
		queueLength--;
		setTone(frequencies.pop(), durations.pop());
	}
   }
}
/*	if(ToneOn && piezoTimeout.hasElapsed())
	{
		TCCR0A &= 0b11001111; //turn PWM output off
		ToneOn = false;	
		if(queueLength > 0)
		{
			queueLength--;
			setTone(frequencies.pop(), durations.pop());
		}
	}
	*/
}

ISR(TIMER0_COMPB_vect)
{
  Piezo::doInterrupt();
}

