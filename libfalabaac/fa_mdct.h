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


  filename: fa_mdct.h 
  version : v1.0.0
  time    : 2012/07/16 - 2012/07/18  
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_MDCT_H
#define _FA_MDCT_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  

#include "fa_inttypes.h"

typedef int mdct_win_t;

/*
    origin: the naive mdct using the original formular, this mdct is leading to you learning mdct
    fft   : normally called fast mdct, use fft transform to compute mdct
    fft4  : the wildly used, N/4 point FFT to compute mdct
*/
enum{
    MDCT_ORIGIN = 0,
    MDCT_FFT,
    MDCT_FFT4,
};

enum{
    MDCT_SINE = 0,
    MDCT_KBD,
};

uintptr_t fa_mdct_init(int type, int len);
void      fa_mdct_uninit(uintptr_t handle);

void fa_mdct(uintptr_t handle, float *x, float *X);
void fa_imdct(uintptr_t handle, float *X, float *x);

int fa_mdct_sine(float *w, int N);
int fa_mdct_kbd(float *w, int N, float alpha);

#ifdef __cplusplus 
}
#endif  


#endif
