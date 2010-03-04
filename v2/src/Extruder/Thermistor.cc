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

/*
uint16_t Thermistor::rawToCelsius(uint16_t raw) {
	// lookup.  The tables are now guaranteed to have their entries at fixed intervals over the range of the ADC.
	const static int SLOT_WIDTH = ADC_RANGE/(THERM_TABLE_SIZE-1);

	int floor_table_index = (raw * (THERM_TABLE_SIZE-1)) / ADC_RANGE;
	int ceiling_table_index = floor_table_index+1;

	int entry_floor = (floor_table_index * ADC_RANGE) / (THERM_TABLE_SIZE-1);

	int16_t floor = eeprom_read_word((const uint16_t*)(table_offset_ + (4 * floor_table_index) + 2));
	int16_t ceiling = eeprom_read_word((const uint16_t*)(table_offset_ + (4 * ceiling_table_index) + 2));

	return floor + ((raw - entry_floor) * (ceiling-floor))/SLOT_WIDTH;
}
*/

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
