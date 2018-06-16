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


  filename: fa_aacpsy.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_AACPSY_H
#define _FA_AACPSY_H 

#include "fa_inttypes.h"
#include "fa_swbtab.h"
#include "fa_psychomodel1.h"
#include "fa_psychomodel2.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


uintptr_t fa_aacpsy_init(int sample_rate);
void fa_aacpsy_uninit(uintptr_t handle);

void update_psy_long2short_previnfo(uintptr_t handle);
void update_psy_short2long_previnfo(uintptr_t handle);

void update_psy_short_previnfo(uintptr_t handle, int index);

//reset previnfo
void reset_psy_previnfo(uintptr_t handle);

//the x must be 16bits sample quantize
void fa_aacpsy_calculate_pe(uintptr_t handle, float *x, int block_type, float *pe_block, int *tns_active);
void fa_aacpsy_calculate_pe_hp(uintptr_t handle, float *x, int block_type, float *pe_block, int *tns_active);
void fa_aacpsy_calculate_xmin(uintptr_t handle, float *mdct_line, int block_type, float xmin[8][FA_SWB_NUM_MAX], float qcof);

void fa_aacpsy_calculate_xmin_usepsych1(uintptr_t handle, float *mdct_line, int block_type, float xmin[8][FA_SWB_NUM_MAX]);

#ifdef __cplusplus 
}
#endif  



#endif
