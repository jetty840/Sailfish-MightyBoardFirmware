// Stepperaccelplannerextrasquer.cc
//
// This module fills two roles
//   1. Provide no-op stubs for routines needed by StepperAccelPlanner, and
//   2. Provide additional utility routines needed by the simulated planner

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>

#include "Simulator.hh"
#include "EepromMap.hh"
#include "Steppers.hh"
#include "StepperAccelPlanner.hh"
#include "StepperAccelPlannerExtras.hh"
#include "avrfix.h"

#define min(a,b) (((a)<=(b))?(a):(b))
#define max(a,b) (((a)>=(b))?(a):(b))

uint32_t simulator_debug              = false;
bool     simulator_use_max_feed_rate  = false;
FPTYPE   simulator_max_feed_rate      = 0;
bool     simulator_dump_speeds        = false;
bool     simulator_show_alt_feed_rate = false;

uint32_t z1[100000];
uint32_t z2[100000];
uint32_t iz = 0;

// From Command.cc
int64_t filamentLength[2] = {0, 0};
int64_t lastFilamentLength[2] = {0, 0};
int32_t lastFilamentPosition[2];

// From StepperAccel.cc
static bool deprime_enabled = true;
static bool deprimed[EXTRUDERS];
int16_t extruder_deprime_steps[EXTRUDERS];
bool extrude_when_negative[EXTRUDERS];

// From Steppers.cc
float extruder_only_max_feedrate[EXTRUDERS];
volatile int32_t starting_e_position[2];
bool extruder_deprime_travel;

// From time to time, StepperAccelPlanner.cc wants these for debugging
volatile float zadvance, zadvance2;

// Bins for tallying up how many blocks are planned once, twice, thrice, ...
// A block cannot be planned more time than there are blocks in the pipe line
static int planner_counts[BLOCK_BUFFER_SIZE+1];

// Track total time required to print
static float total_time = 0.0;

// Storage for the plan_record() counters
static int record_add    = 0;
static int record_mul    = 0;
static int record_div    = 0;
static int record_sqrt   = 0;
static int record_calc   = 0;
static int record_recalc = 0;

void plan_record(void *ctx, int item_code, ...)
{
     va_list ap;

     (void)ctx;

     va_start(ap, item_code);
     while (item_code != 0)
     {
	  switch(item_code)
	  {
	  case RECORD_CALC:
	       record_calc += va_arg(ap, int);
	       break;

	  case RECORD_ADD:
	       record_add += va_arg(ap, int);
	       break;

	  case RECORD_MUL:
	       record_mul += va_arg(ap, int);
	       break;

	  case RECORD_DIV:
	       record_div += va_arg(ap, int);
	       break;

	  case RECORD_SQRT:
	       record_sqrt += va_arg(ap, int);
	       break;

	  case RECORD_RECALC:
	       record_recalc += va_arg(ap, int);
	       break;

	  default :
	       goto badness;
	  }
	  item_code = va_arg(ap, int);
     }
badness:
     va_end(ap);
}

extern volatile unsigned char block_buffer_head;           // Index of the next block to be pushed
extern volatile unsigned char block_buffer_tail;           // Index of the block to process now


void st_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b)
{
  CRITICAL_SECTION_START;
  dda_position[X_AXIS] = x;
  dda_position[Y_AXIS] = y;
  dda_position[Z_AXIS] = z;
  dda_position[A_AXIS] = a;
  dda_position[B_AXIS] = b;
#ifdef JKN_ADVANCE
  starting_e_position[0] = dda_position[A_AXIS]; 
  starting_e_position[1] = dda_position[B_AXIS]; 
#endif
  CRITICAL_SECTION_END;
}

void st_set_e_position(const int32_t &a, const int32_t &b)
{
  CRITICAL_SECTION_START;
  dda_position[A_AXIS] = a;
  dda_position[B_AXIS] = b;
#ifdef JKN_ADVANCE
  starting_e_position[0] = dda_position[A_AXIS]; 
  starting_e_position[1] = dda_position[B_AXIS]; 
#endif
  CRITICAL_SECTION_END;
}

int32_t st_get_position(uint8_t axis)
{
  int32_t count_pos;
  CRITICAL_SECTION_START;
  count_pos = dda_position[axis];
  CRITICAL_SECTION_END;
  return count_pos;
}

void st_deprime_enable(bool enable)
{
    deprime_enabled = enable;

    for ( uint8_t i = 0; i < EXTRUDERS; i++ ) {
	deprimed[i] = true;
    }  
}

#if defined(THINGOMATIC)

static uint16_t calc_timer(uint16_t step_rate, int *step_loops)
{
     if (step_rate > MAX_STEP_FREQUENCY)
	  step_rate = MAX_STEP_FREQUENCY;
  
     if (step_rate > 20000)
     {
          // If steprate > 20kHz >> step 4 times
	  step_rate = (step_rate >> 2)&0x3fff;
	  *step_loops = 4;
     }
     else if(step_rate > 10000)
     {
          // If steprate > 10kHz >> step 2 times
	  step_rate = (step_rate >> 1)&0x7fff;
	  *step_loops = 2;
     }
     else
	  *step_loops = 1;

     if (step_rate < 32)
	  step_rate = 32;

     return (uint16_t)((uint32_t)2000000 / (uint32_t)step_rate);
}

#else

#define SHIFT1(x) (uint8_t)(x >> 8)

static uint16_t calc_timer(uint16_t step_rate, int *step_loops)
{
     uint8_t step_rate_high = SHIFT1(step_rate);
  
     if (step_rate_high > SHIFT1(19968)) { // If steprate > 19.968 kHz >> step 8 times
	 if (step_rate_high > SHIFT1(MAX_STEP_FREQUENCY)) // ~39.936 kHz
	     step_rate = (MAX_STEP_FREQUENCY >> 3) & 0x1fff;
	 else
	     step_rate = (step_rate >> 3) & 0x1fff;
	 *step_loops = 8;
     }
     else if (step_rate_high > SHIFT1(9984)) { // If steprate > 9.984 kHz >> step 4 times
	 step_rate = (step_rate >> 2) & 0x3fff;
	 *step_loops = 4;
     }
     else if (step_rate_high > SHIFT1(4864)) { // If steprate > 4.864 kHz >> step 2 times
	 step_rate = (step_rate >> 1) & 0x7fff;
	 *step_loops = 2;
     } else {
	 if (step_rate < 32) step_rate = 32;
	 *step_loops = 1;
     }

     return (uint16_t)((uint32_t)2000000 / (uint32_t)step_rate);
}

#endif

void init_extras(bool accel)
{
     steppers::acceleration = (accel & 0x01) ? true : false;
     steppers::setSegmentAccelState(steppers::acceleration);

     filamentLength[0]       = filamentLength[1]       = 0;
     lastFilamentLength[0]   = lastFilamentLength[1]   = 0;
     lastFilamentPosition[0] = lastFilamentPosition[1] = 0;

#if 0 
     fprintf(stderr, "p_acceleration = %u\n", p_acceleration);
     fprintf(stderr, "p_retract_acceleration = %u\n", p_retract_acceleration);

     for (int i = 0; i < 5; i++)
	     fprintf(stderr, "steps per mm %d = %f\n", i, (float)replicator_axis_steps_per_mm::axis_steps_per_mm[i] /  1000000.0f);
#endif
}

float stepperAxisStepsToMM_(int32_t steps, uint8_t axis)
{
     return ((float)steps * 1000000.0f /
	     (float)replicator_axis_steps_per_mm::axis_steps_per_mm[axis]);
}

int64_t getFilamentLength(uint8_t extruder)
{
     if ( filamentLength[extruder] < 0 )
	  return (-filamentLength[extruder]);
     return (filamentLength[extruder]);
}

int64_t getLastFilamentLength(uint8_t extruder)
{
     if ( lastFilamentLength[extruder] < 0 )
	  return (-lastFilamentLength[extruder]);
     return (lastFilamentLength[extruder]);
}

float filamentUsed(void)
{
     float filamentUsed = 
	  stepperAxisStepsToMM_(getLastFilamentLength(0), A_AXIS) +
	  stepperAxisStepsToMM_(getLastFilamentLength(1), B_AXIS);

     if ( filamentUsed == 0.0 )
	  filamentUsed =
	       stepperAxisStepsToMM_(getFilamentLength(0), A_AXIS) +
	       stepperAxisStepsToMM_(getFilamentLength(1), B_AXIS); 

     return (filamentUsed);
}

#define CHECK_SPEED_CHANGES
#ifdef CHECK_SPEED_CHANGES
static int total_violation_count = 0;
static float total_violation     = 0.0;
static float total_violation_min = 1.0E10;
static float total_violation_max = 0.0;
#endif

void plan_dump_current_block(int discard, int report)
{
     int32_t acceleration_time, coast_time, deceleration_time;
     uint16_t acc_step_rate, dec_step_rate, intermed;
     char action[STEPPER_COUNT+1];
     block_t *block;
     int count_direction[STEPPER_COUNT], step_loops;
     uint32_t initial_rate, step_events_completed;
     static int i = 0;
     uint8_t out_bits;
     static float z_height = 10.0;  // figure z-offset is around 10
     uint16_t timer;
#ifdef CHECK_SPEED_CHANGES
     static float maxd;
     static float prev_speed[STEPPER_COUNT] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
     static const char axes_names[] = "XYZAB";
#endif

     block = plan_get_current_block();
     if (!block)
	  return;

     if (report && block->message[0] != '\0')
	  printf("%s", block->message);

     action[0] = (block->steps[X_AXIS] != 0) ?
	  (((uint32_t)(0x7fffffff & block->steps[X_AXIS]) == block->step_event_count) ? 'X' : 'x') : ' ';
     action[1] = (block->steps[Y_AXIS] != 0) ?
	  (((uint32_t)(0x7fffffff & block->steps[Y_AXIS]) == block->step_event_count) ? 'Y' : 'y') : ' ';
     action[2] = (block->steps[Z_AXIS] != 0) ?
	  (((uint32_t)(0x7fffffff & block->steps[Z_AXIS]) == block->step_event_count) ? 'Z' : 'z') : ' ';
     action[3] = (block->steps[A_AXIS] != 0) ?
	  (((uint32_t)(0x7fffffff & block->steps[A_AXIS]) == block->step_event_count) ? 'A' : 'a') : ' ';
     action[4] = (block->steps[B_AXIS] != 0) ?
	  (((uint32_t)(0x7fffffff & block->steps[B_AXIS]) == block->step_event_count) ? 'B' : 'b') : ' ';
     action[5] = '\0';

     if (block->acceleration_rate == 0)
     {
	     // No acceleration
	     initial_rate  = block->nominal_rate;
	     acc_step_rate = block->nominal_rate;
	     dec_step_rate = block->nominal_rate;
	     acceleration_time = calc_timer(acc_step_rate, &step_loops) * block->step_event_count;
	     deceleration_time = 0;
	     coast_time        = 0;
	     step_events_completed = block->step_event_count;
     }
     else
     {
	     initial_rate      = block->initial_rate;
	     acc_step_rate     = block->initial_rate;
	     acceleration_time = calc_timer(acc_step_rate, &step_loops);

	     dec_step_rate     = 0;
	     deceleration_time = 0;

	     coast_time        = 0;
	     intermed          = 0;

	     for (step_events_completed = 0; step_events_completed <= block->step_event_count; )
	     {
		     step_events_completed += step_loops;
		     if (step_events_completed <= (uint32_t)(0x7fffffff & block->accelerate_until))
		     {
			     // speed(t) = speed(0) + acceleration * t
			     uint16_t old_acc_step_rate = acc_step_rate;
			     uint16_t intermed_a;
			     acc_step_rate = intermed_a =
				     (uint16_t)((0xffffffffff & ((uint64_t)(0x00ffffff & acceleration_time) * 
								 (uint64_t)(0x00ffffff & block->acceleration_rate))) >> 24);
			     acc_step_rate += block->initial_rate;
			     if (acc_step_rate < old_acc_step_rate && report)
				     printf("*** While accelerating, the step rate overflowed: "
					    "acc_step_rate = %u = %u + %u = %u + 0x%x * 0x%x\n",
					    acc_step_rate, block->initial_rate,
					    intermed_a, block->initial_rate,
					    block->acceleration_rate,
					    acceleration_time);
			     if (acc_step_rate > block->nominal_rate)
				     acc_step_rate = block->nominal_rate;
			     acceleration_time += timer = calc_timer(acc_step_rate, &step_loops);
			     dec_step_rate = acc_step_rate;
		     }
		     else if (step_events_completed > (uint32_t)(0x7fffffff & block->decelerate_after))
		     {
			     // speed(t) = speed(0) - deceleration * t
			     uint16_t old_intermed = intermed;
			     intermed =
				     (uint16_t)((0xffffffffff & ((uint64_t)(0x00ffffff & deceleration_time) * 
								 (uint64_t)(0x00ffffff & block->acceleration_rate))) >> 24);
			     if (intermed > acc_step_rate)
				     dec_step_rate = block->final_rate;
			     else
				     dec_step_rate = acc_step_rate - intermed;
			     if (dec_step_rate < block->final_rate)
				     dec_step_rate = block->final_rate;
			     if (intermed < old_intermed && report)
				     printf("*** While decelerating, the step rate overflowed: "
					    "%u = %u - %u = %u - 0x%x * 0x%x\n",
					    dec_step_rate, acc_step_rate, intermed,
					    acc_step_rate, block->acceleration_rate,
					    deceleration_time);
			     deceleration_time += calc_timer(dec_step_rate, &step_loops);
		     }
		     else
		     {
			     // Must make this call as it has side effects
			     coast_time += calc_timer(acc_step_rate, &step_loops);
			     dec_step_rate = acc_step_rate;
		     }
	     }
     }

     out_bits = block->direction_bits;
     count_direction[X_AXIS] = ((out_bits & (1<<X_AXIS)) != 0) ? -1 : 1;
     count_direction[Y_AXIS] = ((out_bits & (1<<Y_AXIS)) != 0) ? -1 : 1;
     count_direction[Z_AXIS] = ((out_bits & (1<<Z_AXIS)) != 0) ? -1 : 1;
     count_direction[A_AXIS] = ((out_bits & (1<<A_AXIS)) != 0) ? -1 : 1;
     count_direction[B_AXIS] = ((out_bits & (1<<B_AXIS)) != 0) ? -1 : 1;

#ifdef CHECK_SPEED_CHANGES
     maxd = 0.0f;
     for (int j = 0; j < STEPPER_COUNT; j++)
     {
	     float delta, s, speed;

	     s = (float)block->steps[j] / (float)block->step_event_count;
	     if (count_direction[j] < 0)
		     s = -s;
	     speed = s * stepperAxisStepsToMM_(initial_rate, (uint8_t)j);
	     delta = fabs(speed - prev_speed[j]) - FPTOF(max_speed_change[j]);
	     if (delta > 0.1f && report)
	     {
		     printf("Max speed change of %f for %c axis exceeded going into this move; change is %f + max speed change\n",
			    FPTOF(max_speed_change[j]), (char)axes_names[j], delta); 
		     if (delta > maxd)
			     maxd = delta;
	     }
	     prev_speed[j] = s * stepperAxisStepsToMM_(dec_step_rate, (uint8_t)j);
     }
     if (maxd > 0.1f)
     {
	     // maxd isn't really the smallest
	     if (maxd < total_violation_min)
		     total_violation_min = maxd;
	     if (maxd > total_violation_max)
		     total_violation_max = maxd;
	     total_violation += maxd;
	     total_violation_count += 1;
     }
#endif

     if (block->steps[Z_AXIS] != 0)
     {
	  float delta_z = block->steps[Z_AXIS] * FPTOF(steppers::axis_steps_per_unit_inverse[Z_AXIS]);
	  z_height += count_direction[Z_AXIS] * delta_z;
	  if (iz < sizeof(z1)/sizeof(uint32_t))
	  {
		  z1[iz] = block->initial_rate;
		  z2[iz] = acc_step_rate;
		  iz++;
	  }
     }

     i++;
     if (report)
     {
	 if (simulator_dump_speeds)
	 {
	     float total_time = (float)(acceleration_time + coast_time + deceleration_time /*- last_time */) / 2000000.0;
	     float speed_xyze = FPTOF(block->millimeters)/total_time;
	     float dx = stepperAxisStepsToMM_(block->steps[X_AXIS], X_AXIS);
	     float dy = stepperAxisStepsToMM_(block->steps[Y_AXIS], Y_AXIS);
	     float dz = stepperAxisStepsToMM_(block->steps[Z_AXIS], Z_AXIS);
	     // float de = stepperAxisStepsToMM_(block->steps[A_AXIS], A_AXIS);
	     float speed_xyz = sqrt(dx*dx+dy*dy+dz*dz) / total_time;

	     printf("%d %s: z=%4.1f entry=%5u, peak=%5d, final=%5d steps/s; planned=%d; "
		    "feed_rate=%6.2f mm/s; xyze-dist/t=%6.2f, xyz-dist/t=%6.2f mm/s\n",
		    i, action, z_height, initial_rate, acc_step_rate, dec_step_rate,
		    block->planned, FPTOF(block->feed_rate), speed_xyze, speed_xyz);
	 }
	 else
	     printf("%d %s: z=%4.1f entry=%5u, peak=%5d, final=%5d steps/s; planned=%d; "
		    "feed_rate=%6.2f mm/s (x/y/z/a/b=%d/%d/%d/%d/%d); filament used=%6.1f\n",
		    i, action, z_height, initial_rate, acc_step_rate,
		    dec_step_rate, block->planned, FPTOF(block->feed_rate),
		    count_direction[X_AXIS]*block->steps[X_AXIS],
		    count_direction[Y_AXIS]*block->steps[Y_AXIS],
		    count_direction[Z_AXIS]*block->steps[Z_AXIS],
		    count_direction[A_AXIS]*block->steps[A_AXIS],
		    count_direction[B_AXIS]*block->steps[B_AXIS],
		    filamentUsed());
     }

     planner_counts[max(0, min(block->planned, BLOCK_BUFFER_SIZE))] += 1;
     total_time += (float)(acceleration_time + coast_time + deceleration_time) / 2000000.0;

     if (discard)
	  plan_discard_current_block();
}

void plan_dump_run_data(int time_only)
{
     int cnt, ihours, imins, isecs, idsecs;
     unsigned i;
     float ttime = total_time;
     uint32_t ztot1, zavg_min1, zavg_max1;
     uint32_t ztot2, zavg_min2, zavg_max2;
     float zavg1, zavg2;

     ihours = (int)(ttime / (60.0 * 60.0));
     ttime -= (float)(ihours * 60 * 60);
     imins = (int)(ttime / 60.0);
     ttime -= (float)(imins * 60);
     isecs = (int)ttime;
     ttime -= (float)isecs;
     idsecs = (int)(0.5 + ttime * 100.0);
     printf("Total print time is %02d:%02d:%02d.%02d (%f seconds)\n",
	    ihours, imins, isecs, idsecs, total_time);

     if (time_only)
	     return;

#ifdef CHECK_SPEED_CHANGES
     if (total_violation_count > 0)
     {
	     total_violation = total_violation / (float)total_violation_count;
	     printf("%d max speed change violations; excess beyond max speed change Min / Average / Max = %f / %f / %f mm/s\n",
		    total_violation_count, total_violation_min, total_violation, total_violation_max);
     }
     else
     {
	     printf("No violations of the maximum speed changes seen\n");
     }
#endif

     printf("Planner counts:\n");
     for (i = 0; i <= BLOCK_BUFFER_SIZE; i++)
	  if (planner_counts[i])
	       printf("    %d: %d\n", i, planner_counts[i]);

     memset(planner_counts, 0, sizeof(planner_counts));

     ztot1 = 0.0;
     ztot2 = 0.0;
     zavg_min1 = z1[2];
     zavg_max1 = z1[2];
     zavg_min2 = z2[2];
     zavg_max2 = z2[2];
     cnt = 0;
     for (i = 2; i < (iz - 2); i++)
     {
	  cnt++;
	  if (z1[i] < zavg_min1) zavg_min1 = z1[i];
	  if (z1[i] > zavg_max1) zavg_max1 = z1[i];
	  if (z2[i] < zavg_min2) zavg_min2 = z2[i];
	  if (z2[i] > zavg_max2) zavg_max2 = z2[i];
	  ztot1 += z1[i];
	  ztot2 += z2[i];
     }
     zavg1 = (float)ztot1 / (float)cnt;
     zavg2 = (float)ztot2 / (float)cnt;
     printf("Min / Average / Max entry speed = %d / %f / %d steps/s\n",
	    zavg_min1, zavg1, zavg_max1);
     printf("Min / Average / Max peak speed = %d / %f / %d steps/s\n",
	    zavg_min2, zavg2, zavg_max2);
}

void plan_block_notice(const char *fmt, ...)
{
     va_list ap;
     block_t *block;
     uint8_t index;
     size_t len;

     va_start(ap, fmt);

     index = (block_buffer_head == 0) ? BLOCK_BUFFER_SIZE - 1 : block_buffer_head - 1;
     block = &block_buffer[index];

     len = strlen(block->message);
     vsnprintf(block->message + len, sizeof(block->message) - len, fmt, ap);

     va_end(ap);
}

FPTYPE ftofpS(float x, int lineno, const char *src)
{
    if (x > 32767.0f || x < -32768.0f)
	 printf(">>> OVERFLOW: FTOFP(%f) call on line %d pf %s is suspect; "
		"the value %f is too large for an FPTYPE <<<\n",
		x, lineno, src ? src : "???", x);
    return ftok(x);
}

FPTYPE itofpS(int32_t x, int lineno, const char *src)
{
    if (x > 0x7fff || x < -0x8000)
	 printf(">>> OVERFLOW: IPTOF(%d) call on line %d of %s is suspect; "
		"the value %d is too large for an FPTYPE <<<\n",
		x, lineno, src ? src : "???", x);
    return itok(x);
}

FPTYPE fpsquareS(FPTYPE x, int lineno, const char *src)
{
    double z = ktof(x) * ktof(x); 
    if (z > 32767.0f)
	 printf(">>> OVERFLOW: FPSQUARE(%f) call on line %d of %s is suspect; "
		"the value %f * %f is too large for an FPTYPE <<<\n",
		ktof(x), lineno, src ? src : "???", ktof(x), ktof(x));
    return mulk(x, x);
}

FPTYPE fpmult2S(FPTYPE x, FPTYPE y, int lineno, const char *src)
{
     double z = ktof(x) * ktof(y);
     if (z > 32767.0f || z < -32768.0f)
	 printf(">>> OVERFLOW: FPMULT2(%f, %f) call on line %d of %s is suspect; "
		"the product %f * %f is too large for an FPTYPE <<<\n",
		ktof(x), ktof(y), lineno, src ? src : "???", ktof(x), ktof(y));
     return mulk(x, y);
}

FPTYPE fpmult3S(FPTYPE x, FPTYPE y, FPTYPE a, int lineno, const char *src)
{
     double z = ktof(x) * ktof(y) * ktof(a);
     if (z > 32767.0f || z < -32768.0f)
	 printf(">>> OVERFLOW: FPMULT3(%f, %f, %f) call on line %d of %s is suspect; "
		"the product %f * %f * %f is too large for an FPTYPE <<<\n",
		ktof(x), ktof(y), ktof(a), lineno, src ? src : "???", ktof(x), ktof(y), ktof(a));
     return mulk(mulk(x, y), a);
}

FPTYPE fpmult4S(FPTYPE x, FPTYPE y, FPTYPE a, FPTYPE b, int lineno, const char *src)
{
     double z = ktof(x) * ktof(y) * ktof(a) * ktof(b);
     if (z > 32767.0f || z < -32768.0f)
	 printf(">>> OVERFLOW: FPMULT4(%f, %f, %f, %f) call on line %d of %s is suspect; "
		"the product %f * %f * %f * %f is too large for an FPTYPE <<<\n",
		ktof(x), ktof(y), ktof(a), ktof(b), lineno, src ? src : "???",
		ktof(x), ktof(y), ktof(a), ktof(b));
     return mulk(mulk(mulk(x, y), a), b);
}

FPTYPE fpdivS(FPTYPE x, FPTYPE y, int lineno, const char *src)
{
     double z = ktof(x) / ktof(y);
     if (z > 32767.0f || z < -32768.0f)
	 printf(">>> OVERFLOW: FPDIV(%f, %f) call on line %d of %s is suspect; "
		"%f / %f is too large for an FPTYPE <<<\n",
		ktof(x), ktof(y), lineno, src ? src : "???", ktof(x), ktof(y));
     return divk(x, y);
}

FPTYPE fpscale2S(FPTYPE x, int lineno, const char *src)
{
     double z = ktof(x) * 2.0;
     if (z > 32767.0f || z < -32768.0f)
	  printf(">>> OVERFLOW: FPSCALE(%f) call on line %d of %s is suspect; "
		 "%f << 1 is too large for an FPTYPE <<<\n",
		 ktof(x), lineno, src ? src : "???", ktof(x));
     return x << 1;
}

namespace eeprom {

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value) { return default_value; }
uint16_t getEeprom16(const uint16_t location, const uint16_t default_value) { return default_value; }
uint32_t getEeprom32(const uint16_t location, const uint32_t default_value) { return default_value; }
float getEepromFixed16(const uint16_t location, const float default_value) { return default_value; }
void setEepromFixed16(const uint16_t location, const float new_value) { }
int64_t getEepromInt64(const uint16_t location, const int64_t default_value) { return default_value; }
void setEepromInt64(const uint16_t location, const int64_t value) { }
void storeToolheadToleranceDefaults() { }
void setDefaultsAcceleration() { }

}

#ifdef linux

size_t strlcat(char *dst, const char *src, size_t size)
{
     size_t copy_len, dst_len, remaining, src_len;

     if (!dst || !src)
     {
	  errno = EFAULT;
	  return 0;
     }

     dst_len = strlen(dst);
     src_len = strlen(src);

     if (size <= (dst_len + 1))
	  goto done;

     remaining = size - (dst_len + 1);

     if (src_len < remaining)
	  copy_len = src_len;
     else
	  copy_len = remaining;

     // +1 byte to pick up NUL
     memcpy(dst + dst_len, src, copy_len + 1);

     // Not quite correct: not handling the case where there's
     // no NUL in the first size bytes of dst or src.
done:
     return dst_len + src_len;
}

#endif
