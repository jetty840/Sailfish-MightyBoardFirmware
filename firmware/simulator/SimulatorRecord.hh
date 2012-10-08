#ifndef SIMULATORRECORD_HH_

#define SIMULATORRECORD_HH_

// Item codes for use with plan_record()
#define RECORD_ADD    1  // Record an addition or subtraction op
#define RECORD_MUL    2  // Record a multiplication op
#define RECORD_DIV    3  // Record a division op
#define RECORD_SQRT   4  // Record a square root op
#define RECORD_CALC   5  // Record a calculation op
#define RECORD_RECALC 6  // Record a re-calculation op

// This macro is used in StepperAccelPlanner.cc to record
// operations.  When SIMULATOR is defined, it actually calls
// down to plan_record().  Otherwise, it is a no-op as set
// in StepperAccelPlanner.hh

#ifdef SIMULATOR_RECORD
#undef SIMULATOR_RECORD
#endif

#define SIMULATOR_RECORD(x...) plan_record(NULL, ## x, 0)

// void plan_record(void *ctx, int item_code, ...)
//
// Record an operation such as an addition or multiply.  Intended usage is
//
//     plan_record(ctx, item_code_1, count_1 [, item_code_2, count_2, ...], 0);
//
// where each item code is one of the RECORD_ constants such as RECORD_ADD
// which names an operation to record, and count is the number of occurrences
// of that operation.  A final value of 0 should be used to terminate the
// list of item codes and counts.
//
// Call arguments:
//
//   void *ctx
//     Reserved for future use.  A placeholder for now.
//
//   int item_code
//     The type of operation to record.  Must be one of the RECORD_
//     constants.  Each RECORD_ item code must be followed by a single
//     integer value of type "int".  The list must be terminated by a
//     final int argument of value 0.
//
// Return values: none

extern void plan_record(void *ctx, int item_code, ...);

#endif
