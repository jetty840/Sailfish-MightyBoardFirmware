#ifndef __SKEW_TILT_HH__

#if defined(AUTO_LEVEL)

#include "Point.hh"

extern bool skew_active;

extern int32_t skew(const int32_t P[]);
extern bool skew_init(const int32_t P1[], const int32_t P2[], const int32_t P3[]);
extern void skew_deinit(void);

#if defined(AUTO_LEVEL_TILT)

extern Point tilt(Point &P);
extern Point tilt_inverse(Point &P);
extern bool tilt_init(Point &P1, Point &P2, Point &P3);

#endif

#endif

#endif
