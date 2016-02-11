#include "Compat.hh"
#include "Point.hh"
#include <stdlib.h> // for labs()

/* Setup some utilities -- TODO: Move this to a common file */

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#ifdef labs
#undef labs
#endif

template <typename T>
inline T abs(T x) { return (x)>0?(x):-(x); }

template <>
inline int abs(int x) { return __builtin_abs(x); }

template <>
inline long abs(long x) { return __builtin_labs(x); }


/* end utilities */

Point::Point()
{
	// coordinates[0] = 0;
	// coordinates[1] = 0;
	// coordinates[2] = 0;
	// coordinates[3] = 0;
	// coordinates[4] = 0;
}

#if AXIS_COUNT >= 5

Point::Point(const int32_t x, const int32_t y, const int32_t z,
			 const int32_t a, const int32_t b)
{
	coordinates[0] = x;
	coordinates[1] = y;
	coordinates[2] = z;
	coordinates[3] = a;
	coordinates[4] = b;
}

#elif AXIS_COUNT >= 4

Point::Point(const int32_t x, const int32_t y, const int32_t z, const int32_t a)
{
	coordinates[0] = x;
	coordinates[1] = y;
	coordinates[2] = z;
	coordinates[3] = a;
}

#endif

const int32_t& Point::operator[](const unsigned int index) const {
	return coordinates[index];
}

int32_t& Point::operator[](const unsigned int index) {
	return coordinates[index];
}

/// Subtraction operator, for fast deltas
Point operator- (const Point &a, const Point &b) {
	Point c = Point(
		a.coordinates[0] - b.coordinates[0],
		a.coordinates[1] - b.coordinates[1],
		a.coordinates[2] - b.coordinates[2],
#if AXIS_COUNT >= 5
		a.coordinates[3] - b.coordinates[3],
		a.coordinates[4] - b.coordinates[4]
#elif AXIS_COUNT == 4
		a.coordinates[3] - b.coordinates[3]
#endif
	);
	return c;
}

/// Addition operator, for offsets
Point operator+ (const Point &a, const Point &b) {
	Point c = Point(
		a.coordinates[0] + b.coordinates[0],
		a.coordinates[1] + b.coordinates[1],
		a.coordinates[2] + b.coordinates[2],
#if AXIS_COUNT >= 5
		a.coordinates[3] + b.coordinates[3],
		a.coordinates[4] + b.coordinates[4]
#elif AXIS_COUNT == 4
		a.coordinates[3] + b.coordinates[3]
#endif
	);
	return c;
}

Point Point::abs() {
	Point absPoint = Point(
		::abs(coordinates[0]),
		::abs(coordinates[1]),
		::abs(coordinates[2]),
#if AXIS_COUNT >= 5
		::abs(coordinates[3]),
		::abs(coordinates[4])
#elif AXIS_COUNT == 4
		::abs(coordinates[3])
#endif
	);
	return absPoint;
}
