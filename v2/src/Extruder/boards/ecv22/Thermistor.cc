#include "Thermistor.hh"
#include "ThermistorTable.hh"
#include "DebugPin.hh"
#include "AnalogPin.hh"
#include <avr/eeprom.h>
#include <util/atomic.h>

Thermistor::Thermistor(uint8_t analog_pin, uint8_t table_index) :
analog_pin_(analog_pin), next_sample_(0), table_index_(table_index) {
	for (int i = 0; i < SAMPLE_COUNT; i++) { sample_buffer_[i] = 0; }
}

void Thermistor::init() {
	initAnalogPins(_BV(analog_pin_));
}

void Thermistor::update() {
	uint16_t temp;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		temp = raw_value_;
	}
	// initiate next read
	startAnalogRead(analog_pin_,&raw_value_);

	sample_buffer_[next_sample_] = temp;
	next_sample_ = (next_sample_+1) % SAMPLE_COUNT;

	// average
	int16_t cumulative = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		cumulative += sample_buffer_[i];
	}
	int16_t avg = cumulative / SAMPLE_COUNT;

	current_temp_ = thermistorToCelsius(avg,table_index_);
}
