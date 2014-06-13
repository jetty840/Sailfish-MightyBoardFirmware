#ifndef __SKEW_TILT_HH__
#define __SKEW_TILT_HH__

#if defined(AUTO_LEVEL)

#include "Point.hh"

// Should fit in int8_t; must be less than 0

#define ALEVEL_NOT_ACTIVE -1 // Auto-leveling not active
#define ALEVEL_BAD_LEVEL  -2 // Auto-leveling failed; too much diff in Z heights
                             //   or surface normal is perpendicular to Z axis
#define ALEVEL_COLINEAR   -3 // Auto-leveling failed: probe points are colinear

extern bool skew_active;

extern int32_t skew(const int32_t *P);
extern bool skew_init(int32_t maxz, const int32_t *P1, const int32_t *P2,
		      const int32_t *P3);
extern void skew_update(const int32_t *delta);
extern void skew_deinit(void);

// Returns -1 if skew not active
// Otherwise, returns the max Z difference (in steps) between probed points
extern int32_t skew_status(void);

#if defined(AUTO_LEVEL_TILT)

extern Point tilt(Point &P);
extern Point tilt_inverse(Point &P);
extern bool tilt_init(Point &P1, Point &P2, Point &P3);

#endif

#endif

#endif
