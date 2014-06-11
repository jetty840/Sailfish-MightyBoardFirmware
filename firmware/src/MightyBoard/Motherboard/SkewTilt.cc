/*
 *  skew-tilt.c
 *  Perform skew or tilt transformations to move points in planes parallel
 *  to Z=0 to a plane parallel to the plane determined by probing the
 *  build plate at three points, P1, P2, P3.
 *
 *  The skew transformation requires only two multiples and two additions
 *  per point.  It does result in a very, very slight skew.  E.g., if the
 *  out-of-level is at most 0.5 mm across a 200 mm baseline, then at worst
 *  a print will be out of perpendicular by 0.25 mm per 100 mm of build
 *  height.  And that's worst case.  For a plate levelled to within 0.2 mm
 *  across the same 200 mm base line, the deviation would be only 0.1 mm
 *  per 100 mm of build height.
 *
 *  The tilt transformation requires eight additional multiples and five
 *  additions per point.  But it introduces no skew.
 *
 *  Skew transform
 *  --------------
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
 *    x * Nx + y * Ny + z * Nz + d = 0
 *
 * We can determine d by substituting in P1 and solving for
 * d,
 *
 *    d = - ( x1 * Nx + y1 * Ny + z1 * Nz )
 * 
 * And from that we have our equation to do the skewing,
 *
 *    z-skew = z - (d + x * Nx + y * Ny ) / Nz
 *
 * Tilt transform
 * --------------
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
 * Next, we wish to determine the angular tilt away from the Z axis as
 * measured in the planes Y=0 and X=0,
 *
 *    Ay = arctan(Nx / Nz)  -- tilt in the plane Y=0
 *    Ax = arctan(Ny / Nz)  -- tilt in the plane X=0
 *
 * To transform a point P from a plane parallel to Z=0 to the point P'
 * in the plane parallel to that defined by P1, P2, and P3, we would
 * just perform the requisite rotations around the Y and then X axes
 * (or around Y and then X),
 *
 *    P' = rotate-X(Ax, rotate-Y(-Ay, P))
 *
 * Inverting the process is then just
 *
 *    P = rotate-Y(Ay, rotate-X(-Ax, P'))
 */

#if defined(AUTO_LEVEL)

#if defined(AUTO_LEVEL_TILT)
#include <math.h>
#endif

#include "SkewTilt.hh"

static int32_t skew_data[4];
bool skew_active = false;

static void crossProduct(const int32_t V1[], const int32_t V2[], int32_t N[])
{
     N[0] = V1[1] * V2[2] - V1[2] * V2[1];
     N[1] = V1[2] * V2[0] - V1[0] * V2[2];
     N[2] = V1[0] * V2[1] - V1[1] * V2[0];
}

int32_t skew(const int32_t P[])
{
     // z - ( x * Nx + y * Ny ) / Nz2
     return ( - ( skew_data[3] + P[0] * skew_data[0] + P[1] * skew_data[1] ) / skew_data[2] );
}

bool skew_init(const int32_t P1[], const int32_t P2[], const int32_t P3[])
{
     int32_t V1[3], V2[3];

     V1[0] = P2[0] - P1[0];
     V1[1] = P2[1] - P1[1];
     V1[2] = P2[2] - P1[2];

     V2[0] = P3[0] - P1[0];
     V2[1] = P3[1] - P1[1];
     V2[2] = P3[2] - P1[2];

     crossProduct(V1, V2, skew_data);

     if ( skew_data[2] == 0 )
     {
	  skew_deinit();
	  return false;
     }

     // We want the upward pointing normal
     if ( skew_data[2] < 0 )
     {
	  skew_data[0] = -skew_data[0];
	  skew_data[1] = -skew_data[1];
	  skew_data[2] = -skew_data[2];
     }

     // Constant d in x*Nx + y*Ny + z*Nz + d = 0
     //  Determine by solving for d using point P1

     skew_data[3] = - ( P1[0] * skew_data[0] + P1[1] * skew_data[1] +
			P1[2] * skew_data[2] );

     skew_active = true;
     return true;
}

void skew_deinit(void)
{
     skew_data[0] = 0;
     skew_data[1] = 0;
     skew_data[2] = 1;
     skew_data[3] = 0;
     skew_active = false;
}

#if defined(AUTO_LEVEL_TILT)

#define cosAx       0
#define cosAy       1
#define sinAx       2
#define sinAy       3
#define cosAx_cosAy 4
#define cosAx_sinAy 5
#define sinAx_sinAy 6
#define sinAx_cosAy 7

static FPTYPE tilt_data[8];

bool tilt_init(Point &P1, Point &P2, Point &P3)
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

     FPTYPE Ax, Ay, Nz;

     FNz = ITOFP(N[2]);
     Ax = ATAN2(ITOFP(N[1]), FNz);
     Ay = ATAN2(ITOFP(N[0]), FNz);

     tilt_data[cosAx] = FCOS(Ax);
     tilt_data[cosAy] = FCOS(Ay);
     tilt_data[sinAx] = FSIN(Ax);
     tilt_data[sinAy] = FSIN(Ay);
     tilt_data[cosAx_cosAy] = FPMULT2(tilt_data[cosAx], tilt_data[cosAy]);
     tilt_data[cosAx_sinAy] = FPMULT2(tilt_data[cosAx], tilt_data[sinAy]);
     tilt_data[sinAx_sinAy] = FPMULT2(tilt_data[sinAx], tilt_data[sinAy]);
     tilt_data[sinAx_cosAy] = FPMULT2(tilt_data[sinAx], tilt_data[cosAy]);

     return true;
}

Point tilt(Point &P)
{
     FPTYPE Px, Py, Pz;

     Px = ITOFP(P[0]);
     Py = ITOFP(P[1]);
     Pz = ITOFP(P[2]);

     Point new(0);

     new[0] = FPTOI(FPMULT2(Px, tilt_data[cosAx]) +
		    FMULT2(Pz, tilt_data[sinAy]));
     new[1] = FPTOI(FPMULT2(Py, tilt_data[cosAx]) -
		    FPMULT2(Px, tilt_data[sinAx_sinAy]) +
		    FPMULT2(Px, tilt_data[sinAx_cosAy)));
     new[3] = FPTOI(FPMULT2(Pz, tilt_data[cosAx_cosAy]) 
		    FPMULT2(Px, tilt_data[cosAx_sinAy]) -
		    FPMULT2(Py, tilt_data[sinAx]));

     return new;
}

Point tilt_inverse(Point &P)
{
     // NOT YET IMPLEMENTED
     return P;
}

#endif

#endif
