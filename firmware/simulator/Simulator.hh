// Simulator.hh
// Handle porting issues between avr-gcc and gcc

#ifndef SIMULATOR_HH_

#define SIMULATOR_HH

#ifdef SIMULATOR

#include <inttypes.h>
#include "avrfix.h"

#ifdef linux
#define __need_size_t
#include <stddef.h>
#undef __need_size_t
#endif

#ifndef _BV
#define _BV(x) (1 << (x))
#endif

#ifndef FPTYPE
#define FPTYPE _Accum
#endif

#ifndef FORCE_INLINE
#define FORCE_INLINE inline
#endif

// avr-gcc makes double the same as float
#define double float

// Maybe at some point in the future, we'll want to replace these
// with pthread mutices.  That, if it becomes desirable to simulate
// interrupts pulling information out of the pipeline: use a thread
// to simulate an interrupt and have the planner running in the
// primal thread, possibly at a lower thread priority.

#define CRITICAL_SECTION_START  {}
#define CRITICAL_SECTION_END    {}

// Seems like a good idea, eh?
#ifndef HAS_STEPPER_ACCELERATION
#define HAS_STEPPER_ACCELERATION
#endif

extern FPTYPE ftofpS(float x, int lineno, const char *src);
extern FPTYPE itofpS(int32_t x, int lineno, const char *src);
extern FPTYPE fpsquareS(FPTYPE x, int lineno, const char *src);
extern FPTYPE fpmult2S(FPTYPE x, FPTYPE y, int lineno, const char *src);
extern FPTYPE fpmult3S(FPTYPE x, FPTYPE y, FPTYPE a, int lineno, const char *src);
extern FPTYPE fpmult4S(FPTYPE x, FPTYPE y, FPTYPE a, FPTYPE b, int lineno, const char *src);
extern FPTYPE fpdivS(FPTYPE x, FPTYPE y, int lineno, const char *src);
extern FPTYPE fpsqrtS(FPTYPE x, int lineno, const char *src);
extern FPTYPE fpabsS(FPTYPE x, int lineno, const char *src);
extern FPTYPE fpscale2S(FPTYPE x, int lineno, const char *src);

#ifdef linux
extern size_t strlcat(char *dst, const char *src, size_t size);
#endif

#endif

#endif
