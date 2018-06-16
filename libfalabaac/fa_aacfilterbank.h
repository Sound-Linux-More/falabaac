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


  filename: fa_aacfilterbank.h
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_AACFILTERBANK_H
#define _FA_AACFILTERBANK_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  

#include "fa_aacenc.h"
#include "fa_inttypes.h"

#define SINE_WINDOW         0
#define KBD_WINDOW          1

typedef struct _fa_aacfilterbank_t {

    float      x_buf[2*AAC_FRAME_LEN];

    float      sin_win_long_left[AAC_BLOCK_LONG_LEN];
    float      sin_win_long_right[AAC_BLOCK_LONG_LEN];
    float      kbd_win_long_left[AAC_BLOCK_LONG_LEN];
    float      kbd_win_long_right[AAC_BLOCK_LONG_LEN];

    float      sin_win_short_left[AAC_BLOCK_SHORT_LEN];
    float      sin_win_short_right[AAC_BLOCK_SHORT_LEN];
    float      kbd_win_short_left[AAC_BLOCK_SHORT_LEN];
    float      kbd_win_short_right[AAC_BLOCK_SHORT_LEN];

    uintptr_t  h_mdct_long;
    float      mdct_long_buf[2*AAC_BLOCK_LONG_LEN];
    uintptr_t  h_mdct_short;
    float      mdct_short_buf[2*AAC_BLOCK_SHORT_LEN];

}fa_aacfilterbank_t;


uintptr_t fa_aacfilterbank_init();
void fa_aacfilterbank_uninit(uintptr_t handle);


void fa_aacfilterbank_analysis(uintptr_t handle, int block_type, int *window_shape, 
                               float *x, float *mdct_line);
void fa_aacfilterbank_synthesis(uintptr_t handle, int block_type,  
                                float *mdct_line, float *x);

void fa_aacfilterbank_get_xbuf(uintptr_t handle, float *x);

#ifdef __cplusplus 
}
#endif  


#endif
