#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>

void set_extruder_temperature(int temp);
int sample_temperature(uint8_t pin);

void manage_extruder_temperature();
void extruder_temp_pid_update_windup();

#endif // HEATER_H
