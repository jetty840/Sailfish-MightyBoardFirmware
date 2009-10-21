#ifndef _DATATYPES_H_
#define _DATATYPES_H_
/**
 *  Sanguino 3rd Generation Firmware (S3G)
 *
 *  Specification for this protocol is located at: 
 *    http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  License: GPLv2
 *  Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 */

#include <stdint.h>

/**
 * Our point representation.  Each coordinate is a 4-byte value.
 */
struct LongPoint {
public:
  int32_t x;
  int32_t y;
  int32_t z;
  LongPoint() : x(0), y(0), z(0) {}
  LongPoint(int32_t ix, int32_t iy, int32_t iz) : x(ix), y(iy), z(iz) {}
};

#define COMMAND_MODE_IDLE 0
#define COMMAND_MODE_WAIT_FOR_TOOL 1

#endif // _DATATYPES_H_
