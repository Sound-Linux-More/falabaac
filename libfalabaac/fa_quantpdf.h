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


  filename: fa_quantpdf.h 
  version : v1.0.0
  time    : 2012/12/20
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_QUANTPDF_H 
#define _FA_QUANTPDF_H 

typedef struct _fa_qpdf_para_t {
    float alpha;
    float beta;
    float a2;
    float a4;

} fa_qpdf_para_t;


extern int fa_protect_db_48k_long[49];
extern int fa_protect_db_48k_short[14];

extern int fa_protect_db_44k_long[49];
extern int fa_protect_db_44k_short[14];

extern int fa_protect_db_32k_long[51];
extern int fa_protect_db_32k_short[14];


#ifdef __cplusplus 
extern "C"
{ 
#endif  

void  fa_protect_db_rom_init();
float fa_get_subband_power(float *X, int kmin, int kmax);
float fa_get_subband_abspower(float *X, int kmin, int kmax);
float fa_get_subband_sqrtpower(float *X, int kmin, int kmax);
void  fa_get_subband_abssqrtpower(float *X, int kmin, int kmax, float *Px1, float *Px2);
void fa_get_subband_abssqrtpower_improve(float *X, int kmin, int kmax, float *Px1, float *Px2, float *Px3);

float fa_get_scaling_para(int scale_factor);
int   fa_mpeg_round(float x);
float fa_inverse_error_func(float alpha);
float fa_get_pdf_beta(float alpha);
int   fa_estimate_sf(float T, int K, float beta,
                     float a2, float a4, float miu, float miuhalf);
int   fa_estimate_sf_fast(float T, float t);
int   fa_estimate_sf_fast_improve(float T, float t, float miu2);
float fa_pow2db(float power);
float fa_db2pow(float db);
void  fa_adjust_thr(int subband_num, 
                    float *Px, float *Tm, float *G, 
                    float *Ti, float *Ti1);
void  fa_quantqdf_para_init(fa_qpdf_para_t *f, float alpha);

#ifdef __cplusplus 
}
#endif  



#endif
