#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>

class Heater
{
  private:
    bool usesThermocoupler;
    int inputPin;
    int outputPin;
    
    int current_temperature;
    int target_temperature;
    int max_temperature;

    bool temp_control_enabled;
    unsigned long temp_prev_time; // ms

#if TEMP_PID
    float temp_pGain;
    float temp_iGain;
    float temp_dGain;

    int  temp_dState;
    long temp_iState;
    float temp_iState_max; // set in update_windup
    float temp_iState_min; // set in update_windup
#endif

  protected:
    int read_thermistor();
    int read_thermocouple();
    int sample_thermistor();
    int sample_thermocouple();
    int temp_update(int dt);
    
  public:    
    void init(int inPin, int outPin, bool isThermocoupler);
    
    int get_current_temperature(); 
    void set_target_temperature(int temp);
    bool hasReachedTargetTemperature();

    void manage_temperature();
    void temp_pid_update_windup();
};

#endif // HEATER_H
