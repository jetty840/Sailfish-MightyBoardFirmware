/****************************************************************
 *                                                              *
 *          __ ____  _________                                  *
 *         /_ \\\  \/   /|    \______                           *
 *        //   \\\     /|| D  /_    /.                          *
 *       //     \\\_  /.||    \ ___/.                           *
 *      /___/\___\\__/. |__|\__\__.___  ___                     *
 *       ....  .......   ...||  _/_\  \////.                    *
 *                          || |.| |\  ///.                     *
 *                          |__|.|_|///  \                      *
 *                           .... ./__/\__\                     *
 *                                  ........                    *
 * Fixed Point Library                                          *
 * according to                                                 *
 * ISO/IEC DTR 18037                                            *
 *                                                              *
 * Version 1.0.1                                                *
 * Maximilan Rosenblattl, Andreas Wolf 2007-02-07               *
 ****************************************************************/
#ifndef TEST_ON_PC
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif
#include "avrfix.h"
#include "avrfix_config.h"

#if BYTE_ORDER == BIG_ENDIAN
typedef struct {
   uint16_t ll;
   uint8_t lh;
   int8_t h;
} lAccum_container;
#else
typedef struct {
   int8_t h;
   uint8_t lh;
   uint16_t ll;
} lAccum_container;
#endif

#define us(x) ((uint16_t)(x))
#define ss(x) ((int16_t)(x))
#define ul(x) ((uint32_t)(x))
#define sl(x) ((int32_t)(x))

extern void cordicck(_Accum* x, _Accum* y, _Accum* z, uint8_t iterations, uint8_t mode);
extern void cordichk(_Accum* x, _Accum* y, _Accum* z, uint8_t iterations, uint8_t mode);
extern void cordiccsk(_sAccum* x, _sAccum* y, _sAccum* z, uint8_t mode);
extern void cordichsk(_sAccum* x, _sAccum* y, _sAccum* z, uint8_t mode);

#ifdef SMULSKD
_sAccum smulskD(_sAccum x, _sAccum y)
{
  return ss(RSHIFT_static(sl(x)*sl(y), SACCUM_FBIT));
}
#endif
#ifdef SMULSKS
_sAccum smulskS(_sAccum x, _sAccum y)
{
  int32_t mul = RSHIFT_static(sl(x)*sl(y), SACCUM_FBIT);
  if(mul >= 0) {
    if((mul & 0xFFFF8000) != 0)
      return SACCUM_MAX;
  } else {
    if((mul & 0xFFFF8000) != 0xFFFF8000)
      return SACCUM_MIN;
  }
  return sl(mul);
}
#endif
#ifdef MULKD
_Accum mulkD(_Accum x, _Accum y)
{
#if BYTE_ORDER == BIG_ENDIAN
#  define LO 0
#  define HI 1
#else
#  define LO 1
#  define HI 0
#endif
  uint16_t xs[2];
  uint16_t ys[2];
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  y = absk(y);

#ifndef TEST_ON_PC
//clang doesn't allow pragma's within function def's
//Ignore gcc warnings on the following
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

  *((_Accum*)xs) = absk(x);
  *((_Accum*)ys) = y;
  x = sl(xs[HI])*y + sl(xs[LO])*sl(ys[HI]);
  *((_Accum*)xs) = ul(xs[LO])*ul(ys[LO]);

#ifndef TEST_ON_PC
#pragma GCC diagnostic pop
#endif


  if(positive)
     return x + us(xs[HI]);
  else
     return -(x + us(xs[HI]));
#undef HI
#undef LO
}
#endif
#ifdef MULKS
_Accum mulkS(_Accum x, _Accum y)
{
#if BYTE_ORDER == BIG_ENDIAN
#  define LO 0
#  define HI 1
#else
#  define LO 1
#  define HI 0
#endif
  uint16_t xs[2];
  uint16_t ys[2];
  uint32_t mul;
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  *((_Accum*)xs) = absk(x);
  *((_Accum*)ys) = absk(y);
  mul = ul(xs[HI]) * ul(ys[HI]);
  if(mul > 32767)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  mul =   LSHIFT_static(mul, ACCUM_FBIT)
        + ul(xs[HI])*ul(ys[LO])
        + ul(xs[LO])*ul(ys[HI])
        + RSHIFT_static(ul(xs[LO]*ys[LO]), ACCUM_FBIT);
  if(mul & 0x80000000)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  return (positive ? (int32_t)mul : -(int32_t)mul);
#undef HI
#undef LO
}
#endif
#ifdef LMULLKD
_lAccum lmullkD(_lAccum x, _lAccum y)
{
  lAccum_container *xc, *yc;
  xc = (lAccum_container*)&x;
  yc = (lAccum_container*)&y;
  return   sl(xc->h)*y + sl(yc->h)*(x&0x00FFFFFF)
         + ((ul(xc->lh)*ul(yc->lh))*256)
         + RSHIFT_static((ul(xc->lh)*ul(yc->ll) + ul(xc->ll)*ul(yc->lh)), 8)
         + (RSHIFT_static((ul(xc->lh)*ul(yc->ll) + ul(xc->ll)*ul(yc->lh)), 7)&1)
         + RSHIFT_static((ul(xc->ll)*ul(yc->ll)), 24);
}
#endif
#ifdef LMULLKS
_lAccum lmullkS(_lAccum x, _lAccum y)
{
  lAccum_container xc, yc;
  uint32_t mul;
  int8_t positive = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  x = labslk(x);
  y = labslk(y);
  *((_lAccum*)&xc) = x;
  *((_lAccum*)&yc) = y;
  mul = xc.h * yc.h;
  x &= 0x00FFFFFF;
  y &= 0x00FFFFFF;
  if(mul > 127)
     return (positive ? LACCUM_MAX : LACCUM_MIN);
  mul =   LSHIFT_static(mul, LACCUM_FBIT) + ul(xc.h)*y + ul(yc.h)*x +
        + (ul(xc.lh)*ul(yc.lh)*256)
        + RSHIFT_static((ul(xc.lh)*ul(yc.ll) + ul(xc.ll)*ul(yc.lh)), 8)
        + (RSHIFT_static((ul(xc.lh)*ul(yc.ll) + ul(xc.ll)*ul(yc.lh)), 7)&1)
        + RSHIFT_static((ul(xc.ll)*ul(yc.ll)), 24);
  if(mul & 0x80000000)
     return (positive ? ACCUM_MAX : ACCUM_MIN);
  return (positive ? (int32_t)mul : -(int32_t)mul);
}
#endif
#ifdef SDIVSKD
_sAccum sdivskD(_sAccum x, _sAccum y)
{
  return ss((sl(x) << SACCUM_FBIT) / y);
}
#endif
#ifdef SDIVSKS
_sAccum sdivskS(_sAccum x, _sAccum y)
{
  int32_t div;
  if(y == 0)
     return (x < 0 ? SACCUM_MIN : SACCUM_MAX);
  div = (sl(x) << SACCUM_FBIT) / y;
  if(div >= 0) {
    if((div & 0xFFFF8000) != 0)
      return SACCUM_MAX;
  } else {
    if((div & 0xFFFF8000) != 0xFFFF8000)
      return SACCUM_MIN;
  }
  return ss(div);
}
#endif
#ifdef DIVKD
/* if y = 0, divkD will enter an endless loop */
_Accum divkD(_Accum x, _Accum y) {
  if ( y == 0 ) return ACCUM_INFINITY;
  _Accum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (x > 0 && y > 0)) ? 1 : 0;
  x = absk(x);
  y = absk(y);
  /* Align x leftmost to get maximum precision */

  for (i=0 ; i<ACCUM_FBIT ; i++)
  {
    if (x >= ACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }
  while((y & 1) == 0) {
    y = RSHIFT_static(y, 1);
    j++;
  }
  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (ACCUM_FBIT - i) - j;
  if(i > 0)
     result = LSHIFT_dynamic(result, i);
  else if(i < 0) {
     /* shift right except for 1 bit, wich will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* determine if round is necessary */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif
#ifdef DIVKS
_Accum divkS(_Accum x, _Accum y) {
  _Accum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  if(y == 0)
     return (x < 0 ? ACCUM_MIN : ACCUM_MAX);
  x = absk(x);
  y = absk(y);

  for (i=0 ; i<ACCUM_FBIT ; i++)
  {
    if (x >= ACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }

  while((y & 1) == 0) {
    y = RSHIFT_static(y, 1);
    j++;
  }

  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (ACCUM_FBIT - i) - j;
  if(i > 0)
     for(;i>0;i--) {
       if((result & 0x40000000) != 0) {
         return sign ? ACCUM_MAX : ACCUM_MIN;
       }
       result = LSHIFT_static(result, 1);
     }
  else if(i < 0) {
     /* shift right except for 1 bit, wich will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* round */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif
#ifdef LDIVLKD
/* if y = 0, ldivlkD will enter an endless loop */
_lAccum ldivlkD(_lAccum x, _lAccum y) {
  _lAccum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (x > 0 && y > 0)) ? 1 : 0;
  x = labslk(x);
  y = labslk(y);
  /* Align x leftmost to get maximum precision */

  for (i=0 ; i<LACCUM_FBIT ; i++)
  {
    if (x >= LACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }
  while((y & 1) == 0) {
    y = RSHIFT_static(y, 1);
    j++;
  }
  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (LACCUM_FBIT - i) - j;
  if(i > 0)
     result = LSHIFT_dynamic(result, i);
  else if(i < 0) {
     /* shift right except for 1 bit, wich will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* determine if round is necessary */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif
#ifdef LDIVLKS
_lAccum ldivlkS(_lAccum x, _lAccum y) {
  _lAccum result;
  int i,j=0;
  int8_t sign = ((x < 0 && y < 0) || (y > 0 && x > 0)) ? 1 : 0;
  if(y == 0)
     return (x < 0 ? LACCUM_MIN : LACCUM_MAX);
  x = labslk(x);
  y = labslk(y);

  for (i=0 ; i<LACCUM_FBIT ; i++)
  {
    if (x >= LACCUM_MAX / 2) break;
    x = LSHIFT_static(x, 1);
  }

  while((y & 1) == 0) {
    y = RSHIFT_static(y, 1);
    j++;
  }

  result = x/y;

  /* Correct value by shift left */
  /* Check amount and direction of shifts */
  i = (LACCUM_FBIT - i) - j;
  if(i > 0)
     for(;i>0;i--) {
       if((result & 0x40000000) != 0) {
         return sign ? LACCUM_MAX : LACCUM_MIN;
       }
       result = LSHIFT_static(result, 1);
     }
  else if(i < 0) {
     /* shift right except for 1 bit, wich will be used for rounding */
     result = RSHIFT_dynamic(result, (-i) - 1);
     /* round */
     result = RSHIFT_static(result, 1) + (result & 1);
  }
  return (sign ? result : -result);
}
#endif
#ifdef SINCOSK
_Accum sincosk(_Accum angle, _Accum* cosp)
{
  _Accum x;
  _Accum y = 0;
  uint8_t correctionCount = 0;
  uint8_t quadrant = 1;
  if(cosp == NULL)
     cosp = &x;

  /* move large values into [0,2 PI] */
#define MAX_CORRECTION_COUNT 1
  while(angle >= PIlk) { /* PIlk = PIk * 2^8 */
    angle -= PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle++;
    } else {
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) {
    angle += PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle--;
    } else {
      correctionCount++;
    }
  }
#undef MAX_CORRECTION_COUNT

  /* move small values into [0,2 PI] */
#define MAX_CORRECTION_COUNT 5
  while(angle >= 2*PIk + 1) {
    angle -= 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle++;
    } else {
      correctionCount++;
    }
  }
  if(correctionCount > 0) {
    angle++;
  }
  correctionCount = 0;
  while(angle < 0) {
    angle += 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle--;
    } else {
      correctionCount++;
    }
  }
  if(correctionCount > 0) {
    angle--;
  }
#undef MAX_CORRECTION_COUNT

  if(angle > PIk) {
    angle = angle - PIk;
    quadrant += 2;
  }
  if(angle > (PIk/2 + 1)) {
    angle = PIk - angle + 1;
    quadrant += 1;
  }
  if(angle == 0) {
    *cosp = (quadrant == 2 || quadrant == 3 ? -itok(1) : itok(1));
    return 0;
  }
  *cosp = CORDICC_GAIN;
  angle = LSHIFT_static(angle, 8);
  cordicck(cosp, &y, &angle, 17, 0);
  (*cosp) = RSHIFT_static(*cosp, 8);
  y       = RSHIFT_static(y, 8);
  switch(quadrant) {
    case 2: {
      (*cosp) = -(*cosp);
    } break;
    case 3: {
      y = -y;
      (*cosp) = -(*cosp);
    } break;
    case 4: {
      y = -y;
    } break;
    default:;
  }
  return y;
}
#endif
#ifdef LSINCOSLK
_lAccum lsincoslk(_lAccum angle, _lAccum* cosp)
{
  _lAccum x;
  _lAccum y = 0;
  uint8_t correctionCount;
  uint8_t quadrant = 1;
  if(cosp == NULL)
     cosp = &x;

  /* move values into [0, 2 PI] */
#define MAX_CORRECTION_COUNT 1
  correctionCount = 0;
  while(angle >= 2*PIlk) {
    angle -= 2*PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle++;
    } else {
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) {
    angle += 2*PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle--;
    } else {
      correctionCount++;
    }
  }
#undef MAX_CORRECTION_COUNT

  if(angle > PIlk) {
    angle = angle - PIlk;
    quadrant += 2;
  }
  if(angle > (PIlk/2)) {
    angle = PIlk - angle;
    quadrant += 1;
  }
  if(angle == 0) {
    *cosp = (quadrant == 2 || quadrant == 3 ? -itolk(1) : itolk(1));
    return 0;
  }
  *cosp = CORDICC_GAIN;
  cordicck(cosp, &y, &angle, 24, 0);
  switch(quadrant) {
    case 2: {
      (*cosp) = -(*cosp);
    } break;
    case 3: {
      y = -y;
      (*cosp) = -(*cosp);
    } break;
    case 4: {
      y = -y;
    } break;
    default:;
  }
  return y;
}
#endif
#ifdef LSINCOSK
_lAccum lsincosk(_Accum angle, _lAccum* cosp)
{
  uint8_t correctionCount = 0;
  /* move large values into [0,2 PI] */
#define MAX_CORRECTION_COUNT 1
  while(angle >= PIlk) { /* PIlk = PIk * 2^8 */
    angle -= PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle++;
    } else {
      correctionCount++;
    }
  }
  correctionCount = 0;
  while(angle < 0) {
    angle += PIlk;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle--;
    } else {
      correctionCount++;
    }
  }
#undef MAX_CORRECTION_COUNT

  /* move small values into [0,2 PI] */
#define MAX_CORRECTION_COUNT 5
  while(angle >= 2*PIk + 1) {
    angle -= 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle++;
    } else {
      correctionCount++;
    }
  }
  if(correctionCount > 0) {
    angle++;
  }
  correctionCount = 0;
  while(angle < 0) {
    angle += 2*PIk + 1;
    if(correctionCount == MAX_CORRECTION_COUNT) {
      correctionCount = 0;
      angle--;
    } else {
      correctionCount++;
    }
  }
  if(correctionCount > 0) {
    angle--;
  }
#undef MAX_CORRECTION_COUNT
  return lsincoslk(LSHIFT_static(angle, (LACCUM_FBIT - ACCUM_FBIT)), cosp);
}
#endif
#ifdef ROUNDSKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_sAccum roundskD(_sAccum f, uint8_t n)
{
   n = SACCUM_FBIT - n;
   if(f >= 0) {
      return (f & (0xFFFF << n)) + ((f & (1 << (n-1))) << 1);
   } else {
      return (f & (0xFFFF << n)) - ((f & (1 << (n-1))) << 1);
   }
}
#endif
#ifdef ROUNDKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Accum roundkD(_Accum f, uint8_t n)
{
   n = ACCUM_FBIT - n;
   if(f >= 0) {
      return (f & (0xFFFFFFFF << n)) + ((f & (1 << (n-1))) << 1);
   } else {
      return (f & (0xFFFFFFFF << n)) - ((f & (1 << (n-1))) << 1);
   }
}
#endif
#ifdef ROUNDSKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_sAccum roundskS(_sAccum f, uint8_t n)
{
   if(n > SACCUM_FBIT) {
      return 0;
   }
   return roundskD(f, n);
}
#endif
#ifdef ROUNDKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Accum roundkS(_Accum f, uint8_t n)
{
   if(n > ACCUM_FBIT) {
      return 0;
   }
   return roundkD(f, n);
}
#endif
#ifdef ROUNDLKD
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_lAccum roundlkD(_lAccum f, uint8_t n)
{
   n = LACCUM_FBIT - n;
   if(f >= 0) {
      return (f & (0xFFFFFFFF << n)) + ((f & (1 << (n-1))) << 1);
   } else {
      return (f & (0xFFFFFFFF << n)) - ((f & (1 << (n-1))) << 1);
   }
}
#endif
#ifdef ROUNDLKS
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
_Accum roundlkS(_lAccum f, uint8_t n)
{
   if(n > LACCUM_FBIT) {
      return 0;
   }
   return roundlkD(f, n);
}
#endif
#ifdef COUNTLSSK
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
uint8_t countlssk(_sAccum f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 2;
   for(i = 0; i < 15; i++) {
      if((*pf & 0x40) != 0)
         break;
      f = LSHIFT_static(f, 1);
   }
   return i;
}
#endif
#ifdef COUNTLSK
/*
 * Difference from ISO/IEC DTR 18037:
 * using an uint8_t as second parameter according to
 * microcontroller register size and maximum possible value
 */
uint8_t countlsk(_Accum f)
{
   int8_t i;
   uint8_t *pf = ((uint8_t*)&f) + 3;
   for(i = 0; i < 31; i++) {
      if((*pf & 0x40) != 0)
         break;
      f = LSHIFT_static(f, 1);
   }
   return i;
}
#endif
#ifdef TANKD
_Accum tankD(_Accum angle)
{
  _Accum sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkD(sin, cos);
}
#endif
#ifdef TANKS
_Accum tankS(_Accum angle)
{
  _Accum sin, cos;
  sin = sincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? ACCUM_MIN : ACCUM_MAX);
  return divkS(sin, cos);
}
#endif
#ifdef LTANLKD
_lAccum ltanlkD(_lAccum angle)
{
  _lAccum sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkD(sin, cos);
}
#endif
#ifdef LTANLKS
_lAccum ltanlkS(_lAccum angle)
{
  _lAccum sin, cos;
  sin = lsincoslk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif
#ifdef LTANKD
_lAccum ltankD(_Accum angle)
{
  _lAccum sin, cos;
  sin = lsincosk(angle, &cos);
  return ldivlkD(sin, cos);
}
#endif
#ifdef LTANKS
_lAccum ltankS(_Accum angle)
{
  _lAccum sin, cos;
  sin = lsincosk(angle, &cos);
  if(absk(cos) <= 2)
     return (sin < 0 ? LACCUM_MIN : LACCUM_MAX);
  return ldivlkS(sin, cos);
}
#endif
#ifdef ATAN2K
_Accum atan2kInternal(_Accum x, _Accum y)
{
  _Accum z = 0;
  uint8_t i = 0;
  uint8_t *px = ((uint8_t*)&x) + 3, *py = ((uint8_t*)&y) + 3;
  for(;!(*px & 0x60) && !(*py & 0x60) && i < 8;i++) {
    x = LSHIFT_static(x, 1);
    y = LSHIFT_static(y, 1);
  }
  if(i > 0) {
    cordicck(&x, &y, &z, 16, 1);
    return RSHIFT_static(z, 8);
  } else {
    return PIk/2 - divkD(x, y) - 1;
  }
}

_Accum atan2k(_Accum x, _Accum y)
{
  uint8_t signX, signY;
  if(y == 0)
     return 0;
  signY = (y < 0 ? 0 : 1);
  if(x == 0)
     return (signY ? ACCUM_MAX : ACCUM_MIN);
  signX = (x < 0 ? 0 : 1);
  x = atan2kInternal(absk(x), absk(y));
  if(signY) {
    if(signX) {
      return x;
    } else {
      return x + PIk/2 + 1;
    }
  } else {
    if(signX) {
      return -x;
    } else {
      return -x - PIk/2 - 1;
    }
  }
}
#endif
#ifdef LATAN2LK
_lAccum latan2lk(_lAccum x, _lAccum y)
{
  uint8_t signX, signY;
  _Accum z = 0;
  uint8_t *px = ((uint8_t*)&x) + 3, *py = ((uint8_t*)&y) + 3;
  if(y == 0)
     return 0;
  signY = (y < 0 ? 0 : 1);
  if(x == 0)
     return (signY ? ACCUM_MAX : ACCUM_MIN);
  signX = (x < 0 ? 0 : 1);
  if(!signX)
     x = -x;
  if(!signY)
     y = -y;
  if((*px & 0x40) || (*py & 0x40)) {
    x = RSHIFT_static(x, 1);
    y = RSHIFT_static(y, 1);
  }
  cordicck(&x, &y, &z, 24, 1);
  if(signY) {
    if(signX) {
      return z;
    } else {
      return z+PIlk/2;
    }
  } else {
    if(signX) {
      return -z;
    } else {
      return -z-PIlk/2;
    }
  }
}
#endif
#ifdef CORDICCK
/*
 * calculates the circular CORDIC method in both modes
 * mode = 0:
 * Calculates sine and cosine with input z and output x and y. To be exact
 * x has to be CORDIC_GAIN instead of itok(1) and y has to be 0.
 *
 * mode = 1:
 * Calculates the arctangent of y/x with output z. No correction has to be
 * done here.
 *
 * iterations is the fractal bit count (16 for _Accum, 24 for _lAccum)
 * and now the only variable, the execution time depends on.
 */
void cordicck(_Accum* px, _Accum* py, _Accum* pz, uint8_t iterations, uint8_t mode)
{
  const uint32_t arctan[25] = {13176795, 7778716, 4110060, 2086331, 1047214, 524117, 262123, 131069, 65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};
  register uint8_t i;
  _Accum x, y, z, xH;
  x = *px;
  y = *py;
  z = *pz;
  for (i = 0; i <= iterations; i++) {
    xH = x;
    if((mode && y <= 0) || (!mode && z >= 0)) {
      x -= RSHIFT_dynamic(y, i);
      y += RSHIFT_dynamic(xH, i);
      z -= arctan[i];
    }
    else {
      x += RSHIFT_dynamic(y, i);
      y -= RSHIFT_dynamic(xH, i);
      z += arctan[i];
    }
  }
  *px = x;
  *py = y;
  *pz = z;
}
#endif
#ifdef CORDICHK
/*
 * calculates the hyperbolic CORDIC method in both modes
 * mode = 0:
 * Calculates hyperbolic sine and cosine with input z and output x and y.
 * To be exact x has to be CORDICH_GAIN instead of itok(1) and y has to be 0.
 * This mode is never used in this library because of target limitations.
 *
 * mode = 1:
 * Calculates the hyperbolic arctangent of y/x with output z. No correction
 * has to be done here.
 *
 * iterations is the fractal bit count (16 for _Accum, 24 for _lAccum)
 */
void cordichk(_Accum* px, _Accum* py, _Accum* pz, uint8_t iterations, uint8_t mode)
{
  const uint32_t arctanh[24] = {9215828, 4285116, 2108178, 1049945, 524459, 262165, 131075, 65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};
  register uint8_t i, j;
  _Accum x, y, z, xH;
  x = *px;
  y = *py;
  z = *pz;
  for (i = 1; i <= iterations; i++) {
    for(j = 0; j < 2; j++) {/*repeat iterations 4, 13, 40, ... 3k+1*/
      xH = x;
      if((mode && y <= 0) || (!mode && z >= 0)) {
        x += RSHIFT_dynamic(y, i);
        y += RSHIFT_dynamic(xH, i);
        z -= arctanh[i-1];
      }
      else {
        x -= RSHIFT_dynamic(y, i);
        y -= RSHIFT_dynamic(xH, i);
        z += arctanh[i-1];
      }
      if(i != 4 && i != 13)
        break;
    }
  }
  *px = x;
  *py = y;
  *pz = z;
}
#endif
#ifdef SQRT
_Accum sqrtk_uncorrected(_Accum a, int8_t pow2, uint8_t cordic_steps)
{
  _Accum x, y, z;
  if(a <= 0)
    return 0;
  /* The cordich method works only within [0.03, 2]
   * for other values the following identity is used:
   *
   * sqrt(2^n * a) = sqrt(a) * sqrt(2^n) = sqrt(a) * 2^(n/2)
   *
   * Here, the interval [0.06, 1] is taken, because the
   * number of shifts may be odd and the correction shift
   * may be outside the original interval in that case.
   */
  for(; a > 16777216; pow2++)
    a = RSHIFT_static(a, 1);
  for(; a < 1006592; pow2--)
    a = LSHIFT_static(a, 1);
  /* pow2 has to be even */
  if(pow2 > 0 && pow2 & 1) {
    pow2--;
    a = LSHIFT_static(a, 1);
  } else if(pow2 < 0 && pow2 & 1) {
    pow2++;
    a = RSHIFT_static(a, 1);
  }
  pow2 = RSHIFT_static(pow2, 1);
  x = a + 4194304;
  y = a - 4194304;
  z = 0;
  cordichk(&x, &y, &z, cordic_steps, 1);
  return (pow2 < 0 ? RSHIFT_dynamic(x, -pow2) : LSHIFT_dynamic(x, pow2));
}
#endif
#ifdef LOGK
_Accum logk(_Accum a)
{
  register int8_t pow2 = 8;
  _Accum x, y, z;
  if(a <= 0)
    return ACCUM_MIN;
  /* The cordic method works only within [1, 9]
   * for other values the following identity is used:
   *
   * log(2^n * a) = log(a) + log(2^n) = log(a) + n log(2)
   */
  for(; a > 150994944; pow2++)
    a = RSHIFT_static(a, 1);
  for(; a < 16777216; pow2--)
    a = LSHIFT_static(a, 1);
  x = a + 16777216;
  y = a - 16777216;
  z = 0;
  cordichk(&x, &y, &z, 17, 1);
  return RSHIFT_static(z, 7) + LOG2k*pow2;
}
#endif
#ifdef LLOGLK
_lAccum lloglk(_lAccum a)
{
  register int8_t pow2 = 0;
  _Accum x, y, z;
  if(a <= 0)
    return LACCUM_MIN;
  /* The cordic method works only within [1, 9]
   * for other values the following identity is used:
   *
   * log(2^n * a) = log(a) + log(2^n) = log(a) + n log(2)
   */
  for(; a > 150994944; pow2++)
    a = RSHIFT_static(a, 1);
  for(; a < 16777216; pow2--)
    a = LSHIFT_static(a, 1);
  x = a + 16777216;
  y = a - 16777216;
  z = 0;
  cordichk(&x, &y, &z, 24, 1);
  return LSHIFT_static(z, 1) + LOG2lk*pow2;
}
#endif
