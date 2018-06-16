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


  filename: fa_quantpdf.c 
  version : v1.0.0
  time    : 2012/12/20
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "fa_quantpdf.h"
#include "fa_fastmath.h"

#ifndef M_PI
#define M_PI			3.14159265358979323846
#endif

#define M_PI_SQRT       1.7724538509
#define M_PI_SQRT_HALF  0.886226925451
#define SQRT2           1.41421356237

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef FA_ABS 
#define FA_ABS(a)    ( (a) > 0 ? (a) : (-(a)) )
#endif



int fa_protect_db_48k_long[49];
int fa_protect_db_48k_short[14];

int fa_protect_db_44k_long[49];
int fa_protect_db_44k_short[14];

int fa_protect_db_32k_long[51];
int fa_protect_db_32k_short[14];


void  fa_protect_db_rom_init()
{
    int i;

    /*48k long*/
    for (i = 0; i < 49; i++) {
        if (i < 3)
            fa_protect_db_48k_long[i] = 9;
        else if (i < 27)
            fa_protect_db_48k_long[i] = 8;
        else if (i < 31)
            fa_protect_db_48k_long[i] = 8;
        else if (i < 42)
            fa_protect_db_48k_long[i] = 7;
        else 
            fa_protect_db_48k_long[i] = 0;
    }

    /*48k short*/
    for (i = 0; i < 14; i++) {
        if (i < 1)
            fa_protect_db_48k_short[i] = 8;
        else if (i < 2)
            fa_protect_db_48k_short[i] = 7;
        else if (i < 11) 
            fa_protect_db_48k_short[i] = 7;
        else 
            fa_protect_db_48k_short[i] = 0;
    }

    /*44k long*/
#if 1 

    for (i = 0; i < 49; i++) {
        if (i < 3)
            fa_protect_db_44k_long[i] = 9;
        else if (i < 27)
            fa_protect_db_44k_long[i] = 8;
        else if (i < 31)
            fa_protect_db_44k_long[i] = 8;
        else if (i < 42)
            fa_protect_db_44k_long[i] = 7;
        else 
            fa_protect_db_44k_long[i] = 0;
    }
    
#else 
 
    for (i = 0; i < 49; i++) {
        if (i < 3)
            fa_protect_db_44k_long[i] = 9;
        else if (i < 27)
            fa_protect_db_44k_long[i] = 8;
        else if (i < 31)
            fa_protect_db_44k_long[i] = 7;
        else if (i < 42)
            fa_protect_db_44k_long[i] = 4;
        else 
            fa_protect_db_44k_long[i] = 0;
    }
    
#endif

    /*44k short*/
#if  1 
    for (i = 0; i < 14; i++) {
        if (i < 1)
            fa_protect_db_44k_short[i] = 8;//7;
        else if (i < 2)
            fa_protect_db_44k_short[i] = 7;//3;
        else if (i < 11) 
            fa_protect_db_44k_short[i] = 7;//2;
        else 
            fa_protect_db_44k_short[i] = 0;//0;
    }

#else 
    for (i = 0; i < 14; i++) {
        if (i < 1)
            fa_protect_db_44k_short[i] = 8;
        else if (i < 2)
            fa_protect_db_44k_short[i] = 5;
        else if (i < 11) 
            fa_protect_db_44k_short[i] = 2;
        else 
            fa_protect_db_44k_short[i] = 0;
    }

#endif

    /*32k long*/
    for (i = 0; i < 51; i++) {
        if (i < 3)
            fa_protect_db_32k_long[i] = 10;
        else if (i < 27)
            fa_protect_db_32k_long[i] = 9;
        else if (i < 31)
            fa_protect_db_32k_long[i] = 9;
        else if (i < 42)
            fa_protect_db_32k_long[i] = 7;
        else 
            fa_protect_db_32k_long[i] = 0;
    }

    /*32k short*/
    for (i = 0; i < 14; i++) {
        if (i < 1)
            fa_protect_db_32k_short[i] = 9;
        else if (i < 2)
            fa_protect_db_32k_short[i] = 8;
        else if (i < 11) 
            fa_protect_db_32k_short[i] = 7;
        else 
            fa_protect_db_32k_short[i] = 0;
    }


}

float fa_get_subband_power(float *X, int kmin, int kmax)
{
    int k;
    float Px;

    Px = 0.0;
    for (k = kmin; k <= kmax; k++) 
        Px += X[k] * X[k];

    return Px;
}

float fa_get_subband_abspower(float *X, int kmin, int kmax)
{
    int k;
    float Px;

    Px = 0.0;
    for (k = kmin; k <= kmax; k++) 
        Px += FA_ABS(X[k]);

    return Px;
}


float fa_get_subband_sqrtpower(float *X, int kmin, int kmax)
{
    int k;
    float tmp;
    float Px;

    Px = 0.0;
    for (k = kmin; k <= kmax; k++) {
        tmp = FA_ABS(X[k]);
        Px += FA_SQRTF(tmp);
    }

    return Px;
}

void fa_get_subband_abssqrtpower(float *X, int kmin, int kmax, float *Px1, float *Px2)
{
    int k;
    float tmp;
    float tPx1, tPx2;

    tPx1 = 0.0;
    tPx2 = 0.0;
    for (k = kmin; k <= kmax; k++) {
        tmp = FA_ABS(X[k]);
        tPx1 += tmp;
        tPx2 += FA_SQRTF(tmp);
    }

    *Px1 = tPx1;
    *Px2 = tPx2;
}

void fa_get_subband_abssqrtpower_improve(float *X, int kmin, int kmax, float *Px1, float *Px2, float *Px3)
{
    int k;
    float tmp;
    float tPx1, tPx2, tPx3;

    tPx1 = 0.0;
    tPx2 = 0.0;
    tPx3 = 0.0;
    for (k = kmin; k <= kmax; k++) {
        tmp = FA_ABS(X[k]);
        tPx1 += tmp;
        tPx2 += FA_SQRTF(tmp);
        tPx3 += tmp*tmp;
    }

    *Px1 = tPx1;
    *Px2 = tPx2;
    *Px3 = tPx3;
}



float fa_get_scaling_para(int scale_factor)
{
    return pow(2, (float)scale_factor/4.0);
}

int fa_mpeg_round(float x)
{
    int rx;

    if (x > 0) {
        rx = (int)(x + 0.4054);
    } else {
        /*rx = -1 * (int)(-x + 0.4054);*/
        rx = -1 * (int)(-x - 0.4054);
    }

    return rx;
}

float fa_inverse_error_func(float alpha)
{
    float erf_inv = 0.0;

    erf_inv = alpha + 
              (M_PI/12.) * alpha * alpha * alpha + 
              ((7*M_PI*M_PI)/480.) * alpha * alpha * alpha * alpha * alpha +
              ((127*M_PI*M_PI*M_PI)/40320.) * alpha * alpha * alpha * alpha * alpha * alpha * alpha;

    erf_inv = erf_inv * M_PI_SQRT_HALF;

    return erf_inv;
}

float fa_get_pdf_beta(float alpha)
{
    float beta;

    beta = SQRT2 * fa_inverse_error_func(2.*alpha - 1);

    return beta;
}

int   fa_estimate_sf(float T, int K, float beta,
                     float a2, float a4, float miu, float miuhalf)
{
    float ratio;
    float diff;
    int sf;
    float t;


    diff = a4*miu - a2*a2*miuhalf*miuhalf;
    /*if (diff < 0)*/
        /*printf("diff=%f\n", diff);*/
    /*assert(diff >= 0);*/

    t = K*a2*miuhalf + beta*FA_SQRTF(2*K*diff);
    if (t > 0) {
        ratio = T/t;
        sf = fa_mpeg_round((8./3.) * FA_LOG2(ratio));
    } else {
        sf = 0;
    }

    return sf;
}


int   fa_estimate_sf_fast(float T, float t)
{
    float ratio;
    int sf;

    if (t > 0) {
        ratio = T/t;
        sf = fa_mpeg_round((8./3.) * FA_LOG2(ratio));
    } else {
        sf = 0;
    }
/*
    if (sf < 0)
        sf = 0;
*/
    return sf;
}

int   fa_estimate_sf_fast_improve(float T, float t, float miu2)
{
    float ratio;
    int sf;

    if (t > 0) {
        ratio = T/(t*sqrt(miu2*sqrt(miu2)));
        sf = fa_mpeg_round((8./3.) * FA_LOG2(ratio) + 2 * FA_LOG2(miu2));
    } else {
        sf = 0;
    }
/*
    if (sf < 0)
        sf = 0;
*/
    return sf;
}





float fa_pow2db(float power)
{
    return 10*FA_LOG10(power);
}

float fa_db2pow(float db)
{
    return pow(10, 0.1*db);
}


void fa_adjust_thr(int subband_num, 
                   float *Px, float *Tm, float *G, 
                   float *Ti, float *Ti1)
{
    int s;
    float Ti1_tmp;
    float mi;
    float r1, r2;

    r1 = 1.0;
    r2 = 0.25;

    mi = 10000000;
    for (s = 0; s < subband_num; s++) {
        if (Ti1[s] < mi)
            mi = Ti1[s];
    }

    mi = FA_MAX(mi, 0);

#if  1 

    for (s = 0; s < subband_num; s++) {
        if (Px[s] <= Tm[s]) {                           //masked by thr
            Ti[s] = Px[s];
        } else {                                        //unmasked 
            /*if ((Ti[s] - Tm[s]) < 6.0) {                //high SNR, use constant NMR adjust*/
            if ((Ti[s] - Tm[s]) < 15.0) {                //high SNR, use constant NMR adjust
                /*assert(Ti[s] >= Tm[s]);*/
                Ti1_tmp = Ti[s] + r1;
                Ti[s]   = FA_MIN(Ti1_tmp, G[s]);
                Ti[s]   = FA_MAX(Ti[s], Tm[s]);
                Ti1[s]  = Ti1_tmp;
            } else if (Ti[s] < G[s]) {                  //low SNR, use water-filling adjust
                Ti1_tmp = FA_MAX(Ti1[s], mi+r1);
                Ti[s]   = FA_MIN(Ti1_tmp, G[s]);
                /*printf("-----------tt1\n");*/
            } else {                                    //very low SNR, small constant adjust
                Ti[s] += r2;
                /*printf("-----------tt2\n");*/
            }
        }
    }
#else 

    for (s = 0; s < subband_num; s++) {
        if (Px[s] <= Tm[s]) {                           //masked by thr
            Ti[s] = Px[s];
            /*printf("-----------ttx\n");*/
        } else {                                        //unmasked 
            if ((Ti[s] - Tm[s]) < 15.0) {                //high SNR, use constant NMR adjust
                /*assert(Ti[s] >= Tm[s]);*/
                Ti1_tmp = Ti[s] + 1;//r1;
                Ti[s]   = FA_MIN(Ti1_tmp, G[s]);
                Ti[s]   = FA_MAX(Ti[s], Tm[s]);
                Ti1[s]  = Ti1_tmp;
                /*printf("-----------tt0\n");*/
            } else if (Ti[s] < G[s]) {                  //low SNR, use water-filling adjust
                Ti1_tmp = FA_MAX(Ti1[s], mi+r1);
                Ti[s]   = FA_MIN(Ti1_tmp, G[s]);
                /*printf("-----------tt1\n");*/
            } else {                                    //very low SNR, small constant adjust
                Ti[s] += r2;
                /*printf("-----------tt2\n");*/
            }
        }
    }

#endif

}

void fa_quantqdf_para_init(fa_qpdf_para_t *f, float alpha)
{
    f->alpha = alpha;
    f->beta  = fa_get_pdf_beta(alpha);
    f->a2    = 4./27.;
    f->a4    = 16./405.;
}

