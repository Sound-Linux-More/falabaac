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


  filename: fa_aacquant.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/11/24 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_AACQUANT_H
#define _FA_AACQUANT_H 

#include "fa_aacenc.h"
#include "fa_swbtab.h"

#ifndef NUM_SFB_MAX
#define NUM_SFB_MAX           FA_SWB_NUM_MAX
#endif

void fa_quantize_loop(fa_aacenc_ctx_t *f);
void fa_quantize_fast(fa_aacenc_ctx_t *f);
void fa_quantize_best(fa_aacenc_ctx_t *f);

void fa_calculate_scalefactor_win(aacenc_ctx_t *s, float xmin[8][NUM_SFB_MAX]);

void fa_fastquant_calculate_sfb_avgenergy(aacenc_ctx_t *s);
void fa_fastquant_calculate_xmin(aacenc_ctx_t *s, float xmin[8][NUM_SFB_MAX], float qcof);

void fa_adjust_scalefactor(fa_aacenc_ctx_t *f);

#endif
