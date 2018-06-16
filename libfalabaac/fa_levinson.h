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


  filename: fa_levinson.h 
  version : v1.0.0
  time    : 2012/11/17 15:19 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_LEVINSON_H
#define _FA_LEVINSON_H 

#ifdef __cplusplus 
extern "C"
{ 
#endif  

#define FA_LEVINSON_ORDER_MAX   64

void fa_levinson(float *r,    int p, 
                 float *acof, float *kcof, float *err);
void fa_levinson1(float *r, int p, 
                  float *acof, float *kcof, float *err);
void fa_levinson_hp(double *r,    int p, 
                    double *acof, double *kcof, double *err);
void fa_levinson1_hp(double *r, int p, 
                     double *acof, double *kcof, double *err);


int  fa_atlvs(float *r, int n, float *b, 
              float *x, float *kcof, float *err);
int  fa_atlvs_hp(double *r, int n, double *b, 
                 double *x, double *kcof, double *err);

#ifdef __cplusplus 
}
#endif  



#endif
