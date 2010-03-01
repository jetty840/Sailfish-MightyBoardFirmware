#ifndef HEATER_H
#define HEATER_H

#include "Temperature.hh"
#include "AvrPort.hh"
#include "PID.hh"
#include "Timers.hh"

class Heater
{
  private:
	TemperatureSensor& sensor_;
    
    int current_temperature_;
    int target_temperature_;
    int max_temperature_;

    PID pid_;

    const static int UPDATE_INTERVAL_MICROS = 600;

  public:
    micros_t last_update;
    Heater(TemperatureSensor& sensor);
    
    int get_current_temperature(); 
    void set_target_temperature(int temp);
    bool hasReachedTargetTemperature();

    // Call once each temperature interval
    void manage_temperature();

    void set_output(uint8_t value);
};

extern Heater extruder_heater;

#endif // HEATER_H
