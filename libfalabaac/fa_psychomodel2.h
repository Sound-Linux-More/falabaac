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


  filename: fa_psychomodel2.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_PSYCHOMODEL2_H
#define _FA_PSYCHOMODEL2_H 

#include "fa_inttypes.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


uintptr_t fa_psychomodel2_init(int cbands_num, int *w_low, float *barkval, float *qsthr, 
                               int swb_num   , int *swb_offset,
                               int iblen);
void fa_psychomodel2_uninit(uintptr_t handle);
void fa_psychomodel2_calculate_pe(uintptr_t handle, float *x, float *pe);
void fa_psychomodel2_calculate_xmin(uintptr_t handle, float *mdct_line, float *xmin, float qcof);
void fa_psychomodel2_calculate_pe_improve(uintptr_t handle, float *x, float *pe, int *tns_active,
                                          float tmn, float nmt, float nb_cof, int do_precho_ctrl);
void fa_psychomodel2_calculate_xmin_short(uintptr_t handle, float xmin[8][51]);
 

void fa_psychomodel2_get_mag_prev1(uintptr_t handle, float *mag, int *len);
void fa_psychomodel2_get_mag_prev2(uintptr_t handle, float *mag, int *len);
void fa_psychomodel2_get_phi_prev1(uintptr_t handle, float *phi, int *len);
void fa_psychomodel2_get_phi_prev2(uintptr_t handle, float *phi, int *len);

void fa_psychomodel2_set_mag_prev1(uintptr_t handle, float *mag, int len);
void fa_psychomodel2_set_mag_prev2(uintptr_t handle, float *mag, int len);
void fa_psychomodel2_set_phi_prev1(uintptr_t handle, float *phi, int len);
void fa_psychomodel2_set_phi_prev2(uintptr_t handle, float *phi, int len);

void fa_psychomodel2_reset_nb_prev(uintptr_t handle);
void fa_psychomodel2_reset_mag_prev1(uintptr_t handle);
void fa_psychomodel2_reset_mag_prev2(uintptr_t handle);
void fa_psychomodel2_reset_phi_prev1(uintptr_t handle);
void fa_psychomodel2_reset_phi_prev2(uintptr_t handle);

#ifdef __cplusplus 
}
#endif  


#endif
