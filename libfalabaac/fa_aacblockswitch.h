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


  filename: fa_aacblockswitch.h 
  version : v1.0.0
  time    : 2012/10/27 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_AACBLOCKSWITCH_H
#define _FA_AACBLOCKSWITCH_H

#include "fa_aacenc.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  

int fa_blockswitch_psy(aacenc_ctx_t *s);
int fa_blockswitch_var(aacenc_ctx_t *s);

uintptr_t fa_blockswitch_init(int block_len);
void fa_blockswitch_uninit(uintptr_t handle);

int fa_blockswitch_robust(aacenc_ctx_t *s, float *sample_buf);
int fa_blocksync(fa_aacenc_ctx_t *f);



#ifdef __cplusplus 
}
#endif  

#endif
