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


  filename: fa_fastmath.c
  version : 2.1.0.229
  time    : 2019/07/14
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: https://github.com/Sound-Linux-More/falabaac
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fa_fastmath.h"

static ieee754_float32_t log_table[FA_LOG2_SIZE+1];

void fa_logtab_init()
{
    int j;


    assert((1<<FA_LOG2_SIZE_L2)==FA_LOG2_SIZE);

    for (j = 0; j < FA_LOG2_SIZE+1; j++)
        log_table[j] = log(1.0f+j/(ieee754_float32_t)FA_LOG2_SIZE)/log(2.0f);
}

ieee754_float32_t fa_fast_log2(ieee754_float32_t x)
{
    ieee754_float32_t log2val;
    union
    {
        ieee754_float32_t f;
        int     i;
    } fi;
    int mantisse;
    fi.f = x;
    mantisse =   fi.i & 0x7fffff;
    log2val  =   ((fi.i>>23) & 0xFF)-0x7f;
    mantisse >>= (23-FA_LOG2_SIZE_L2);
    log2val  +=  log_table[mantisse];

    return log2val;
}

//angle function
float fa_fast_sin(float angle)
{
    float real_angle;
    float square,result;
    int sign;

    /*normalize the angle in [0,2*M_PI]*/
    angle = angle + M_PI_MUL2;
    angle = fmod(angle, M_PI_MUL2); //now the angle is in(0,2*M_PI)

    if (angle <= M_PI)
    {
        real_angle = M_PI - angle;
        angle      = FA_MIN(real_angle, angle);
        sign = 1;
    }
    else
    {
        angle      = angle - M_PI;
        real_angle = M_PI - angle;
        angle      = FA_MIN(real_angle, angle);
        sign       = -1;
    }

    /*do fast compute*/
    square =  angle*angle;
    result =  7.61e-03f;
    result *= square;
    result -= 1.6605e-01f;
    result *= square;
    result += 1.0f;
    result *= angle;

    return result*sign;
}

float fa_fast_cos(float angle)
{
    float real_angle;
    float square,result;
    int sign;

    /*normalize the angle in [0,2*M_PI]*/
    angle = angle + M_PI_MUL2;
    angle = fmod(angle, M_PI_MUL2); //now the angle is in(0,2*M_PI)

    if (angle <= M_PI)
    {
        real_angle = M_PI - angle;
        /*sign       = (M_PI_DIV2 - angle)/FA_ABS(M_PI_DIV2 - angle);*/
        sign       = ((M_PI_DIV2 - angle) >= 0 ? 1 : -1);
        angle      = FA_MIN(real_angle,angle);
    }
    else
    {
        angle      = angle - M_PI;
        real_angle = M_PI - angle;
        /*sign     = (angle - M_PI_DIV2)/FA_ABS(angle - M_PI_DIV2);*/
        sign       = ((angle - M_PI_DIV2) >= 0 ? 1 : -1);
        angle      = FA_MIN(real_angle,angle);
    }

    /*do fast compute*/
    square = angle*angle;
    result = 3.705e-02f;
    result *= square;
    result -= 4.967e-01f;
    result *= square;
    result += 1.0f;

    return result*sign;
}

/*value should be (-1,1)*/
static float fast_atan (float value)
{
    float square = value*value;
    float result = 0.0208351f;

    result *= square;
    result -= 0.085133f;
    result *= square;
    result += 0.180141f;
    result *= square;
    result -= 0.3302995f;
    result *= square;
    result += 0.999866f;
    result *= value;

    return result;
}

float fa_fast_atan (float value)
{
    return fast_atan (value);
}

float fa_fast_atan2(float y, float x)
{
    float sita;
    float abs_y, abs_x;

    abs_y = FA_ABS(y);
    abs_x = FA_ABS(x);

    if (abs_y >= abs_x)
    {
        if (y > 0)
            sita = M_PI_DIV2 - fast_atan(x/y);
        else
            sita = M_PI_DIV2 - fast_atan(x/y) - M_PI;

    }
    else
    {
        if (x > 0)
            sita = fast_atan(y/x);
        else
            sita = fast_atan(y/x) - M_PI;
    }

    return sita;
}

float fa_fast_invsqrtf(float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);

    return x;
}

float fa_fast_sqrtf(float number)
{
    long i;
    float x, y;
    const float f = 1.5F;
    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}

float fa_cbrtf (float x)
{
    if (x == 0)
    {
        return 0;
    }
    float y = 1, last_y_1 = 0, last_y_2 = 0;
    while (last_y_1 != y && last_y_2 != y)
    {
        last_y_1 = y;
        y = (y + x / y / y) / 2;
        last_y_2 = y;
        y = (y + x / y / y) / 2;
    }
    return y;
}

float fa_sqrf(float x)
{
    x *= x;

    return x;
}

float fa_quadf(float x)
{
    x *= x;
    x *= x;

    return x;
}
