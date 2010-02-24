#ifndef HEATER_H
#define HEATER_H

#include "Temperature.hh"
#include "AvrPort.hh"
#include "PID.hh"

class Heater
{
  private:
	TemperatureSensor *sensor_;
    
    int current_temperature_;
    int target_temperature_;
    int max_temperature_;

    PID pid_;

  public:    
    void init(TemperatureSensor *sensor);
    
    int get_current_temperature(); 
    void set_target_temperature(int temp);
    bool hasReachedTargetTemperature();

    // Call once each temperature interval
    void manage_temperature();

};

#endif // HEATER_H
