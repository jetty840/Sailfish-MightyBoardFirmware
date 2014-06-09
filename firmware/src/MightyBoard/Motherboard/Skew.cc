/*
 *  skew.c -- skew transformation to move points in planes parallel to Z=0
 *            to a plane parallel to the plane determined by probing the
 *            build plate at three points, P1, P2, P3.
 *
 *  Probe the build platform's Z height at three points, P1, P2, and P3,
 *
 *    P1 = (x1, y1, z1)
 *    P2 = (x2, y2, z2)
 *    P3 = (x3, y3, z3)
 *
 * The three points P1, P2, and P3 define a plane.  Equivalently, the two
 * vectors V1 and V2 given by
 *
 *    V1 = P2 - P1
 *    V2 = P3 - P1
 *
 * define the same plane.  A vector N normal to the plane is then given by
 *
 *    N = (Nx, Ny, Nz) = V1 x V2
 *
 * where "x" is the vector cross product.  We want the upward pointing
 * normal so if the Nz component of N is negative, we can just invert N
 * by negating each of its components, Nx, Ny, and Nz.
 *
 * Now, the equation for the plane is then
 *
 *    x * Nx + y * Ny + z * Nz = C
 *
 * for some constant C.  We're interested in C = 0 which gives a parallel
 * plane containing the origin (0, 0, 0).  For that plane,
 *
 *    z = - (x * Nx + y * Ny ) / Nz
 *
 * and from that we have our equation to do the skewing,
 *
 *    z-skew = z - (x * Nx + y * Ny ) / Nz
 *
 */

#if defined(AUTO_LEVEL)

#include "Skew.hh"

static int32_t skew_data[2];

static void crossProduct(const int32_t V1[], const int32_t V2[], int32_t N[])
{
     N[0] = V1[1] * V2[2] - V1[2] * V2[1];
     N[1] = V1[2] * V2[0] - V1[0] * V2[2];
     N[2] = V1[0] * V2[1] - V1[1] * V2[0];
}

int32_t skew(Point &P)
{
     // P[2] - ( P[0] * normal[0] + P[1] * normal[1] ) / normal[2];
     return P[2] - P[0] * skew_data[0] - P[1] * skew_data[1];
}

int32_t skew_inverse(Point &P)
{
     // P[2] + (P[0] * normal[0] + P[1] * normal[1] ) / normal[2];
     return P[2] + P[0] * skew_data[0] + P[1] * skew_data[1];
}

bool skew_init(Point &P1, Point &P2, Point &P3)
{
     int32_t N[3], V1[3], V2[3];

     V1[0] = P2[0] - P1[0];
     V1[1] = P2[1] - P1[1];
     V1[2] = P2[2] - P1[2];

     V2[0] = P3[0] - P1[0];
     V2[1] = P3[1] - P1[1];
     V2[2] = P3[2] - P1[2];

     crossProduct(V1, V2, N);

     if ( N[2] == 0 )
	  return false;

     // We want the upward pointing normal
     if ( N[2] < 0 )
     {
	  N[0] = -N[0];
	  N[1] = -N[1];
	  N[2] = -N[2];
     }

     skew_data[0] = N[0] / N[2];
     skew_data[1] = N[1] / N[2];

     return true;
}

#endif
