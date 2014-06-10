#ifndef __SKEW_TILT_HH__
#define __SKEW_TILT_HH__

#if defined(AUTO_LEVEL)

#include "Point.hh"

extern int32_t skew(Point &P);
extern int32_t skew_inverse(Point &P);
extern bool skew_init(Point &P1, Point &P2, Point &P3);

#if defined(AUTO_LEVEL_TILT)

extern Point tilt(Point &P);
extern Point tilt_inverse(Point &P);
extern bool tilt_init(Point &P1, Point &P2, Point &P3);

#endif

#endif

#endif
