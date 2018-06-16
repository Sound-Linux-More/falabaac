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


  filename: fa_mdct.c 
  version : v1.0.0
  time    : 2012/07/16 - 2012/07/18  
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "fa_mdct.h"
#include "fa_fft.h"

#ifndef		M_PI
#define		M_PI							3.14159265358979323846
#endif

#undef  EPS
#define EPS		1E-16

#ifndef FA_CMUL
/*#define FA_CMUL(dre, dim, are, aim, bre, bim) do { \*/
        /*(dre) = (are) * (bre) - (aim) * (bim);  \*/
        /*(dim) = (are) * (bim) + (aim) * (bre);  \*/
    /*} while (0)*/
#define FA_CMUL(dre, dim, are, aim, bre, bim)  { \
        (dre) = (are) * (bre) - (aim) * (bim);  \
        (dim) = (are) * (bim) + (aim) * (bre);  \
    } 
#endif

typedef struct _fa_mdct_ctx_t {
    int     type;
    int     length;

    /*fft used by mdct1 and mdct2, shared by pos and inv*/
    uintptr_t h_fft;
    float   *fft_buf;

    /*mdct0*/
    float   *mdct_work;
    float   **cos_ang_pos;
    float   **cos_ang_inv;

    /*mdct1*/
    float   *pre_c_pos, *pre_s_pos;
    float   *c_pos    , *s_pos;
    float   *pre_c_inv, *pre_s_inv;
    float   *c_inv    , *s_inv;

    /*mdct2*/
    float   *tw_c, *tw_s;
    float   *rot; 
    float   sqrt_cof;

    void    (*mdct_func)(struct _fa_mdct_ctx_t *f, const float *x, float *X, int N);
    void    (*imdct_func)(struct _fa_mdct_ctx_t *f, const float *X, float *x, int N2);

}fa_mdct_ctx_t;


static float ** matrix_init(int Nrow, int Ncol)
{
    float **A;
    int i;

    /* Allocate the row pointers */
    A = (float **) malloc (Nrow * sizeof (float *));

    /* Allocate the matrix of float values */
    A[0] = (float *) malloc (Nrow * Ncol * sizeof (float));
    memset(A[0], 0, Nrow*Ncol*sizeof(float));

    /* Set up the pointers to the rows */
    for (i = 1; i < Nrow; ++i)
        A[i] = A[i-1] + Ncol;

    return A;
}

static void matrix_uninit(float **A)
{
    /*free the Nrow*Ncol data space*/
    if (A[0])
        free(A[0]);

    /*free the row pointers*/
    if (A)
        free(A);

}

int fa_mdct_sine(float *w, int N)
{
    float tmp;
    int   n;

    for (n = 0; n < N; n++) {
        tmp = (M_PI/N) * (n + 0.5);
        w[n] = sin(tmp); 
    }

    return N;
}

static float bessel(float x)
{
  	float  xh, sum, pow, ds;
 	int k;

  	xh  = (float)0.5 * x;
  	sum = 1.0;
  	pow = 1.0;
  	k   = 0;
  	ds  = 1.0;
 	while (ds > sum * EPS) {
        ++k;
        pow = pow * (xh / k);
        ds = pow * pow;
        sum = sum + ds;
  	}

 	return sum;	
}

static int kaiser_beta(float *w, const int N, const float beta)
{
	int i;
	float Ia,Ib;

	for (i = 0 ; i < N ; i++) {
		float x;
		
		Ib = bessel(beta);
		
		x = (float)((2. *i/(N-1))-1);
		Ia = bessel(beta*(float)sqrt(1. - x*x));
		
		w[i] = (float)(Ia/Ib);
	}

	return N;
}

/*you can read details of the KBD formula from wiki, google "mdct Kaiser wiki"*/
int fa_mdct_kbd(float *w, int N, float alpha)
{
    int i,j;
    int N2;
    float *w1;
    float sum = 0.0;
    float tmp = 0.0;

    N2 = N >> 1;
    w1 = (float *)malloc(sizeof(float)*(N2+1));

    /*generate kaiser-basel window*/
    kaiser_beta(w1, N2+1, alpha*M_PI);

    /*calculate the denuminotor sum*/
    for (i = 0; i < N2+1; i++) 
        sum += w1[i];
    sum = 1.0/sum;

    /*calculate the KBD, window symmetric*/
    tmp = 0.0;
    for (i = 0, j = N-1; i < N2; i++, j--) {
        tmp += w1[i];
        w[i] = w[j] = sqrt(tmp*sum);
    }

    free(w1);

    return N;
}


static void mdct0(fa_mdct_ctx_t *f, const float *x, float *X, int N)
{
    int k, n;
    int N2;

    float Xk;

    N2 = N >> 1;

    for (k = 0; k < N2; k++) {
        Xk = 0;
        for (n = 0; n < N; n++) {
            Xk += x[n] * f->cos_ang_pos[k][n];
        }
        X[k] = Xk;
    }

}

static void imdct0(fa_mdct_ctx_t *f, const float *X, float *x, int N2)
{
    int n, k;
    int N;

    float xn;

    N = N2 << 1;

    for (n = 0; n < N; n++) {
        xn = 0;
        for (k = 0; k < N2; k++) {
            xn += X[k] * f->cos_ang_inv[n][k];
        }
        x[n] = (xn * 4)/N;
    }

}


static void mdct1(fa_mdct_ctx_t *f, const float *x, float *X, int N)
{
    int k;
    int N2;

    N2 = N >> 1;

    for (k = 0; k < N; k++) {
        f->fft_buf[k+k]   = x[k] * f->pre_c_pos[k];
        f->fft_buf[k+k+1] = x[k] * f->pre_s_pos[k];
    }

    fa_fft(f->h_fft, f->fft_buf);

    for (k = 0; k < N2; k++) 
        X[k] = f->fft_buf[k+k] * f->c_pos[k] - f->fft_buf[k+k+1] * f->s_pos[k];

}

static void imdct1(fa_mdct_ctx_t *f, const float *X, float *x, int N2)
{
    int k, i;
    int N;

    N = N2 << 1;

    for (k = 0; k < N2; k++) {
        f->fft_buf[k+k]   = X[k] * f->pre_c_inv[k];
        f->fft_buf[k+k+1] = X[k] * f->pre_s_inv[k];
    }
    for (k = N2, i = N2 -1; k < N; k++, i--) {
        f->fft_buf[k+k]   = -X[i] * f->pre_c_inv[k];
        f->fft_buf[k+k+1] = -X[i] * f->pre_s_inv[k];
    }

    fa_ifft(f->h_fft, f->fft_buf);

    for (k = 0; k < N; k++) 
        x[k] = 2 * (f->fft_buf[k+k] * f->c_inv[k] - f->fft_buf[k+k+1] * f->s_inv[k]);

}

static void mdct2(fa_mdct_ctx_t *f, const float *x, float *X, int N)
{
    int k;
    int N2;
    int N4;
    float re, im;
    float *rot = f->rot;

    N2 = N >> 1;
    N4 = N >> 2;

    memset(rot, 0, sizeof(float)*f->length);
    /*shift x*/
    for (k = 0; k < N4; k++) 
        rot[k] = -x[k+3*N4];
    for (k = N4; k < N; k++)
        rot[k] = x[k-N4];

    /*pre twiddle*/
    for (k = 0; k < N4; k++) {
        re = rot[2*k]      - rot[N-1-2*k];
        im = rot[N2-1-2*k] - rot[N2+2*k] ;
        f->fft_buf[k+k]   = 0.5 * (re * f->tw_c[k] - im * f->tw_s[k]);
        f->fft_buf[k+k+1] = 0.5 * (re * f->tw_s[k] + im * f->tw_c[k]);
    }

    fa_fft(f->h_fft, f->fft_buf);

    /*post twiddle*/
    for (k = 0; k < N4; k++) {
        re = f->fft_buf[k+k];
        im = f->fft_buf[k+k+1];
        X[2*k]      =  2 * (re * f->tw_c[k] - im * f->tw_s[k]);
        X[N2-1-2*k] = -2 * (re * f->tw_s[k] + im * f->tw_c[k]);
    }

}

static void imdct2(fa_mdct_ctx_t *f, const float *X, float *x, int N2)
{
    int k;
    int N;
    int N4;
    float re, im;
    float *rot = f->rot;
    float cof = f->sqrt_cof;

    N  = N2 << 1;
    N4 = N2 >> 1;

    memset(rot, 0, sizeof(float)*f->length);

    /*pre twiddle*/
    for (k = 0; k < N4; k++) {
        re = X[2*k];
        im = X[N2-1-2*k];
        f->fft_buf[k+k]   = 0.5 * (re * f->tw_c[k] - im * f->tw_s[k]);
        f->fft_buf[k+k+1] = 0.5 * (re * f->tw_s[k] + im * f->tw_c[k]);
    }

    fa_fft(f->h_fft, f->fft_buf);

    /*post twiddle*/
    for (k = 0; k < N4; k++) {
        re = f->fft_buf[k+k];
        im = f->fft_buf[k+k+1];
        f->fft_buf[k+k]   = 8 * cof * (re * f->tw_c[k] - im * f->tw_s[k]);
        f->fft_buf[k+k+1] = 8 * cof * (re * f->tw_s[k] + im * f->tw_c[k]);
    }

    /*shift*/
    for (k = 0; k < N4; k++) {
        rot[2*k]    = f->fft_buf[k+k];
        rot[N2+2*k] = f->fft_buf[k+k+1];
    }
    for (k = 1; k < N; k+=2)
        rot[k] = -rot[N-1-k];

    for (k = 0; k < 3*N4; k++)
        x[k] = rot[N4+k] * cof;
    for (k = 3*N4; k < N; k++)
        x[k] = -rot[k-3*N4] * cof;

}

uintptr_t fa_mdct_init(int type, int size)
{
    int   k, n;
    float tmp;
    int   base;
    int   length;
    fa_mdct_ctx_t *f = NULL;

    f = (fa_mdct_ctx_t *)malloc(sizeof(fa_mdct_ctx_t));
    memset(f, 0, sizeof(fa_mdct_ctx_t));

    base = (int)(log(size)/log(2));
    if ((1<<base) < size)
        base += 1;

    length    = (1 << base);
    f->length = length;

    f->type = type;

    switch (type) {
        case MDCT_ORIGIN:{
                             /*mdct0 init*/
                             f->mdct_work   = (float *)malloc(sizeof(float)*(length>>1));
                             memset(f->mdct_work, 0, sizeof(float)*(length>>1));
                             f->cos_ang_pos = (float **)matrix_init(length>>1, length);
                             f->cos_ang_inv = (float **)matrix_init(length   , length>>1);

                             for (k = 0; k < (length>>1); k++) {
                                 for (n = 0; n < length; n++) {
                                     /* formula: cos( (pi/(2*N)) * (2*n + 1 + N/2) * (2*k + 1) ) */
                                     tmp = (M_PI/(2*length)) * (2*n + 1 + (length>>1)) * (2*k + 1);
                                     f->cos_ang_pos[k][n] = f->cos_ang_inv[n][k] = cos(tmp);
                                 }
                             }

                             f->mdct_func  = mdct0;
                             f->imdct_func = imdct0;
                             break;
                         }
        case MDCT_FFT:   {
                             float n0;

                             n0 = ((float)length/2 + 1) / 2;

                             f->h_fft   = fa_fft_init(length);
                             f->fft_buf = (float *)malloc(sizeof(float)*length*2);

                             /*positive transform --> mdct*/
                             f->pre_c_pos = (float *)malloc(sizeof(float)*length);
                             f->pre_s_pos = (float *)malloc(sizeof(float)*length);
                             f->c_pos     = (float *)malloc(sizeof(float)*(length>>1));
                             f->s_pos     = (float *)malloc(sizeof(float)*(length>>1));

                             for (k = 0; k < length; k++) {
                                 f->pre_c_pos[k] = cos(-(M_PI*k)/length);
                                 f->pre_s_pos[k] = sin(-(M_PI*k)/length);
                             }
                             for (k = 0; k < (length>>1); k++) {
                                 f->c_pos[k] = cos(-2*M_PI*n0*(k+0.5)/length); 
                                 f->s_pos[k] = sin(-2*M_PI*n0*(k+0.5)/length); 
                             }


                             /*inverse transform -->imdct*/
                             f->pre_c_inv = (float *)malloc(sizeof(float)*length);
                             f->pre_s_inv = (float *)malloc(sizeof(float)*length);
                             f->c_inv     = (float *)malloc(sizeof(float)*length);
                             f->s_inv     = (float *)malloc(sizeof(float)*length);

                             for (k = 0; k < length; k++) {
                                 f->pre_c_inv[k] = cos((2*M_PI*k*n0)/length);
                                 f->pre_s_inv[k] = sin((2*M_PI*k*n0)/length);
                             }
                             for (k = 0; k < length; k++) {
                                 f->c_inv[k] = cos(M_PI*(k+n0)/length); 
                                 f->s_inv[k] = sin(M_PI*(k+n0)/length); 
                             }

                             f->mdct_func  = mdct1;
                             f->imdct_func = imdct1;
                             break;
                         }
        case MDCT_FFT4:  {
                             f->h_fft   = fa_fft_init(length>>2);
                             f->fft_buf = (float *)malloc(sizeof(float)*(length>>1));
                             f->sqrt_cof = 1./sqrt(length);

                             f->rot = (float *)malloc(sizeof(float)*length);
                             f->tw_c = (float *)malloc(sizeof(float)*(length>>2));
                             f->tw_s = (float *)malloc(sizeof(float)*(length>>2));

                             memset(f->rot, 0, sizeof(float)*length);
                             for (k = 0; k < (length>>2); k++) {
                                 f->tw_c[k] = cos(-2*M_PI*(k+0.125)/length);
                                 f->tw_s[k] = sin(-2*M_PI*(k+0.125)/length);
                             }

                             f->mdct_func  = mdct2;
                             f->imdct_func = imdct2;
                             break;
                         }

    }

    return (uintptr_t)f;
}


static void free_mdct_origin(fa_mdct_ctx_t *f)
{
    if (f->cos_ang_pos) {
        matrix_uninit(f->cos_ang_pos);
        f->cos_ang_pos = NULL;
    }

    if (f->cos_ang_inv) {
        matrix_uninit(f->cos_ang_inv);
        f->cos_ang_inv = NULL;
    }

    if (f->mdct_work) {
        free(f->mdct_work);
        f->mdct_work = NULL;
    }
}

static void free_mdct_fft(fa_mdct_ctx_t *f)
{
    if (f->pre_c_pos) {
        free(f->pre_c_pos);
        f->pre_c_pos = NULL;
    }

    if (f->pre_s_pos) {
        free(f->pre_s_pos);
        f->pre_s_pos = NULL;
    }

    if (f->pre_c_inv) {
        free(f->pre_c_inv);
        f->pre_c_inv = NULL;
    }

    if (f->pre_s_inv) {
        free(f->pre_s_inv);
        f->pre_s_inv = NULL;
    }

    if (f->c_pos) {
        free(f->c_pos);
        f->c_pos = NULL;
    }
 
    if (f->s_pos) {
        free(f->s_pos);
        f->s_pos = NULL;
    }

    if (f->c_inv) {
        free(f->c_inv);
        f->c_inv = NULL;
    }

    if (f->s_inv) {
        free(f->s_inv);
        f->s_inv = NULL;
    }

    if (f->fft_buf) {
        free(f->fft_buf);
        f->fft_buf = NULL;
    }

    fa_fft_uninit(f->h_fft);

}

static void free_mdct_fft4(fa_mdct_ctx_t *f)
{
    if (f->fft_buf) {
        free(f->fft_buf);
        f->fft_buf = NULL;
    }

    if (f->tw_c) {
        free(f->tw_c);
        f->tw_c = NULL;
    }

    if (f->tw_s) {
        free(f->tw_s);
        f->tw_s = NULL;
    }

    if (f->rot) {
        free(f->rot);
        f->rot = NULL;
    }

    fa_fft_uninit(f->h_fft);
}


void fa_mdct_uninit(uintptr_t handle)
{
    int type;

    fa_mdct_ctx_t *f = (fa_mdct_ctx_t *)handle;
    type = f->type;

    if (f) {
        switch (type) {
            case MDCT_ORIGIN:
                free_mdct_origin(f);
                break;
            case MDCT_FFT:
                free_mdct_fft(f);
                break;
            case MDCT_FFT4:
                free_mdct_fft4(f);
                break;
        }
        free(f);
        f = NULL;
    }

}

void fa_mdct(uintptr_t handle, float *x, float *X)
{
    fa_mdct_ctx_t *f = (fa_mdct_ctx_t *)handle;

    f->mdct_func(f, x, X, f->length); 
}


void fa_imdct(uintptr_t handle, float *X, float *x)
{
    fa_mdct_ctx_t *f = (fa_mdct_ctx_t *)handle;

    f->imdct_func(f, X, x, (f->length)>>1); 

}
