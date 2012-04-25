#ifndef POINT_HH
#define POINT_HH

#include "Configuration.hh"
#include <stdint.h>


#if STEPPER_COUNT > 3
#define AXIS_COUNT 5
#else
#define AXIS_COUNT 3
#endif

/// Class that represents an N-dimensional point, where N is the number of
/// stepper axes present in the system. Can support 3 or 5 axes.
class Point {
private:
	int32_t coordinates[AXIS_COUNT];        ///< n-dimensional coordinate
public:
	/// Default point constructor
	Point();

		/// Copy Point constructor
		// Point(const Point &other);

	/// Construct a point with the given cooridnates. Coordinates are in
	/// stepper steps.
	/// \param[in] x X axis position
	/// \param[in] y Y axis position
	/// \param[in] z Z axis position
	/// \param[in] a (if supported) A axis position
	/// \param[in] b (if supported) B axis position
	Point(int32_t x, int32_t y, int32_t z, int32_t a, int32_t b);


	// TODO: Can this be removed by giving the 5-dimensional function
	//       some initial values?
	/// Construct a point with the given cooridnates. Coordinates are in
	/// stepper steps. If used on a 5-dimesional system, the A and B
	/// axes are set to zero.
	/// \param[in] x X axis position
	/// \param[in] y Y axis position
	/// \param[in] z Z axis position
	Point(int32_t x, int32_t y, int32_t z);


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
	// Point & operator= (const Point &other);

	// friend const Point &operator-(const Point &a, const Point &b);


	/// Absolute value -- convert all point to positive
	Point abs();
} __attribute__ ((__packed__));


#endif // POINT_HH
