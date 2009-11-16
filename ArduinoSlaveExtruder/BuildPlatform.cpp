#include "Configuration.h"
#include "BuildPlatform.h"
#include "Variables.h"
#include "ThermistorTable.h"
#include "Heater.h" // for sample_temperature

void set_platform_temperature(int temp)
{
  platform_target_temperature = temp;
  platform_max_temperature = (int)((float)temp * 1.1);
}

#ifdef PLATFORMTHERMISTOR_PIN
int read_platform_thermistor();
#endif // PLATFORMTHERMISTOR_PIN

#ifdef PLATFORMTHERMOCOUPLE_PIN
int read_platform_thermocouple();
#endif // PLATFORMTHERMOCOUPLE_PIN

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int get_platform_temperature()
{
#ifdef PLATFORMTHERMISTOR_PIN
  return read_platform_thermistor();
#endif
#ifdef PLATFORMTHERMOCOUPLE_PIN
  return read_platform_thermocouple();
#endif
}

/*
* This function gives us the temperature from the thermistor in Celsius
 */
#ifdef PLATFORMTHERMISTOR_PIN
int read_platform_thermistor()
{
  int raw = sample_temperature(PLATFORMTHERMISTOR_PIN);

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
#ifdef PLATFORMTHERMOCOUPLE_PIN
int read_platform_thermocouple()
{
  return ( 5.0 * sample_temperature(PLATFORMTHERMOCOUPLE_PIN) * 100.0) / 1024.0;
}
#endif

int platform_temp_update(int dt);

/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void manage_platform_temperature()
{
  int output, dt;
  unsigned long time;

  //make sure we know what our temp is.
  platform_current_temperature = get_platform_temperature();
    
  // ignoring millis rollover for now
  time = millis();
  dt = time - platform_temp_prev_time;

  if (dt > TEMP_UPDATE_INTERVAL)
  { 
    platform_temp_prev_time = time;
    output = platform_temp_update(dt);
    analogWrite(PLATFORMHEATER_PIN,output);
  }
}


#if TEMP_PID
int platform_temp_update(int dt)
{
  int output;
  int error;
  float pTerm, iTerm, dTerm;
  
  if (platform_temp_control_enabled) {
    error = platform_target_temperature - platform_current_temperature;
    
    pTerm = platform_temp_pGain * error;
    
    platform_temp_iState += error;
    platform_temp_iState = constrain(platform_temp_iState, platform_temp_iState_min, platform_temp_iState_max);
    iTerm = platform_temp_iGain * platform_temp_iState;
    
    dTerm = platform_temp_dGain * (platform_current_temperature - platform_temp_dState);
    platform_temp_dState = platform_current_temperature;
    
    output = pTerm + iTerm - dTerm;
    output = constrain(output, 0, 255);
  } else {
    output = 0;
  }
  return output;
}
 
void platform_temp_pid_update_windup()
{
  platform_temp_iState_min = -TEMP_PID_INTEGRAL_DRIVE_MAX/platform_temp_iGain;
  platform_temp_iState_max =  TEMP_PID_INTEGRAL_DRIVE_MAX/platform_temp_iGain;
}

# else
int platform_temp_update(int dt)
{
  int output;
  
  if (platform_temp_control_enabled) {
    //put the heater into high mode if we're not at our target.
    if (platform_current_temperature < platform_target_temperature)
      output = heater_high;
    //put the heater on low if we're at our target.
    else if (platform_current_temperature < platform_max_temperature)
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

