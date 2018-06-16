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


  filename: fa_bitstream.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_BITSTREAM_H
#define _FA_BITSTREAM_H 

#include "fa_inttypes.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


uintptr_t fa_bitstream_init(int num_bytes);
void      fa_bitstream_uninit(uintptr_t handle);

void fa_bitstream_reset(uintptr_t handle);
int  fa_bitstream_fillbuffer(uintptr_t handle, unsigned char *buf, int num_bytes);

int  fa_bitstream_putbits(uintptr_t handle, unsigned int value, int nbits);
int  fa_bitstream_getbits(uintptr_t handle, unsigned int *value, int nbits);

int  fa_bitstream_getbits_num(uintptr_t handle);

int  fa_bitstream_getbufval(uintptr_t handle, unsigned char *buf_out);

#ifdef __cplusplus 
}
#endif  


#endif
