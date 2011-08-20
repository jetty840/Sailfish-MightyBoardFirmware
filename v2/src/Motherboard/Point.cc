#include "Point.hh"

Point::Point()
{
}

Point::Point(int32_t x, int32_t y, int32_t z, int32_t a, int32_t b) {
        coordinates[0] = x;
        coordinates[1] = y;
        coordinates[2] = z;
#if AXIS_COUNT > 3
        coordinates[3] = a;
        coordinates[4] = b;
#endif
}

Point::Point(int32_t x, int32_t y, int32_t z) {
        coordinates[0] = x;
        coordinates[1] = y;
        coordinates[2] = z;
#if AXIS_COUNT > 3
        coordinates[3] = 0;
        coordinates[4] = 0;
#endif
}

const int32_t& Point::operator[](unsigned int index) const {
    return coordinates[index];
}

int32_t& Point::operator[](unsigned int index) {
    return coordinates[index];
}
