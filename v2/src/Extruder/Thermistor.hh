#ifndef THERMISTOR_HH_
#define THERMISTOR_HH_

#include "Temperature.hh"
#include "CircularBuffer.hh"
#include "AvrPort.hh"

const static uint16_t THERM_TABLE_SIZE = 20;

struct ThermTableEntry {
	uint16_t adc_;
	int16_t celsius_;
} __attribute__ ((packed));

class Thermistor : public TemperatureSensor {
private:
	uint8_t analog_pin_; // index of analog pin
	volatile uint16_t raw_value_; // raw storage for asynchronous analog read
	const static int ADC_RANGE = 1024;
	const static uint8_t SAMPLE_COUNT = 10;
	uint16_t sample_buffer_[SAMPLE_COUNT];
	uint8_t next_sample_;
	const uint8_t table_index_;

public:
	Thermistor(uint8_t analog_pin, uint8_t table_index);
	void update();
};

#endif //THERMISTOR_H
