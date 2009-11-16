#ifndef VARIABLES_H
#define VARIABLES_H

#include "Configuration.h"
#include "Datatypes.h"
#include <Servo.h>
#include "WProgram.h"

//this is the version of our host software
extern unsigned int master_version;

//are we paused?
extern bool is_tool_paused;

// Two Temp Zones
// 1. Extruder
extern int extruder_current_temperature;
extern int extruder_target_temperature;
extern int extruder_max_temperature;

// 2. Platform
extern int platform_current_temperature;
extern int platform_target_temperature;
extern int platform_max_temperature;

extern int heater_low;
extern int heater_high;

extern MotorControlStyle motor1_control;
extern volatile MotorControlDirection motor1_dir;
extern volatile byte motor1_pwm;
extern volatile long motor1_target_rpm;
extern volatile long motor1_current_rpm;
extern bool motor1_reversal_state;
extern int motor1_reversal_count;

extern MotorControlStyle motor2_control;
extern MotorControlDirection motor2_dir;
extern byte motor2_pwm;
extern long motor2_target_rpm;
extern long motor2_current_rpm;

extern Servo servo1;
extern Servo servo2;

//these are for the extruder PID
extern volatile int speed_error;        // extruder position / error variable.
extern volatile int pGain;              // Proportional gain
extern volatile int iGain;              // Integral gain
extern volatile int dGain;              // Derivative gain
extern volatile int iMax;               // Integrator max
extern volatile int iMin;               // Integrator min
extern volatile int iState;             // Integrator state
extern volatile int dState;             // Last position input

//variables to keep track of stepper state.
const byte coil_a_enabled   = B10011001;
const byte coil_a_direction = B11000011;
const byte coil_b_enabled   = B01100110;
const byte coil_b_direction = B11110000;

//what state are we in?
extern volatile byte stepper_index;
extern volatile long stepper_ticks;
extern volatile byte stepper_high_pwm;
extern volatile byte stepper_low_pwm;

// Two Temp Zones

// 1. Extruder
extern boolean extruder_temp_control_enabled;
extern unsigned long extruder_temp_prev_time; // ms

#if TEMP_PID
extern float extruder_temp_pGain;
extern float extruder_temp_iGain;
extern float extruder_temp_dGain;

extern int  extruder_temp_dState;
extern long extruder_temp_iState;
extern float extruder_temp_iState_max; // set in update_windup
extern float extruder_temp_iState_min; // set in update_windup
#endif

// 2. Platform
extern boolean platform_temp_control_enabled;
extern unsigned long platform_temp_prev_time; // ms

#if TEMP_PID
extern float platform_temp_pGain;
extern float platform_temp_iGain;
extern float platform_temp_dGain;

extern int  platform_temp_dState;
extern long platform_temp_iState;
extern float platform_temp_iState_max; // set in update_windup
extern float platform_temp_iState_min; // set in update_windup
#endif

#endif // VARIABLES_H
