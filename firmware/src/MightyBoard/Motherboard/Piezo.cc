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

Piezo::Piezo(Pin BuzzPinIn):
	BuzzPin(BuzzPinIn),
	frequencies(TONE_QUEUE_SIZE, frequency_buf),
	durations(TONE_QUEUE_SIZE, duration_buf)
{
 	BuzzPin.setDirection(true);
 	ToneOn = false;
 	queueLength = 0;
 }
 
 // call this sequence on startup
 void Piezo::startUpTone()
 {
 	setTone(NOTE_G4, 1000);
 	setTone(NOTE_B0, 500);
 	setTone(NOTE_C8, 500);
 }
 
 // call this sequence on error
 void Piezo::errorTone()
 {
 	setTone(NOTE_B2, 300);
 	setTone(NOTE_A2, 300);
 }
 
 // allow queuing of tones so that multiple tones can be called sequentially
 // without waiting for each to finish
 void Piezo::queueTone(uint16_t frequency, uint16_t duration)
 {
 	queueLength++;
 	frequencies.push(frequency);
 	durations.push(duration);
 }
 
 // derived from arduino Tone library
 void Piezo::setTone(uint16_t frequency, uint16_t duration)
{
	if(ToneOn)
	{
		queueTone(frequency,duration);
//		DEBUG_PIN1.setValue(true);
		return;
	}
		
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
      TCCR0B = 0b00001000 + prescalarbits; //set prescaler to desired value
      
      OCR0B = ocr & 0xFF; //set pwm frequency
      TCCR0A |= 0b00100000; //turn PWM output on
      
      DEBUG_PIN1.setValue(OCR0B & 0x80);
      DEBUG_PIN2.setValue(OCR0B & 0x40);
      DEBUG_PIN3.setValue(OCR0B & 0x20);
      
      ToneOn = true;
      piezoTimeout.start(duration*1000L);
}

void Piezo::doInterrupt()
{
	if(ToneOn && piezoTimeout.hasElapsed())
	{
		TCCR0A &= 0b11001111; //turn PWM output off
		ToneOn = false;	
		if(queueLength > 0)
		{
			queueLength--;
			setTone(frequencies.pop(), durations.pop());
		}
	}
}
