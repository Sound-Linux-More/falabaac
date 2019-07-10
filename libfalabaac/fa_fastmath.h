/*
  falab - free algorithm lab
  Copyright (C) 2012 luolongzhi 罗龙智 (Chengdu, China)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  filename: fa_fastmath.h
  version : v1.0.0
  time    : 2012/10/20 16:47
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_FASTMATH_H
#define _FA_FASTMATH_H
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
WARN: if you want to plant this coder into embed system, you can enable FA_USE_FASTMATH and do fix point.
      OTHERWISE, use the default math library (it will be good quality and fast)
*/
//#define FA_USE_FASTMATH

#ifdef  FA_USE_FASTMATH
#define FA_USE_FASTLOG
#define FA_USE_FASTANGLE
#define FA_USE_FASTSQRTF
#endif

//No1.   log() log10() fast function
typedef float ieee754_float32_t;

void fa_logtab_init();
ieee754_float32_t fa_fast_log2(ieee754_float32_t x);

#ifndef     M_PI
#define     M_PI        3.14159265358979323846
#endif
#define M_PI_MUL2       6.28318530717958647692
#define M_PI_DIV2       1.57079632679489661923

#define M_PI_SQRT       1.7724538509
#define M_PI_SQRT_HALF  0.886226925451
#define SQRT2           1.41421356237

#ifndef FA_ABS
#define FA_ABS(A)    ((A) < 0 ? (-(A)) : (A))
#endif

#ifndef FA_MIN
#define FA_MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef FA_MAX
#define FA_MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef FA_TRIM
#define FA_TRIM(x,a,b) (FA_MIN(FA_MAX(x,a),b))
#endif

#ifdef FA_USE_FASTLOG
#define        FA_LOG2_V         (0.69314718055994530942)
#define        FA_LOG10_V        (2.30258509299404568402)
#define        FA_LOG2(x)        (fa_fast_log2(x))
#define        FA_LOG10(x)       (fa_fast_log2(x)*(FA_LOG2_V/FA_LOG10_V))
#define        FA_LOG(x)         (fa_fast_log2(x)*FA_LOG2_V)
#else
#ifdef WIN32
#define        FA_LOG10_2        (0.301029995664)
#define        FA_LOG2(x)        (log10(x)/FA_LOG10_2)
#else
#define        FA_LOG2(x)        (log2(x))
#endif
#define        FA_LOG10(x)       (log10(x))
#define        FA_LOG(x)         (log(x))
#endif

//No2.    angle fast function
float fa_fast_sin(float angle);
float fa_fast_cos(float angle);
float fa_fast_atan2(float y, float x);

#ifdef FA_USE_FASTANGLE
#define        FA_SIN(x)         (fa_fast_sin(x))
#define        FA_COS(x)         (fa_fast_cos(x))
#define        FA_ATAN2(y,x)     (fa_fast_atan2(y,x))
#else
#define        FA_SIN(x)         (sin(x))
#define        FA_COS(x)         (cos(x))
#define        FA_ATAN2(y,x)     (atan2(y,x))
#endif

float fa_fast_invsqrtf(float x);
float fa_fast_sqrtf(float x);
float fa_cbrtf(float x);
float fa_sqrf(float x);
float fa_quadf(float x);

#ifdef FA_USE_FASTSQRTF
#define        FA_SQRTF(x)       (fa_fast_sqrtf(x))
#define        FA_INVSQRTF(x)    (fa_fast_invsqrtf(x))
#define        FA_CBRTF(x)       (fa_cbrtf(x))
#else
#define        FA_SQRTF(x)       (sqrt(x))
#define        FA_INVSQRTF(x)    (1./sqrt(x))
#define        FA_CBRTF(x)       (cbrtf(x))
#endif

#ifdef __cplusplus
}
#endif

#endif //_FA_FASTMATH_H//
