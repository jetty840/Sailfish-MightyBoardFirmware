#ifndef POINT_HH
#define POINT_HH

#include "Configuration.hh"
#include <stdint.h>

#if STEPPER_COUNT > 3
#define AXIS_COUNT STEPPER_COUNT
#else
#define AXIS_COUNT 3
#endif

/// Class that represents an N-dimensional point, where N is the number of
/// stepper axes present in the system. Can support 3 or 5 axes.
class Point {
public:
#if AXIS_COUNT >= 5
	int32_t coordinates[5];        ///< n-dimensional coordinate
#else
	int32_t coordinates[AXIS_COUNT];        ///< n-dimensional coordinate
#endif

	/// Default point constructor
	Point();

	/// Construct a point with the given cooridnates. Coordinates are in
	/// stepper steps.
	/// \param[in] x X axis position
	/// \param[in] y Y axis position
	/// \param[in] z Z axis position
	/// \param[in] a (if supported) A axis position
	/// \param[in] b (if supported) B axis position
#if AXIS_COUNT >= 5
	Point(int32_t x, int32_t y, int32_t z, int32_t a = 0, int32_t b = 0);
#elif AXIS_COUNT == 4
	Point(int32_t x, int32_t y, int32_t z, int32_t a = 0);
#else
	Point(int32_t x, int32_t y, int32_t z);
#endif

	/// Constant array accessor.
	/// \param[in] index Axis to look up
	/// \return Axis position, in steps
	const int32_t& operator[](unsigned int index) const;

	/// Array accessor.
	/// \param[in] index Axis to look up
	/// \return Reference to the variable containing the axis' position.
	int32_t& operator[](unsigned int index);

	/// Subtraction operator, for fast deltas
	friend Point operator- (const Point &a, const Point &b);

	/// Addition operator, for offsets
	friend Point operator+ (const Point &a, const Point &b);

	/// Absolute value -- convert all point to positive
	Point abs();
} __attribute__ ((__packed__));


#endif // POINT_HH
