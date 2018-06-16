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


  filename: fa_lpc.c 
  version : v1.0.0
  time    : 2012/11/17 23:52
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "fa_lpc.h"
#include "fa_levinson.h"
#include "fa_corr.h"

typedef struct _fa_lpc_t {
    int p;      //order

#ifdef USE_LPC_HP
    double *r;   //relation matrix will be used

    double *acof;
    double *kcof;
    double err;
#else 
    float *r;   //relation matrix will be used

    float *acof;
    float *kcof;
    float err;
#endif

} fa_lpc_t;


uintptr_t fa_lpc_init(int p)
{
    fa_lpc_t *f = NULL;

    f = (fa_lpc_t *)malloc(sizeof(fa_lpc_t));

    memset(f, 0, sizeof(fa_lpc_t));

    f->p = p;

#ifdef USE_LPC_HP
    f->r = (double *)malloc(sizeof(double)*(p+1));
    memset(f->r, 0, sizeof(double)*(p+1));
    f->acof = (double *)malloc(sizeof(double)*(p+1));
    memset(f->acof, 0, sizeof(double)*(p+1));
    f->kcof = (double*)malloc(sizeof(double)*(p+1));
    memset(f->kcof, 0, sizeof(double)*(p+1));
    f->err = 0.0;
#else 
    f->r = (float *)malloc(sizeof(float)*(p+1));
    memset(f->r, 0, sizeof(float)*(p+1));
    f->acof = (float *)malloc(sizeof(float)*(p+1));
    memset(f->acof, 0, sizeof(float)*(p+1));
    f->kcof = (float *)malloc(sizeof(float)*(p+1));
    memset(f->kcof, 0, sizeof(float)*(p+1));
    f->err = 0.0;
#endif
 
    return (uintptr_t)f;
}

void      fa_lpc_uninit(uintptr_t handle)
{
    fa_lpc_t *f = (fa_lpc_t *)handle;

    if (f) {
        free(f->r);
        f->r = NULL;

        free(f->acof);
        f->acof = NULL;
        free(f->kcof);
        f->kcof = NULL;

        free(f);
        f = NULL;
    }
}

#ifdef USE_LPC_HP 

double fa_lpc(uintptr_t handle, double *x, int x_len, double *lpc_cof, double *kcof, double *err)
{
    fa_lpc_t *f = (fa_lpc_t *)handle;
    int k;
    double gain;

    /*caculate autorelation matrix*/
    fa_autocorr_hp(x, x_len, f->p, f->r);

    /*use levinson-durbin algorithm to calculate solution coffients*/
    fa_levinson_hp(f->r, f->p, f->acof, f->kcof, &(f->err));

    *err = f->err / x_len;
    for (k = 0; k <= f->p; k++) {
        lpc_cof[k] = f->acof[k];
        kcof[k]    = f->kcof[k];
    }

    /*gain = f->err / f->r[0];*/
    if (f->err > 0)
        gain = f->r[0] / f->err;
    else 
        gain = 0.0;


    return gain;
}


#else 

float fa_lpc(uintptr_t handle, float *x, int x_len, float *lpc_cof, float *kcof, float *err)
{
    fa_lpc_t *f = (fa_lpc_t *)handle;
    int k;
    float gain;

    /*caculate autorelation matrix*/
    fa_autocorr(x, x_len, f->p, f->r);

    /*use levinson-durbin algorithm to calculate solution coffients*/
    fa_levinson(f->r, f->p, f->acof, f->kcof, &(f->err));

    *err = f->err / x_len;
    for (k = 0; k <= f->p; k++) {
        lpc_cof[k] = f->acof[k];
        kcof[k]    = f->kcof[k];
    }

    /*gain = f->err / f->r[0];*/
    if (f->err > 0)
        gain = f->r[0] / f->err;
    else 
        gain = 0.0;

    return gain;
}

#endif

