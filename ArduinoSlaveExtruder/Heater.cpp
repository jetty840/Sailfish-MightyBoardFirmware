#include "Configuration.h"
#include "Heater.h"
#include "Variables.h"
#include "ThermistorTable.h"

void set_extruder_temperature(int temp)
{
  extruder_target_temperature = temp;
  extruder_max_temperature = (int)((float)temp * 1.1);
}

#ifdef THERMISTOR_PIN
int read_thermistor();
#endif // THERMISTOR_PIN

#ifdef THERMOCOUPLE_PIN
int read_thermocouple();
#endif // THERMOCOUPLE_PIN

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int get_temperature()
{
#ifdef THERMISTOR_PIN
  return read_thermistor();
#endif
#ifdef THERMOCOUPLE_PIN
  return read_thermocouple();
#endif
}

/*
* This function gives us the temperature from the thermistor in Celsius
 */
#ifdef THERMISTOR_PIN
int read_thermistor()
{
  int raw = sample_temperature(THERMISTOR_PIN);

  int celsius = 0;
  byte i;

  for (i=1; i<NUMTEMPS; i++)
  {
    if (temptable[i][0] > raw)
    {
      celsius  = temptable[i-1][1] + 
        (raw - temptable[i-1][0]) * 
        (temptable[i][1] - temptable[i-1][1]) /
        (temptable[i][0] - temptable[i-1][0]);

      if (celsius > 255)
        celsius = 255; 

      break;
    }
  }

  // Overflow: We just clamp to 0 degrees celsius
  if (i == NUMTEMPS)
    celsius = 0;

  return celsius;
}
#endif

/*
* This function gives us the temperature from the thermocouple in Celsius
 */
#ifdef THERMOCOUPLE_PIN
int read_thermocouple()
{
  return ( 5.0 * sample_temperature(THERMOCOUPLE_PIN) * 100.0) / 1024.0;
}
#endif

/*
* This function gives us an averaged sample of the analog temperature pin.
 */
int sample_temperature(uint8_t pin)
{
  int raw = 0;

  //read in a certain number of samples
  for (byte i=0; i<TEMPERATURE_SAMPLES; i++)
    raw += analogRead(pin);

  //average the samples
  raw = raw/TEMPERATURE_SAMPLES;

  //send it back.
  return raw;
}

int temp_update(int dt);

/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void manage_extruder_temperature()
{
  int output, dt;
  unsigned long time;

  //make sure we know what our temp is.
  extruder_current_temperature = get_temperature();
    
  // ignoring millis rollover for now
  time = millis();
  dt = time - extruder_temp_prev_time;

  if (dt > TEMP_UPDATE_INTERVAL)
  { 
    extruder_temp_prev_time = time;
    output = temp_update(dt);
    analogWrite(HEATER_PIN,output);
  }
}


#if TEMP_PID
int temp_update(int dt)
{
  int output;
  int error;
  float pTerm, iTerm, dTerm;
  
  if (extruder_temp_control_enabled) {
    error = extruder_target_temperature - extruder_current_temperature;
    
    pTerm = extruder_temp_pGain * error;
    
    extruder_temp_iState += error;
    extruder_temp_iState = constrain(extruder_temp_iState, extruder_temp_iState_min, extruder_temp_iState_max);
    iTerm = extruder_temp_iGain * extruder_temp_iState;
    
    dTerm = extruder_temp_dGain * (extruder_current_temperature - extruder_temp_dState);
    extruder_temp_dState = extruder_current_temperature;
    
    output = pTerm + iTerm - dTerm;
    output = constrain(output, 0, 255);
  } else {
    output = 0;
  }
  return output;
}
 
void extruder_temp_pid_update_windup()
{
  extruder_temp_iState_min = -TEMP_PID_INTEGRAL_DRIVE_MAX/extruder_temp_iGain;
  extruder_temp_iState_max =  TEMP_PID_INTEGRAL_DRIVE_MAX/extruder_temp_iGain;
}

# else
int temp_update(int dt)
{
  int output;
  
  if (extruder_temp_control_enabled) {
    //put the heater into high mode if we're not at our target.
    if (extruder_current_temperature < extruder_target_temperature)
      output = heater_high;
    //put the heater on low if we're at our target.
    else if (extruder_current_temperature < extruder_max_temperature)
      output = heater_low;
    //turn the heater off if we're above our max.
    else
      output = 0;
  } else {
    output = 0;
  }
  return output;
}
#endif /* TEMP_PID */

