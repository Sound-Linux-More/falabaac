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


  filename: fa_lpc.h 
  version : v1.0.0
  time    : 2012/11/17 23:52
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_LPC_H
#define _FA_LPC_H 

#include "fa_inttypes.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  

//#define USE_LPC_HP


uintptr_t fa_lpc_init(int p);
void      fa_lpc_uninit(uintptr_t handle);

#ifdef USE_LPC_HP 
double fa_lpc(uintptr_t handle, double *x, int x_len, double *lpc_cof, double *kcof, double *err);
#else 
float fa_lpc(uintptr_t handle, float *x, int x_len, float *lpc_cof, float *kcof, float *err);
#endif
 

#ifdef __cplusplus 
}
#endif  



#endif
