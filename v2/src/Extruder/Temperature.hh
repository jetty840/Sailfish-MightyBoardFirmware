#ifndef TEMPERATURE_HH_
#define TEMPERATURE_HH_

#include <stdint.h>

class TemperatureSensor {
protected:
	// Maintained by the sensor update routine
	int16_t current_temp_;
public:
	// Return current estimated temperature in degrees Celsius.
	int16_t getTemperature() const { return current_temp_; }
	// Run this sensor's update routine
	void update() =0;
};

#endif // TEMPERATURE_HH_
