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
	Pin pin_;
	const static int ADC_RANGE = 1024;
	const static uint8_t SAMPLE_COUNT = 10;
	uint8_t sample_buffer_[SAMPLE_COUNT];
	uint8_t next_sample_;
	const uint16_t table_offset_;
public:
	Thermistor(Pin& pin, uint16_t table_offset);
	void update();
};

#endif //THERMISTOR_H
