#include "Configuration.hh"
#include "Heater.hh"
#include "HeatingElement.hh"
#include "Thermistor.hh"
#include "DebugPin.hh"
#include "ExtruderBoard.hh"

Heater::Heater(TemperatureSensor& sensor_in, HeatingElement& element_in) :
		sensor(sensor_in),
		element(element_in),
		current_temperature(0),
		last_update(0)
{
  pid.reset();
  pid.setPGain(5.0);
  pid.setIGain(0.1);
  pid.setDGain(100.0);
  pid.setTarget(0);
}

void Heater::set_target_temperature(int temp)
{
	pid.setTarget(temp);
}

bool Heater::hasReachedTargetTemperature()
{
	// WHAAAA?  Holdover from old code; let's crank it up a bit at least?
	return (current_temperature > (int)(pid.getTarget()* 0.98));
}

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int Heater::get_current_temperature()
{
	return sensor.getTemperature();
}


/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
	micros_t time = ExtruderBoard::getBoard().getCurrentMicros();
	// handle timer overflow
	if (time < last_update) {
		last_update = 0;
	}
	if (time - last_update >= UPDATE_INTERVAL_MICROS) {
		last_update = time;
		sensor.update();
	}
	// update the temperature reading.
	current_temperature = get_current_temperature();

	int mv = pid.calculate(current_temperature);
	// clamp value
	if (mv < 0) { mv = 0; }
	if (mv >255) { mv = 255; }
	set_output(mv);
}

void Heater::set_output(uint8_t value)
{
	element.setHeatingElement(value);
}
