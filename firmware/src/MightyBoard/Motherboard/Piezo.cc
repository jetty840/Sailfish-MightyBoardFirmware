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
 *
 * Modified to reduce CPU load by Jetty 31st August, 2012
 *
 */
 
#include "Piezo.hh"
#include "Configuration.hh"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

#include "Steppers.hh"
 
namespace Piezo {


// Tones are placed on a tone queue when setTone is called.  The sound queue is internally limited to
// TONE_QUEUE_SIZE.
//
// At some point later, runPiezoSlice, processes the tone
//
// Sounds are started and removed from the queue by runPiezoSlice (and also started by setTone if the queue is empty).
//
// There is no interrupt for this buzzer, the Timer is hooked up via OCR0B directly to the buzzer pin, and toggles when
// the timer loops.  This is frequency generation without the overhead of the CPU having to toggle the pin manually and
// run an interrupt at the frequency of the tone.  Reduces CPU load and results in better tone generation.
//


// Index for tone_data
#define FREQUENCY 0
#define DURATION  1

// Union used for mapping a combined 2 x uint16_t value into a uint32_t value for storage in the circular buffer
struct tone {
	union {
		uint32_t tone_entry;	//Place holder for referencing
		uint16_t tone_data[2];	//index 0 is frequency, index 1 is duration
	};
};

// Setup the tone buffer
const static uint8_t TONE_QUEUE_SIZE = 40;

uint32_t tones_buf[TONE_QUEUE_SIZE];
CircularBuffer32 tones(TONE_QUEUE_SIZE, tones_buf);



static bool soundEnabled = false;
static bool playing = false;
static Timeout piezoTimeout;



// This should be called if the buzzer status is ever changed
// It empties the queue and clears the buffer

void reset(void) {
	// Reads the sound setting in from eeprom
	soundEnabled = (bool)(eeprom::getEeprom8(eeprom_offsets::BUZZ_SETTINGS + buzz_eeprom_offsets::BASIC_BUZZ_OFFSET,1) != 0);

	//Empty the queue
	tones.reset();

	// Clear the timer, and we're not playing anymore
	piezoTimeout.clear();
	playing = false;

	// We need to set the buzzer pin to output, so that the timer can drive it
	BUZZER_PIN.setDirection(true);
}


// Switches the hardware timer off

void shutdown_timer(void) {
#ifdef MODEL_REPLICATOR2
  TCCR4A = 0x00;//0x00;
	TCCR4B = 0x00; //0x00;
	OCR4A  = 0x00;
	OCR4B  = 0x00;
	TCNT4  = 0x00;
	TIMSK4 = 0x00;	//No interrupts
#else
	TCCR0A = 0x00;
	TCCR0B = 0x00;
	OCR0A  = 0x00;
	OCR0B  = 0x00;
	TCNT0  = 0x00;
	TIMSK0 = 0x00;	//No interrupts
#endif
}


// Internal routine, sets up timer to start playing
// or stops the timer if the buffer is empty

void processNextTone(void) {
	if ( tones.isEmpty() ) {
		piezoTimeout.clear();
		playing = false;

		//Shutdown the timer for now
		shutdown_timer();
	} else {
 		playing = true;

		//Get the next tone from the buffer
		struct tone tone;

		tone.tone_entry		= tones.pop();

		//Schedule the end of tone.  Duration is in ms, timer is in uS, so
		//we multiply by 1000
		piezoTimeout.start((micros_t)tone.tone_data[DURATION] * (micros_t)1000);

		if ( tone.tone_data[FREQUENCY] == NOTE_0 ) {
			//Note 0 acts as a rest (i.e. doesn't play a note), so we shut the timer down
			shutdown_timer();
		} else {
			uint32_t freq = (uint32_t)tone.tone_data[FREQUENCY];

			// OCR0A for a given frequency can be calculated with:
			// OCR0A = (F_CPU / (f * 2 * prescaler_factor)) - 1

			// Pick the appropriate prescaler for the given frequency
			uint8_t prescalerFactorBits;
			uint8_t prescalerBits;


#ifdef MODEL_REPLICATOR2
			//Setup the counter to count from 0 to OCR0A and toggle OC0B on counter reset
			if ( freq >= 500) {
				//Prescaler = 8	(1 << 3)
				prescalerFactorBits	= 3;
				prescalerBits		= _BV(CS41);
			} else {
				//Prescaler = 64 (1 << 6)
				prescalerFactorBits	= 6;
				prescalerBits		= _BV(CS41) | _BV(CS40);
			} 

			//Calculate the value for OCR0A, but save it in a variable
			//to avoid contaminating the timer with a lengthy calculation
			uint16_t outputCompareTop = (uint16_t)(((uint32_t)F_CPU / ((freq << (uint32_t)1) << (uint32_t)prescalerFactorBits)) - 1);
			TCCR4A = _BV(COM4A0);		//Toggle OC0B on compare match (i.e we're running at half the frequency of ORC0A,

			TCCR4B = prescalerBits |			//Prescaler
								_BV(WGM42);			// CTC (top == OCR0A)
      

			OCR4A  = (uint16_t)outputCompareTop;	//Frequency when compiled with prescaler

			OCR4B  = 0x00;				//Not used
			TCNT4  = 0x00;				//Clear the counter
			TIMSK4 = 0x00;				//No interrupts

#else
			if ( freq >= 3906 ) {
				//Prescaler = 8	(1 << 3)
				prescalerFactorBits	= 3;
				prescalerBits		= _BV(CS01);
			} else if ( freq >= 488  ) {
				//Prescaler = 64 (1 << 6)
				prescalerFactorBits	= 6;
				prescalerBits		= _BV(CS01) | _BV(CS00);
			} else if ( freq >= 122  ) {
				//Prescaler = 256 (1 << 8)
				prescalerFactorBits	= 8;
				prescalerBits		= _BV(CS02);
			} else 			   {
				//Prescaler = 1024 (1 << 10)
				prescalerFactorBits	= 10;
				prescalerBits		= _BV(CS02) | _BV(CS00);
			} 

			//Calculate the value for OCR0A, but save it in a variable
			//to avoid contaminating the timer with a lengthy calculation
			uint8_t outputCompareTop = (uint8_t)(((uint32_t)F_CPU / ((freq << (uint32_t)1) << (uint32_t)prescalerFactorBits)) - 1);
			//Setup the counter to count from 0 to OCR0A and toggle OC0B on counter reset
			TCCR0A = _BV(COM0B0) |			//Toggle OC0B on compare match (i.e we're running at half the frequency of ORC0A,
								_BV(WGM01);			// CTC (top == OCR0A)

			TCCR0B = prescalerBits;			//Prescaler

			OCR0A  = (uint8_t)outputCompareTop;	//Frequency when compiled with prescaler

			OCR0B  = 0x00;				//Not used
			TCNT0  = 0x00;				//Clear the counter
			TIMSK0 = 0x00;				//No interrupts
#endif
		}
	}
}


void setTone(uint16_t frequency, uint16_t duration)
{
	//If sound is switched off, we do nothin
	if ( ! soundEnabled )	return;

	//Add the tone to the buffer
	if ( tones.getRemainingCapacity() ) {
		struct tone tone;

		tone.tone_data[FREQUENCY] = frequency;
		tone.tone_data[DURATION]  = duration;

		tones.push(tone.tone_entry);
	}

	//If we're not playing tones, then we schedule the tone we just put in the queue
	if ( ! playing )	processNextTone();
}


bool isPlaying() {
	return playing;
}


void runPiezoSlice(void) {
	// If we're playing a sound and the last sound has finished, queue
	// the next one if we have one
	if ( (soundEnabled ) && ( playing ) && ( piezoTimeout.hasElapsed() )) {
		processNextTone();
	}
}


//Tunes

const uint16_t tune_startup[] PROGMEM = {
	NOTE_A7,  288,//	288,
	//NOTE_0,		94, //188,
	NOTE_CS8,	188, //188,
	//NOTE_0,		94, //188,
	NOTE_D8,	433, //433,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_print_done[] PROGMEM = {
  1319,   85,
  0,      85,
  1568,   85,
  0,      85,
  2093,   85,
  0,      85,
  2637,   85,
  0,      85,
  3136,   171,
  0,      171,
  2349,   85,
  0,      85,
  2093,   85,
  0,      85,
  2637,   171,
  0,      85,
  0,      256,
  3520,   85,
  0,      85,
  2093,   342,
	NOTE_0,	0,	//Zero duration is a terminator
};

const uint16_t tune_makerbot_tv[] PROGMEM = {
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_AS7,	400,
	NOTE_0,		20,
	NOTE_C8,	400,
	NOTE_0,		20,
	NOTE_A7,	800,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_beethoven_5th[] PROGMEM = {
  NOTE_FS6,   200,
  NOTE_0,     20,
  NOTE_FS6,   200,
  NOTE_0,     20,
  NOTE_FS6,   200,
  NOTE_0,     20,
  NOTE_DS6,   1200,
  NOTE_0,     20,
  NOTE_F6,    200,
  NOTE_0,     20,
  NOTE_F6,    200,
  NOTE_0,     20,
  NOTE_F6,    200,
  NOTE_0,     20,
  NOTE_D6,    1200,
  NOTE_0,     0,  //Zero duration is a terminator
};

const uint16_t tune_start_print[] PROGMEM = {
  2093,   85,
  0,      85,
  831,    85,
  0,      85,
  2489,   85,
  0,      85,
  831,    85,
  0,      85,
  3322,   85,
  0,      85,
  831,    85,
  0,      85,
  2489,   85,
  0,      85,
  831,    85,
  0,      85,
  2349,   85,
  0,      85,
  831,    85,
  0,      85,
  2794,   85,
  0,      85,
  831,    85,
  0,      85,
  2489,   171,
	NOTE_0,	0,	//Zero duration is a terminator
};

const uint16_t tune_filament_start[] PROGMEM = {
  2217,   85,
  0,      85,
  1976,   85,
  0,      85,
  2489,   171,
  0,      171,
  1661,   85,
  0,      85,
  1480,   85,
  0,      85,
  1976,   171,
  0,      171,
  1480,   85,
  0,      85,
  1245,   85,
  0,      85,
  1480,   85,
  0,      85,
  1976,   85,
  0,      85,
  1661,   342,
	NOTE_0,	0,	//Zero duration is a terminator
};

const uint16_t tune_error[] PROGMEM = {
  659,    105,
  0,      105,
  494,    105,
  0,      105,
  294,    440,
	NOTE_0,	0,	//Zero duration is a terminator
};


//Plays a tune given a tune id

void playTune(uint8_t tuneid) {
	const uint16_t *tunePtr = NULL;

	switch ( tuneid ) {
		case TUNE_PRINT_DONE:
			tunePtr = tune_print_done;
			break;
		case TUNE_STARTUP:
			tunePtr = tune_startup;
			break;
		case TUNE_MAKERBOT_TV:
			tunePtr = tune_makerbot_tv;
			break;
		case TUNE_BEETHOVEN_5TH:
			tunePtr = tune_beethoven_5th;
			break;
		case TUNE_FILAMENT_START:
			tunePtr = tune_filament_start;
			break;
		case TUNE_ERROR:
			tunePtr = tune_error;
			break;
    case TUNE_PRINT_START:
      tunePtr = tune_start_print;
		default:
			break;
	}

	if ( tunePtr ) {
		//Play the tune from progmem
		uint8_t count = 0;
		uint16_t note, duration;
		do {
			note		= pgm_read_word(tunePtr + count * 2);
			duration	= pgm_read_word(tunePtr + count * 2 + 1);
			if ( duration != 0 )	setTone(note, duration);
			count ++;
		} while (duration != 0 ); 	//duration == 0 marks the end of the tune
	}
	else setTone(NOTE_B2, 500);	//Play this is the tuneid doesn't exist
}


// call this sequence on startup
void startUpTone()
{		
	playTune(TUNE_FILAMENT_START);
}


// call this sequence at the end of prints
void doneTone( )// Ta-da!
{
	playTune(TUNE_PRINT_DONE);
}


// call this sequence on error
void errorTone(uint8_t iterations)
{
	for(int i = 0; i < iterations; i++) {
		setTone(NOTE_B4, 300);
		setTone(NOTE_A4, 300);
	}
}

}
