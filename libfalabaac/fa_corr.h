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


  filename: fa_corr.h 
  version : v1.0.0
  time    : 2012/11/13 22:52
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_CORR_H
#define _FA_CORR_H 

#ifdef __cplusplus 
extern "C"
{ 
#endif  

void  fa_autocorr(float *x, int n, int p, float *r);
void  fa_autocorr_hp(double *x, int n, int p, double *r);

void  fa_crosscorr(float *x, float *y, int n, int p, float *r);
void  fa_crosscorr_hp(double *x, double *y, int n, int p, double *r);

float fa_corr_cof(float *a, float *b, int len);

#ifdef __cplusplus 
}
#endif  



#endif
