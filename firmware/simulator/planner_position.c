#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "planner.h"
#include "planner_position.h"

unsigned long position_axis_mask = AXES_MASK;
int position[NAXES] = {0, 0, 0, 0, 0};
bool position_known = false;

void s3g_position_init(void)
{
     int i;

     for (i = 0; i < NAXES; i++)
	  position[i] = 0;

     position_known     = false;
     position_axis_mask = AXES_MASK;

     return;
}

void s3g_position_mark_unknown(uint8_t mask)
{
     int i;

     for (i = 0; i < NAXES; i++)
	  if (mask & (1 << i))
	       position_axis_mask |= (1 << i);

     position_known = (position_axis_mask & AXES_MASK) == 0x00;

     return;
}

void s3g_position_mark_known(int axis, int coordinate)
{
     if (axis < 0 || axis >= NAXES)
	  return;

     position[axis] = coordinate;
     position_axis_mask &= ~(1 << axis);
     position_known = (position_axis_mask & AXES_MASK) == 0x00;

     return;
}

void s3g_position_set(int *current)
{
     if (!current)
	  return;

     memcpy(position, current, sizeof(int)*NAXES);
     position_axis_mask = 0;
     position_known     = true;
}
