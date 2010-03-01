#include "Configuration.hh"
#include "Heater.hh"
#include "HeatingElement.hh"
#include "Thermistor.hh"

Thermistor thermistor(THERMISTOR_PIN,128);
Heater extruder_heater(thermistor);

Heater::Heater(TemperatureSensor& sensor) : sensor_(sensor),
	current_temperature_(0),
	target_temperature_(0),
	max_temperature_(250),
	last_update(0)
{
  pid_.reset();
  pid_.setTarget(0);
}

void Heater::set_target_temperature(int temp)
{
	pid_.setTarget(temp);
}

bool Heater::hasReachedTargetTemperature()
{
	// WHAAAA?  Holdover from old code; let's crank it up a bit at least?
	return (current_temperature_ > (int)(pid_.getTarget()* 0.98));
}

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int Heater::get_current_temperature()
{
	return sensor_.getTemperature();
}


/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
	micros_t time = getCurrentMicros();
	// handle timer overflow
	if (time < last_update) {
		last_update = 0;
	}
	if (time - last_update >= UPDATE_INTERVAL_MICROS) {
		last_update = time;
		sensor_.update();
	}
// update the temperature reading.
  current_temperature_ = get_current_temperature();

  int mv = pid_.calculate(current_temperature_);
  set_output(mv);
}

void Heater::set_output(uint8_t value)
{
	setHeatingElement(value);
}
