#ifndef HEATER_H
#define HEATER_H

#include "TemperatureSensor.hh"
#include "HeatingElement.hh"
#include "AvrPort.hh"
#include "PID.hh"
#include "Types.hh"

class Heater
{
  private:
	TemperatureSensor& sensor;
    HeatingElement& element;
    
    int current_temperature;

    PID pid;

    const static int UPDATE_INTERVAL_MICROS = 60;

  public:
    micros_t last_update;
    Heater(TemperatureSensor& sensor, HeatingElement& element);
    
    int get_current_temperature(); 
    void set_target_temperature(int temp);
    bool hasReachedTargetTemperature();

    // Call once each temperature interval
    void manage_temperature();

    void set_output(uint8_t value);
};

#endif // HEATER_H
