#include "Configuration.hh"
#include "Heater.hh"

void Heater::init(int inPin, int outPin, bool isThermocoupler)
{
  usesThermocoupler = isThermocoupler;
  inputPin = inPin;
  outputPin = outPin;

  temp_control_enabled = true;
  pid_.reset();
  pid_.setTarget(0);
  current_temperature =  0;
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
	sensor_->update();
	return sensor_->getTemperature();
}


/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
  // update the temperature reading.
  current_temperature_ = get_current_temperature();

  int mv = pid_.calculate(current_temperature_);
  heater_pin_.
    analogWrite(outputPin, output);
  }
}
