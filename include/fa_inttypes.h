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


  filename: fa_inttypes.h 
  version : v1.0.0
  time    : 2012/07/15 14:14 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/



#ifndef _FA_INTTYPES_H
#define _FA_INTTYPES_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  


typedef char *  caddr_t;

#ifdef WIN32
typedef char                int8_t;
typedef unsigned __int64    u_int64_t;
#else
typedef long long           __int64; 
typedef unsigned long long  u_int64_t;
#endif
typedef unsigned char       u_int8_t;
typedef short               int16_t;
typedef unsigned short      u_int16_t;
typedef int                 int32_t;
typedef unsigned int        u_int32_t;
typedef __int64             int64_t;

typedef int32_t             register_t;
typedef u_int8_t            uint8_t;
typedef u_int16_t           uint16_t;
typedef u_int32_t           uint32_t;
typedef u_int64_t           uint64_t;


#ifdef __GNUC__
#ifndef INT64_MAX
#define INT64_MAX 9223372036854775807LL
#endif
#else
#define INT64_MAX 9223372036854775807i64
#endif
#ifdef __GNUC__
#ifndef INT64_MIN
#define INT64_MIN  (-9223372036854775807LL - 1)
#endif
#else
#define INT64_MIN  (-9223372036854775807i64 - 1)
#endif

#ifndef INT8_MAX
#define INT8_MAX 127
#endif
#ifndef INT8_MIN
#define INT8_MIN (-127 - 1)
#endif

#ifndef INT16_MAX
#define INT16_MAX 32767
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32767 - 1)
#endif


#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif
#ifndef INT32_MIN
#define INT32_MIN (-2147483647 - 1)
#endif


#ifndef UINT8_MAX
#define UINT8_MAX 0xff /* 255U */
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 0xffff /* 65535U */
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#endif
#ifdef __GNUC__
#ifndef UINT64_MAX
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */
#endif
#else
#define UINT64_MAX 0xffffffffffffffffui64 /* 18446744073709551615ULL */
#endif

typedef int intptr_t;
typedef unsigned uintptr_t;

#ifndef INT64_C
#   if defined(__GNUC__)
#       define INT64_C(c)     (c ## LL)
#       define UINT64_C(c)    (c ## ULL)
#   else
#       define INT64_C(c)     (c ## i64)
#       define UINT64_C(c)    (c ## ui64)
#   endif
#endif


#ifndef offsetof
#    define offsetof(T, F) ((unsigned int)((char *)&((T *)0)->F))
#endif


#define INTMAX_MIN	    (-INT64_C(9223372036854775807)-1)
#define INTMAX_MAX		(INT64_C(9223372036854775807))
#define UINTMAX_MAX     (UINT64_C(18446744073709551615))

#ifndef WIN32
#define BYTE unsigned char
#endif

//other function compartiable define
#ifdef  WIN32
#define inline              __inline
#define snprintf            _snprintf
#define vsnprintf           _vsnprintf
#endif


#ifdef __cplusplus 
}
#endif  





#endif
