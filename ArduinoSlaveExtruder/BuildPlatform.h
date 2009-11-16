#ifndef BUILDPLATFORM_H
#define BUILDPLATFORM_H

#include <stdint.h>

void set_platform_temperature(int temp);

void manage_platform_temperature();
void platform_temp_pid_update_windup();

#endif // BUILDPLATFORM_H
