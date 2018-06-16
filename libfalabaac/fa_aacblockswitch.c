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


  filename: fa_aacblockswitch.c 
  version : v1.0.0
  time    : 2012/10/27 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "fa_aacblockswitch.h"
#include "fa_aacfilterbank.h"
#include "fa_aacstream.h"
#include "fa_fir.h"

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef FA_ABS 
#define FA_ABS(a)    ( (a) > 0 ? (a) : (-(a)) )
#endif


/*---------------------------------- psy blockswitch --------------------------------------------------*/
#define SWITCH_PE  400 //2500 //1000 //1800//1800 //300// 1800

static void blockswitch_pe(float pe, int prev_block_type, int *cur_block_type, uintptr_t h_aacpsy)
{
    int prev_coding_block_type;
    int cur_coding_block_type;

    /*get prev coding block type*/

    if (prev_block_type == ONLY_SHORT_BLOCK)
        prev_coding_block_type = SHORT_CODING_BLOCK;
    else 
        prev_coding_block_type = LONG_CODING_BLOCK;

    /*use pe to decide current coding block type*/
    if (pe > SWITCH_PE) 
        cur_coding_block_type = SHORT_CODING_BLOCK;
    else 
        cur_coding_block_type = LONG_CODING_BLOCK;
/*
    if (cur_coding_block_type != prev_coding_block_type)
        reset_psy_previnfo(h_aacpsy);
*/

    if (cur_coding_block_type == LONG_CODING_BLOCK && prev_coding_block_type == SHORT_CODING_BLOCK)
        update_psy_short2long_previnfo(h_aacpsy);
    if (cur_coding_block_type == SHORT_CODING_BLOCK && prev_coding_block_type == LONG_CODING_BLOCK)
        update_psy_long2short_previnfo(h_aacpsy);

    /*use prev coding block type and current coding block type to decide current block type*/
#if 1 
    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            *cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            *cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            *cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            *cur_block_type = LONG_STOP_BLOCK;
    }
#else 

    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            *cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            *cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            *cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK) // || prev_block_type == ONLY_SHORT_BLOCK)
            *cur_block_type = ONLY_SHORT_BLOCK; //LONG_STOP_BLOCK;
        if (prev_block_type == ONLY_SHORT_BLOCK)
            *cur_block_type = LONG_STOP_BLOCK;
    }

#endif

}


/*this function used in aac encode*/
static int aac_blockswitch_psy(int block_type, float pe, uintptr_t h_aacpsy)
{
    int prev_block_type;
    int cur_block_type;

    prev_block_type = block_type;
    blockswitch_pe(pe, prev_block_type, &cur_block_type, h_aacpsy);

    return cur_block_type;
}

int fa_blockswitch_psy(aacenc_ctx_t *s)
{
    if (s->psy_enable) {
        s->block_type = aac_blockswitch_psy(s->block_type, s->pe, s->h_aacpsy);
        s->bits_alloc = calculate_bit_allocation(s->pe, s->block_type);
        s->bits_more  = s->bits_alloc + 10;//100;
    } else {
        s->block_type = ONLY_LONG_BLOCK;
        s->bits_alloc = s->bits_average;
        s->bits_more  = s->bits_alloc;// - 10;//100;
    }

    return s->block_type;
}


/*---------------------------------- time var fast blockswitch --------------------------------------------------*/
static float frame_var_max(float *x, int len)
{
    int   k, i;
    int   hop;
    float *xp;

    float sum, avg;
    float diff;
    float var;
    float var_max;
    int level;
    int bands;

    level = 1;//4;
    bands = (1<<level);

    hop = len >> level;
    var_max = 0.;

    for (k = 0; k < bands; k++) {
        xp  =  x + k*hop;
        sum =  0.;
        avg =  0.;
        var =  0.;

        for (i = 0; i < hop; i++) {
            float tmp;
            /*tmp = fabs(xp[i]/32768.);*/
            tmp = FA_ABS(xp[i]/32768.);
            sum += tmp;
        }
        avg = sum / hop;

        for (i = 0; i < hop; i++) {
            float tmp;
            /*tmp  = fabs(xp[i]/32768.);*/
            tmp  = FA_ABS(xp[i]/32768.);
            diff =  tmp - avg;
            var  += diff  * diff; 
        }

        var /= hop;

        var_max = FA_MAX(var_max, var);
    }

    return var_max;
}


#define SWITCH_E_BASE  1 //(32768*32768)
#define SWITCH_E   (0.03*SWITCH_E_BASE)
#define SWITCH_E1  (0.075*SWITCH_E_BASE)
#define SWITCH_E2  (0.003*SWITCH_E_BASE)

int fa_blockswitch_var(aacenc_ctx_t *s)
{
    float x[2*AAC_FRAME_LEN];
    float cur_var_max;
    float var_diff;

    int   prev_block_type;
    int   prev_coding_block_type;
    int   cur_coding_block_type;

    int   cur_block_type;

    fa_aacfilterbank_get_xbuf(s->h_aac_analysis, x);
    cur_var_max = frame_var_max(x, 2*AAC_FRAME_LEN);
    var_diff    = FA_ABS(cur_var_max - s->var_max_prev);
    /*var_diff    = fabsf(cur_var_max - s->var_max_prev);*/
    s->var_max_prev = cur_var_max;

    prev_block_type = s->block_type;
    /*get prev coding block type*/
    if (prev_block_type == ONLY_SHORT_BLOCK)
        prev_coding_block_type = SHORT_CODING_BLOCK;
    else 
        prev_coding_block_type = LONG_CODING_BLOCK;

    if (cur_var_max < SWITCH_E1)
        cur_coding_block_type = LONG_CODING_BLOCK;
    else {
        if (prev_coding_block_type == LONG_CODING_BLOCK) {
            if (var_diff > SWITCH_E)
                cur_coding_block_type = SHORT_CODING_BLOCK;
            else 
                cur_coding_block_type = LONG_CODING_BLOCK;
        } else {
            if (var_diff > SWITCH_E2)
                cur_coding_block_type = SHORT_CODING_BLOCK;
            else 
                cur_coding_block_type = LONG_CODING_BLOCK;
        }
    }
 
    /*use prev coding block type and current coding block type to decide current block type*/
#if 0 
    /*test switch */
    if (prev_block_type == ONLY_LONG_BLOCK)
        cur_block_type = LONG_START_BLOCK;
    if (prev_block_type == LONG_START_BLOCK)
        cur_block_type = ONLY_SHORT_BLOCK;
    if (prev_block_type == ONLY_SHORT_BLOCK)
        cur_block_type = LONG_STOP_BLOCK;
    if (prev_block_type == LONG_STOP_BLOCK)
        cur_block_type = ONLY_LONG_BLOCK;
#else
    #if 0
    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = LONG_STOP_BLOCK;
    }
    #else 

    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK) // || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK; //LONG_STOP_BLOCK;
        if (prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = LONG_STOP_BLOCK;
    }


    #endif
#endif

    s->block_type = cur_block_type;

    if (s->psy_enable) {
        s->bits_alloc = calculate_bit_allocation(s->pe, s->block_type);
        /*s->bits_alloc = s->bits_average;*/
        s->bits_more  = s->bits_alloc+10;// - 10;//100;
    } else {
        s->bits_alloc = s->bits_average;
        s->bits_more  = s->bits_alloc;// - 10;//100;
    }

    return cur_block_type;
}


#define WINCNT  4
/*#define WINCNT  8 */

typedef struct _fa_blockctrl_t {
    uintptr_t  h_flt_fir;

    int block_len;  // analysis block length
    float *x;       // block sample 2048
    float *x_flt;   // filter by hp_flt, high frequence sample

    // 1 means attack, 0 means no attack
    int attack_flag;           
    int lastattack_flag;
    int attack_index;
    int lastattack_index;

    float max_win_enrg;         // max energy for attack when detected, and used in sync short block group info

    float win_enrg[2][WINCNT];
    float win_hfenrg[2][WINCNT];
    float win_accenrg;          // smooth energy, accumulate by frame by frame

} fa_blockctrl_t;

uintptr_t fa_blockswitch_init(int block_len)
{
    fa_blockctrl_t *f = (fa_blockctrl_t *)malloc(sizeof(fa_blockctrl_t));
    memset(f, 0, sizeof(fa_blockctrl_t));

    /*use 3 taps, should be smoothly filter the low frequence*/
    /*f->h_flt_fir    = fa_fir_filter_hpf_init(block_len, 3, 0.4, KAISER);*/
    f->h_flt_fir    = fa_fir_filter_hpf_init(block_len, 3, 0.45, KAISER);
    f->block_len    = block_len;

    f->x = (float *)malloc(block_len * sizeof(float));
    memset(f->x, 0, block_len * sizeof(float));
    f->x_flt = (float *)malloc(block_len * sizeof(float));
    memset(f->x_flt, 0, block_len * sizeof(float));

    return (uintptr_t)f;
}

void fa_blockswitch_uninit(uintptr_t handle)
{
    fa_blockctrl_t *f = (fa_blockctrl_t *)handle;

    if (f) {
        if (f->h_flt_fir) {
            fa_fir_filter_uninit(f->h_flt_fir);
            f->h_flt_fir = 0; //NULL;
        }

        if (f->x) {
            free(f->x);
            f->x = NULL;
        }

        if (f->x_flt) {
            free(f->x_flt);
            f->x_flt = NULL;
        }

        free(f);
        f = NULL;
    }

}

static void calculate_win_enrg(fa_blockctrl_t *f)
{
    int win;
    int i;
    int win_len;

    float win_enrg_tmp;
    float win_hfenrg_tmp;
    float x_tmp, x_flt_tmp;

    win_len = f->block_len / WINCNT;

    fa_fir_filter(f->h_flt_fir   , f->x, f->x_flt   , f->block_len);

    for (win = 0; win < WINCNT; win++) {
        win_enrg_tmp   = 0.0;
        win_hfenrg_tmp = 0.0;

        for (i = 0; i < win_len; i++) {
            x_tmp     = f->x    [win*win_len + i];
            x_flt_tmp = f->x_flt[win*win_len + i];

            win_enrg_tmp       += x_tmp     * x_tmp;
            win_hfenrg_tmp     += x_flt_tmp * x_flt_tmp;
        }

        f->win_enrg[1][win]      = win_enrg_tmp;
        f->win_hfenrg[1][win]    = win_hfenrg_tmp;
    }

}

                            //lastattack-attack-blocktype
static const int win_sequence[2][2][4] =
{
   /*  ONLY_LONG_BLOCK    LONG_START_BLOCK   ONLY_SHORT_BLOCK   LONG_STOP_BLOCK   */  
   /*last no attack*/
   { 
       /*no attack*/
       {ONLY_LONG_BLOCK,   ONLY_SHORT_BLOCK,  LONG_STOP_BLOCK,   ONLY_LONG_BLOCK,  },   
       /*attack*/
       {LONG_START_BLOCK,  ONLY_SHORT_BLOCK,  ONLY_SHORT_BLOCK,  LONG_START_BLOCK, } 
   }, 
   /*last attack*/
   { 
       /*no attack*/
       {ONLY_LONG_BLOCK,   ONLY_SHORT_BLOCK,  ONLY_SHORT_BLOCK,  ONLY_LONG_BLOCK,  },  
       /*attack*/
       {LONG_START_BLOCK,  ONLY_SHORT_BLOCK,  ONLY_SHORT_BLOCK,  LONG_START_BLOCK, } 
   }  
};


static int select_block(int prev_block_type, int attack_flag)
{
    int prev_coding_block_type;
    int cur_coding_block_type;
    int cur_block_type;

    if (attack_flag)
        cur_coding_block_type = SHORT_CODING_BLOCK;
    else 
        cur_coding_block_type = LONG_CODING_BLOCK;

    /*get prev coding block type*/
    if (prev_block_type == ONLY_SHORT_BLOCK)
        prev_coding_block_type = SHORT_CODING_BLOCK;
    else 
        prev_coding_block_type = LONG_CODING_BLOCK;

#if 0 
    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = LONG_STOP_BLOCK;
    }

#else 

    if (cur_coding_block_type == SHORT_CODING_BLOCK) {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = LONG_START_BLOCK;
        if (prev_block_type == LONG_START_BLOCK || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK;
    } else {
        if (prev_block_type == ONLY_LONG_BLOCK || prev_block_type == LONG_STOP_BLOCK)
            cur_block_type = ONLY_LONG_BLOCK;
        if (prev_block_type == LONG_START_BLOCK) // || prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = ONLY_SHORT_BLOCK; //LONG_STOP_BLOCK;
        if (prev_block_type == ONLY_SHORT_BLOCK)
            cur_block_type = LONG_STOP_BLOCK;
    }

#endif

    return cur_block_type;
}

int fa_blockswitch_robust(aacenc_ctx_t *s, float *sample_buf)
{
    int i;
    fa_aacfilterbank_t *fb = (fa_aacfilterbank_t *)(s->h_aac_analysis);
    fa_blockctrl_t *f = (fa_blockctrl_t *)(s->h_blockctrl);
    /*float win_enrg_max;*/
    float win_enrg_prev;

    float frac; 
    float ratio; 

    float max_attack;
    float cur_attack;

    /*save current attack info to last attack info*/
    f->lastattack_flag  = f->attack_flag;
    f->lastattack_index = f->attack_index;

    /*save current analysis win energy to last analysis win energy*/
    for (i = 0; i < WINCNT; i++) {
        f->win_enrg[0][i]   = f->win_enrg[1][i];
        f->win_hfenrg[0][i] = f->win_hfenrg[1][i];
    }

#if 0
    for (i = 0; i < 1024; i++) {
        f->x[i] = sample_buf[i];
    }
#else 
    for (i = 0; i < 1024; i++) {
        f->x[i]      = fb->x_buf[i+1024];
        f->x[i+1024] = sample_buf[i];
    }

#endif

    calculate_win_enrg(f);

    f->attack_flag = 0;
    f->attack_index = 0;

    /*win_enrg_max = 0.0;*/
    win_enrg_prev = f->win_hfenrg[0][WINCNT-1];

    /*frac = 0.42;*/
    /*ratio = 0.044;*/
    /*frac = 0.32;*/
    /*ratio = 0.15;*/
    /*frac = 0.42;*/
    frac = 0.42;
    ratio = 0.044;

    max_attack = 0.;
    cur_attack = 0.;
    for (i = 0; i < WINCNT; i++) {
        /*the accenrg is the smooth energy threshold*/
        f->win_accenrg = (1-frac)*f->win_accenrg + frac*win_enrg_prev;

        if ((f->win_hfenrg[1][i]*ratio) > f->win_accenrg) {
            f->attack_flag  = 1;
            cur_attack = f->win_hfenrg[1][i]*ratio;

            f->attack_index = i;

            if (cur_attack > max_attack) {
                f->attack_index = i;
                max_attack = cur_attack;
            }
        }

        win_enrg_prev = f->win_hfenrg[1][i];
        /*win_enrg_max  = FA_MAX(win_enrg_max, win_enrg_prev);*/
    }

    /*check if last prev attack spread to this frame*/
    if (f->lastattack_flag && !f->attack_flag) {
        if  (((f->win_hfenrg[0][WINCNT-1] > f->win_hfenrg[1][0]) &&
             (f->lastattack_index == WINCNT-1))
            )  {
            f->attack_flag  = 1;
            f->attack_index = 0;
        }
    }
    
    /*s->block_type = select_block(s->block_type, f->attack_flag);*/
    s->block_type = win_sequence[f->lastattack_flag][f->attack_flag][s->block_type];

    if (f->attack_flag)
        f->max_win_enrg = f->win_enrg[0][f->attack_index];
    else
        f->max_win_enrg = 0.0;

    return s->block_type;
}


static const int block_sync_tab[4][4] =
{
  /*                      ONLY_LONG_BLOCK   LONG_START_BLOCK  ONLY_SHORT_BLOCK  LONG_STOP_BLOCK   */
  /* ONLY_LONG_BLOCK  */ {ONLY_LONG_BLOCK,  LONG_START_BLOCK, ONLY_SHORT_BLOCK, LONG_STOP_BLOCK  },
  /* LONG_START_BLOCK */ {LONG_START_BLOCK, LONG_START_BLOCK, ONLY_SHORT_BLOCK, ONLY_SHORT_BLOCK },
  /* ONLY_SHORT_BLOCK */ {ONLY_SHORT_BLOCK, ONLY_SHORT_BLOCK, ONLY_SHORT_BLOCK, ONLY_SHORT_BLOCK },
  /* LONG_STOP_BLOCK  */ {LONG_STOP_BLOCK,  ONLY_SHORT_BLOCK, ONLY_SHORT_BLOCK, LONG_STOP_BLOCK  },
};

#define MAX_GROUP_CNT  3
/*#define MAX_GROUP_CNT  4*/
/*#define MAX_GROUP_CNT 5*/
static const int group_tab[WINCNT][MAX_GROUP_CNT] =
{
     /*{1,  3,  3,  1},*/
     /*{1,  1,  3,  3},*/
     /*{2,  1,  3,  2},*/
     /*{3,  1,  3,  1},*/
     /*{3,  1,  1,  3},*/
     /*{3,  2,  1,  2},*/
     /*{3,  3,  1,  1},*/
     /*{3,  3,  1,  1}*/

     /*{1,  2,  2,  3},*/
     /*{1,  1,  3,  3},*/
     /*{2,  1,  2,  3},*/
     /*{2,  2,  1,  3},*/
     /*{2,  3,  1,  2},*/
     /*{3,  2,  1,  2},*/
     /*{3,  3,  1,  1},*/
     /*{3,  2,  2,  1}*/

     /*{1,  3,  4},*/
     /*{2,  3,  3},*/
     /*{2,  1,  5},*/
     /*{3,  2,  3},*/
     /*{3,  2,  3},*/
     /*{3,  3,  2},*/
     /*{3,  3,  2},*/
     /*{4,  3,  1}*/

     /*{2,  3,  3},*/
     /*{2,  3,  3},*/
     /*{2,  2,  4},*/
     /*{3,  2,  3},*/
     /*{3,  2,  3},*/
     /*{4,  2,  2},*/
     /*{3,  3,  2},*/
     /*{3,  3,  2}*/

     {2,  3,  3},
     {2,  2,  4},
     {4,  2,  2},
     {3,  3,  2},

     /*{1,  1,  2,  3,  1},*/
     /*{1,  1,  1,  2,  3},*/
     /*{2,  1,  1,  2,  2},*/
     /*{3,  1,  1,  1,  2},*/
     /*{2,  1,  1,  1,  3},*/
     /*{2,  2,  1,  1,  2},*/
     /*{3,  2,  1,  1,  1},*/
     /*{1,  3,  2,  1,  1}*/

     /*{1,  1,  2,  2,  2},*/
     /*{1,  1,  2,  2,  2},*/
     /*{2,  2,  2,  1,  1},*/
     /*{1,  2,  2,  2,  1},*/
     /*{1,  2,  2,  2,  1},*/
     /*{1,  1,  2,  2,  2},*/
     /*{2,  2,  2,  1,  1},*/
     /*{2,  2,  2,  1,  1}*/

     /*{1,  1,  1,  2,  3},*/
     /*{1,  1,  1,  2,  3},*/
     /*{1,  1,  1,  3,  2},*/
     /*{1,  1,  1,  3,  2},*/
     /*{2,  3,  1,  1,  1},*/
     /*{2,  3,  1,  1,  1},*/
     /*{3,  2,  1,  1,  1},*/
     /*{3,  2,  1,  1,  1}*/
};

int fa_blocksync(fa_aacenc_ctx_t *f)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;
    fa_blockctrl_t *bcl, *bcr; // = (fa_blockctrl_t *)(s->h_blockctrl);
    int block_type;
    int k;

    chn_num = f->cfg.chn_num;

    i = 0;
    chn = 1;

    while (i < chn_num) {
        block_type = ONLY_LONG_BLOCK;
        s = &(f->ctx[i]);

        if (s->chn_info.cpe == 1) {
            int last_block_type;

            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            bcl = (fa_blockctrl_t *)(sl->h_blockctrl);
            bcr = (fa_blockctrl_t *)(sr->h_blockctrl);

            last_block_type = sl->block_type;

            block_type = block_sync_tab[block_type][sl->block_type];
            block_type = block_sync_tab[block_type][sr->block_type];

            sl->block_type = block_type;
            sr->block_type = block_type;

#if 0 
            if (block_type != 0)
                printf("i=%d, block_type=%d\n", i+1, s->block_type);
#endif
            
            if (block_type == ONLY_SHORT_BLOCK) {
                sl->num_window_groups = MAX_GROUP_CNT;
                sr->num_window_groups = MAX_GROUP_CNT;
#if  1 
                sl->window_group_length[0] = group_tab[bcl->attack_index][0];
                sl->window_group_length[1] = group_tab[bcl->attack_index][1];
                sl->window_group_length[2] = group_tab[bcl->attack_index][2];
                sl->window_group_length[3] = 0;//group_tab[bcl->attack_index][3];
                sl->window_group_length[4] = 0;
                sl->window_group_length[5] = 0;
                sl->window_group_length[6] = 0;
                sl->window_group_length[7] = 0;
                sr->window_group_length[0] = group_tab[bcr->attack_index][0];
                sr->window_group_length[1] = group_tab[bcr->attack_index][1];
                sr->window_group_length[2] = group_tab[bcr->attack_index][2];
                sr->window_group_length[3] = 0;//group_tab[bcr->attack_index][3];
                sr->window_group_length[4] = 0;
                sr->window_group_length[5] = 0;
                sr->window_group_length[6] = 0;
                sr->window_group_length[7] = 0;
#else 
                sl->window_group_length[0] = group_tab[bcl->attack_index][0];
                sl->window_group_length[1] = group_tab[bcl->attack_index][1];
                sl->window_group_length[2] = group_tab[bcl->attack_index][2];
                sl->window_group_length[3] = group_tab[bcl->attack_index][3];
                sl->window_group_length[4] = group_tab[bcl->attack_index][4];
                sl->window_group_length[5] = 0;
                sl->window_group_length[6] = 0;
                sl->window_group_length[7] = 0;
                sr->window_group_length[0] = group_tab[bcr->attack_index][0];
                sr->window_group_length[1] = group_tab[bcr->attack_index][1];
                sr->window_group_length[2] = group_tab[bcr->attack_index][2];
                sr->window_group_length[3] = group_tab[bcr->attack_index][3];
                sr->window_group_length[4] = group_tab[bcr->attack_index][4];
                sr->window_group_length[5] = 0;
                sr->window_group_length[6] = 0;
                sr->window_group_length[7] = 0;

#endif

                /*printf("lwe=%f, rwe=%f\n", bcl->max_win_enrg, bcr->max_win_enrg);*/
                if (bcl->max_win_enrg > bcr->max_win_enrg) {
                    for (k = 0; k < MAX_GROUP_CNT; k++)
                        sr->window_group_length[k] = sl->window_group_length[k];
                } else {
                    for (k = 0; k < MAX_GROUP_CNT; k++)
                        sl->window_group_length[k] = sr->window_group_length[k];
                }
            } else {
                sl->num_window_groups = 1;
                sl->window_group_length[0] = 1;
                sr->num_window_groups = 1;
                sr->window_group_length[0] = 1;
            }
/*
            {
                int kk;
                if (bcl->attack_index != 0 || bcr->attack_index != 0) {
                    for (kk = 0; kk < sl->num_window_groups; kk++)
                        printf("-------lattack_index=%d, rattack_index=%d, g[%d]=%d\n",
                                bcl->attack_index, bcr->attack_index, kk, sl->window_group_length[kk]);
                }
            }
*/
        } else {
            chn = 1;
            s->num_window_groups = 1;
            s->window_group_length[0] = 1;
        }

        i += chn;
    } 

    return 0;
}
