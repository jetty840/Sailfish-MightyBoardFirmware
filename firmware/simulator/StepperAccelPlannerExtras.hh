// Declarations for various routines needed by StepperAccelPlanner.cc
// which we would prefer it not to get from elsewhere

#ifndef STEPPERACCELPLANNEREXTRAS_HH_

#define _STEPPERACCELPLANNEREXTRAS_HH_

#include "Point.hh"
#include "SimulatorRecord.hh"
#include "StepperAccelPlanner.hh"
#include "StepperAccel.hh"

// Debugging switch & bit flags
extern uint32_t simulator_debug;
#define DEBUG_FEEDRATE 0x01

extern bool   simulator_dump_speeds;
extern bool   simulator_use_max_feed_rate;
extern bool   simulator_show_alt_feed_rate;
extern FPTYPE simulator_max_feed_rate;

extern void init_extras(bool acceleration);
extern void st_set_position(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &a, const int32_t &b);
extern void st_set_e_position(const int32_t &a, const int32_t &b);
extern int32_t st_get_position(uint8_t axis);
extern void st_deprime_enable(bool enable);
extern void plan_dump(int chart);
extern void plan_dump_current_block(int discard);
extern void plan_dump_run_data(void);
void plan_block_notice(const char *fmt, ...);

#endif
