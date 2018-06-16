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


  filename: fa_tns.c 
  version : v1.0.0
  time    : 2012/11/20 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "fa_tns.h"
#include "fa_aaccfg.h"
#include "fa_mdctquant.h"
#include "fa_lpc.h"

#ifndef		M_PI
#define		M_PI							3.14159265358979323846
#endif

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif


static int tns_samplerate[13] =
{ 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,0 };

/* Limit bands to > 2.0 kHz */
static int tns_minband_long[12] =
{ 11, 12, 15, 16, 17, 20, 25, 26, 24, 28, 30, 31 };

static int tns_minband_short[12] =
{ 2, 2, 2, 3, 3, 4, 6, 6, 8, 10, 10, 12 };

/* Main/LC profile TNS parameters, if SSR add _ssr for the table in the future    */
static int tns_maxband_long[12] =
{ 31, 31, 34, 40, 42, 51, 46, 46, 42, 42, 42, 39 };

static int tns_maxband_short[12] =
{ 9, 9, 10, 14, 14, 14, 14, 14, 14, 14, 14, 14 };

#define TNS_MAX_ORDER_LONG_MAIN   20
#define TNS_MAX_ORDER_LONG_LC     12
#define TNS_MAX_ORDER_SHORT       7

   
uintptr_t fa_tns_init(int mpeg_version, int objtype, int sr_index)
{
    tns_info_t *f = (tns_info_t *)malloc(sizeof(tns_info_t));

    memset(f, 0, sizeof(tns_info_t));

    switch (objtype) {
    case MAIN:
    case LTP:
        f->tns_maxband_long = tns_maxband_long[sr_index];
        f->tns_maxband_short= tns_maxband_short[sr_index];
        if (mpeg_version == 1) { /* MPEG2 */
            f->tns_maxorder_long = TNS_MAX_ORDER_LONG_MAIN;
        } else { /* MPEG4 */
            if (sr_index <= 5) /* fs > 32000Hz */
                f->tns_maxorder_long = 12;
            else
                f->tns_maxorder_long = 20;
        }
        f->tns_maxorder_short = TNS_MAX_ORDER_SHORT;
        break;
    case LOW :
        f->tns_maxband_long = tns_maxband_long[sr_index];
        f->tns_maxband_short= tns_maxband_short[sr_index];
        if (mpeg_version == 1) { /* MPEG2 */
            f->tns_maxorder_long = TNS_MAX_ORDER_LONG_LC;
        } else { /* MPEG4 */
            if (sr_index <= 5) /* fs > 32000Hz */
                f->tns_maxorder_long = 12;//12; //12; //12;
            else
                f->tns_maxorder_long = 20;//12; //20;
        }
        f->tns_maxorder_short = 7; //TNS_MAX_ORDER_SHORT;
        break;
    }
    f->tns_minband_long = tns_minband_long[sr_index];
    f->tns_minband_short= tns_minband_short[sr_index];

    f->h_lpc_short = fa_lpc_init(f->tns_maxorder_short);
    f->h_lpc_long  = fa_lpc_init(f->tns_maxorder_long);

    f->tns_gain_thr = DEF_TNS_GAIN_THRESH;

    return (uintptr_t)f;
}

void fa_tns_uninit(uintptr_t handle)
{
    tns_info_t *f = (tns_info_t *)handle;

    if (f) {
        fa_lpc_uninit(f->h_lpc_long);
        fa_lpc_uninit(f->h_lpc_short);
        free(f);
        f = NULL;
    }

}

static int truncate_cof(int order, float threshold, float * kcof)
{
    int i;

    for (i = order; i >= 0; i--) {
        /*printf("kcof[%d]=%f\n", i, kcof[i]);*/
        kcof[i] = (fabs(kcof[i])>threshold) ? kcof[i] : 0.0;
        if (kcof[i]!=0.0) return i;
    }

    return 0;
}

static void quant_reflection_cof(int order, int cof_res, float *kcof, int *index)
{
    float iqfac, iqfac_m;
    int i;

    iqfac = ((1<<(cof_res-1))-0.5)/(M_PI/2);
    iqfac_m = ((1<<(cof_res-1))+0.5)/(M_PI/2);

    /* Quantize and inverse quantize */
    for (i = 1;i <= order; i++) {
#if 0
        index[i] = (int)(0.5+(asin(kcof[i])*((kcof[i]>=0)?iqfac:iqfac_m)));
#else 
        float tmp;
        int   tmp_int;
        tmp = ((asin(kcof[i])*((kcof[i]>=0)?iqfac:iqfac_m)));
        tmp_int = (int)(floor(tmp));
        if ((tmp-tmp_int) > 0.5) 
            index[i] = tmp_int + 1;
        else 
            index[i] = tmp_int;
#endif
        kcof[i]  = sin((float)index[i]/((index[i]>=0)?iqfac:iqfac_m));
    }
}


static void kcof2acof(int order, float * kcof, float * acof)
{
    float atmp[TNS_MAX_ORDER+2];
    int   i,p;
#if 0
    acof[0] = 1.0;
    atmp[0] = 1.0;
    for (p = 1; p <= order; p++) {
        acof[p] = 0.0;
        for (i = 1; i <= p; i++) {
            atmp[i] = acof[i] + kcof[p]*acof[p-i];
        }
        for (i = 1; i <= p; i++) {
            acof[i]=atmp[i];
        }
    }
#else 
    acof[0] = 1.0;
    atmp[0] = 1.0;
    for (p = 1; p <= order; p++) {
        acof[p] = 0.0;
        for (i = 1; i < p; i++) {
            atmp[i] = acof[i] + kcof[p-1]*acof[p-i];
        }
        for (i = 1; i < p; i++) {
            acof[i]=atmp[i];
        }
        acof[p] = kcof[p-1];
    }

#endif
}


static void tns_ma_filter(float *spec, int length, tns_flt_t *flt)
{
    int i, j, k;
    int order = flt->order;
    float *acof = flt->acof;
    float tmp[1024];

    memset(tmp, 0, 1024*sizeof(float));

    k = 0;
    if (flt->direction) {
        tmp[length-1] = spec[length-1];
        for (i = length - 2; i > (length-1-order); i--) {
            tmp[i] = spec[i];
            k++;
            for (j = 1; j <= k; j++)
                spec[i] += tmp[i+j] * acof[j];
        }

        for (i = length-1-order; i >= 0; i--) {
            tmp[i] = spec[i];
            for (j = 1; j <= order; j++) 
                spec[i] += tmp[i+j] * acof[j];
        }
    } else {
        tmp[0] = spec[0];
        for (i = 1; i < order; i++) {
            tmp[i] = spec[i];
            for (j = 1; j <= i; j++)
                spec[i] += tmp[i-j] * acof[j];
                /*spec[i] += tmp[i-j] * acof[j] * 0.3;*/
        }

        for (i = order; i < length; i++) {
            tmp[i] = spec[i];
            for (j = 1; j < order; j++)
                spec[i] += tmp[i-j] * acof[j];
                /*spec[i] += tmp[i-j] * acof[j] * 0.3;*/
        }
    }

}



void fa_tns_encode_frame(aacenc_ctx_t *f)
{
    tns_info_t *s = (tns_info_t *)f->h_tns;
    int num_windows;
    int window_len;

    int start, end, band_len;
    /*int swb_num;*/
    int *swb_low;
    /*int *swb_high;*/
    int max_sfb;
    float *mdct_line;

    uintptr_t h_lpc;
    int order;
    float cof_res;

    int w;

    int direction;

    fa_mdctquant_t *fs = (fa_mdctquant_t *)(f->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(f->h_mdctq_long);

    mdct_line = f->mdct_line;

    /*initial the band parameters*/
    if (f->block_type == ONLY_SHORT_BLOCK) {
/*
        s->tns_data_present = 0;

        return ;
*/
        num_windows = 8;
        window_len  = 128;

        /*swb_num  = fs->sfb_num;*/
        swb_low  = fs->swb_low;
        /*swb_high = fs->swb_high;*/
        max_sfb  = f->sfb_num_short;

        start = s->tns_minband_short;
        end   = max_sfb;
        order = s->tns_maxorder_short;
        start = FA_MIN(start, s->tns_maxband_short);
        end   = FA_MAX(end  , s->tns_maxband_short);
        h_lpc = s->h_lpc_short;
        cof_res = 3;
    } else {
        num_windows = 1;
        window_len  = 1024;

        /*swb_num  = fl->sfb_num;*/
        swb_low  = fl->swb_low;
        /*swb_high = fl->swb_high;*/
        max_sfb  = f->sfb_num_long;

        start = s->tns_minband_long;
        end   = max_sfb;
        /*band_len = end -start;*/
        order = s->tns_maxorder_long;
        start = FA_MIN(start, s->tns_maxband_long);
        end   = FA_MAX(end  , s->tns_maxband_long);
        h_lpc = s->h_lpc_long;
        cof_res = 4;
    }

    start = FA_MIN(start, max_sfb);
    end   = FA_MIN(end  , max_sfb);
    start = FA_MAX(start, 0);
    end   = FA_MAX(end  , 0);

    band_len = end -start;

    s->tns_data_present = 0;

    direction = 0;
    /*if (f->block_type == LONG_STOP_BLOCK)*/
        /*direction = 1;*/


    for (w = 0; w < num_windows; w++) {
        tns_win_t * tns_win = &(s->tns_win[w]);
        tns_flt_t * tns_flt = tns_win->tns_flt;

        float *kcof = tns_flt->kcof;
        float *acof = tns_flt->acof;
        float err;
        float gain;

        int   mdct_line_index;
        int   mdct_line_len;

/*#define DEBUG_ACOF*/
#ifdef DEBUG_ACOF
        float atmp[15];
        int   ordertmp;
#endif

        tns_win->num_flt = 0;
        tns_win->coef_resolution = cof_res; //DEF_TNS_COEFF_RES;
        mdct_line_index = w*window_len + swb_low[start];
        mdct_line_len   = swb_low[end] - swb_low[start];
/*
        if (mdct_line_len < 0)
            printf("tns error happen\n");
*/
        gain = fa_lpc(h_lpc, &(mdct_line[mdct_line_index]), mdct_line_len, acof, kcof, &err);
        /*printf("w=%d, tns gain=%f\n", w, gain);*/

        /*if (gain > DEF_TNS_GAIN_THRESH) {*/
        if (gain > s->tns_gain_thr) {
            int real_order;
            
            /*printf("\ngain=%f\n", gain);*/

            tns_win->num_flt++;
            s->tns_data_present = 1;
            /*tns_flt->direction  = 0;*/
            tns_flt->direction  = direction;
            tns_flt->coef_compress = 0;
            tns_flt->length = band_len;

#ifdef DEBUG_ACOF
            ordertmp = order;
            memset(atmp, 0, sizeof(float)*15);
            kcof2acof(ordertmp, kcof, atmp);
#endif
            /*quant_reflection_cof(order, DEF_TNS_COEFF_RES, kcof, tns_flt->index);*/
            quant_reflection_cof(order, cof_res, kcof, tns_flt->index);
            real_order = truncate_cof(order, DEF_TNS_COEFF_THRESH, kcof);
            tns_flt->order = real_order;
            kcof2acof(real_order, kcof, acof);
            tns_ma_filter(&(mdct_line[mdct_line_index]), mdct_line_len, tns_flt);
        }

    }

}


int fa_tnssync(fa_aacenc_ctx_t *f)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;
    int tns_active;
    int block_type;

    chn_num = f->cfg.chn_num;

    i = 0;
    chn = 1;

    while (i < chn_num) {
        s = &(f->ctx[i]);

        tns_active = 0;
        if (s->chn_info.cpe == 1) {
            tns_info_t *tns_sl, *tns_sr;

            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            tns_sl = (tns_info_t *)sl->h_tns;
            tns_sr = (tns_info_t *)sr->h_tns;

            /*if (1==sl->tns_active && 1==sr->tns_active)*/
                /*tns_active = 1;*/
            tns_active = sl->tns_active & sr->tns_active;

            sl->tns_active = sr->tns_active = tns_active;
            /*printf("sl_tnsa=%d, sr_tnsa=%d\n", sl->tns_active, sr->tns_active);*/

            block_type = sl->block_type;
            if (block_type == LONG_START_BLOCK || block_type == LONG_STOP_BLOCK)
                tns_sl->tns_gain_thr = tns_sr->tns_gain_thr = 60;
                /*tns_sl->tns_gain_thr = tns_sr->tns_gain_thr = 2;*/
            else 
                tns_sl->tns_gain_thr = tns_sr->tns_gain_thr = 50;
                /*tns_sl->tns_gain_thr = tns_sr->tns_gain_thr = 2;*/
           
        } else {
            tns_info_t *tns_s = (tns_info_t *)s->h_tns;
            chn = 1;

            block_type = s->block_type;
            if (block_type == LONG_START_BLOCK || block_type == LONG_STOP_BLOCK)
                tns_s->tns_gain_thr = 60;
            else 
                tns_s->tns_gain_thr = 50;
        }

        i += chn;
    } 

    return 0;

}





