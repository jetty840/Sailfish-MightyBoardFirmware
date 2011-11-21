#ifndef POINT_HH
#define POINT_HH

#include "Configuration.hh"
#include <stdint.h>


#define AXIS_COUNT STEPPER_COUNT

/// Class that represents an N-dimensional point, where N is the number of
/// stepper axes present in the system. Can support 3 or 5 axes.
class Point {
private:
        int32_t coordinates[AXIS_COUNT];        ///< n-dimensional coordinate
public:
        /// Default point constructor
        Point();

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

} __attribute__ ((__packed__));


#endif // POINT_HH
