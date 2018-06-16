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


  filename: fa_mdctquant.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "fa_mdctquant.h"
#include "fa_iqtab.h"
#include "fa_fastmath.h"
#include "fa_huffman.h"
/*#include "fa_timeprofile.h"*/


#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef FA_ABS
#define FA_ABS(a) ( (a) > 0 ? (a) : (-(a)))
#endif

#define MAX_QUANT             8191
#define MAGIC_NUMBER          0.4054

#define COF_SCALE_NUM         256

/*ROM: table will be used*/
static float rom_cof_scale[2*COF_SCALE_NUM];
static float rom_cof_quant[2*COF_SCALE_NUM];
static float rom_inv_cof[2*COF_SCALE_NUM];
void fa_mdctquant_rom_init()
{
    int i;

    for (i = 0; i < 2*COF_SCALE_NUM; i++) {
        rom_cof_scale[i] = pow(2, (3./16.) * (i-255)); 
        rom_cof_quant[i] = 1./rom_cof_scale[i];
    }

    for (i = 0; i < 2*COF_SCALE_NUM; i++) {
        rom_inv_cof[i] = pow(2, 0.25*(i-255)); 
    }

}

uintptr_t fa_mdctquant_init(int mdct_line_num, int sfb_num, int *swb_low, int block_type_cof)
{
    int swb;

    fa_mdctquant_t *f = (fa_mdctquant_t *)malloc(sizeof(fa_mdctquant_t));
    
    memset(f, 0, sizeof(fa_mdctquant_t));

    f->block_type_cof = block_type_cof;
    f->mdct_line_num  = mdct_line_num;

    memset(f->mdct_line  , 0, sizeof(float)*NUM_MDCT_LINE_MAX);
    memset(f->xr_pow     , 0, sizeof(float)*NUM_MDCT_LINE_MAX);
    memset(f->mdct_scaled, 0, sizeof(float)*NUM_MDCT_LINE_MAX);
    memset(f->xmin       , 0, sizeof(float)*NUM_WINDOW_GROUPS_MAX*NUM_SFB_MAX*NUM_WINDOWS_MAX);

    f->sfb_num = sfb_num;
    memset(f->swb_low    , 0, sizeof(int)*(FA_SWB_NUM_MAX+1));
    memset(f->swb_high   , 0, sizeof(int)*FA_SWB_NUM_MAX);
    memset(f->sfb_low    , 0, sizeof(int)*(FA_SWB_NUM_MAX+1)*NUM_WINDOW_GROUPS_MAX);
    memset(f->sfb_high   , 0, sizeof(int)*FA_SWB_NUM_MAX*NUM_WINDOW_GROUPS_MAX);

    for (swb = 0; swb < sfb_num; swb++) {
        f->swb_low[swb] = swb_low[swb];
/*
        if (swb == sfb_num - 1)
            f->swb_high[swb] = mdct_line_num-1;
        else 
            f->swb_high[swb] = swb_low[swb+1] - 1;
*/
        f->swb_high[swb] = swb_low[swb+1] - 1;
    }
    f->swb_low[sfb_num] = swb_low[sfb_num];

    return (uintptr_t)f;
}

void      fa_mdctquant_uninit(uintptr_t handle)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    if (f) {
        free(f);
        f = NULL;
    }

}

static void xr_pow34_calculate(float *mdct_line, float mdct_line_num, 
                               float *xr_pow)
{
    int i;
    float tmp;

    for (i = 0; i < mdct_line_num; i++) {
        tmp = FA_ABS(mdct_line[i]); 
#if 0 
        xr_pow[i] = sqrtf(tmp*sqrtf(tmp));
#else 
        xr_pow[i] = FA_SQRTF(tmp*FA_SQRTF(tmp));
#endif

        if (mdct_line[i] < 0)
            xr_pow[i] = -xr_pow[i];
    }
}

float fa_mdctline_getmax(uintptr_t handle)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int   i;
    float max_mdct_line;
    float abs_mdct_line;
    float *mdct_line = f->mdct_line;

    max_mdct_line = 0;

    /*calculate max mdct_line*/
    for (i = 0; i < (f->block_type_cof*f->mdct_line_num); i++) {
        abs_mdct_line = FA_ABS(mdct_line[i]);
        if (abs_mdct_line > max_mdct_line)
            max_mdct_line = abs_mdct_line;
    }

    return max_mdct_line;
}

int fa_get_start_common_scalefac(float max_mdct_line)
{
    int start_common_scalefac;
    float tmp;

    if (max_mdct_line == 0.)
        return 0;

#if 0
    tmp = ceil(16./3 * (log2f((powf(max_mdct_line, 0.75))/MAX_QUANT)));
#else 
    tmp = ceil(16./3 * (FA_LOG2((FA_SQRTF(max_mdct_line*FA_SQRTF(max_mdct_line)))/MAX_QUANT)));
#endif
    start_common_scalefac = (int)tmp;

    start_common_scalefac = FA_MIN(start_common_scalefac, 255);
    start_common_scalefac = FA_MAX(start_common_scalefac, 0);

    return start_common_scalefac;
}


void fa_mdctline_pow34(uintptr_t handle)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    xr_pow34_calculate(f->mdct_line, f->block_type_cof*f->mdct_line_num, 
                       f->xr_pow);

}

void fa_mdctline_scaled(uintptr_t handle,
                        int num_window_groups, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX])
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int i;
    int gr;
    int sfb;
    int sfb_num;
    float *xr_pow;
    float *mdct_scaled;
    float cof_scale;

    sfb_num      = f->sfb_num;
    xr_pow       = f->xr_pow;
    mdct_scaled  = f->mdct_scaled;

    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            /*cof_scale = powf(2, (3./16.) * scalefactor[gr][sfb]);*/
            cof_scale = rom_cof_scale[scalefactor[gr][sfb]+255];

            for (i = f->sfb_low[gr][sfb]; i <= f->sfb_high[gr][sfb]; i++) 
                mdct_scaled[i] = xr_pow[i] * cof_scale;
        }
    }

}

void fa_mdctline_quant(uintptr_t handle, 
                       int common_scalefac, int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int i;
    float *mdct_scaled = f->mdct_scaled;
    float cof_quant;

    /*FA_CLOCK_START(5);*/
    for (i = 0; i < f->block_type_cof*f->mdct_line_num; i++) {
        /*cof_quant = powf(2, (-3./16)*common_scalefac);*/
        cof_quant = rom_cof_quant[common_scalefac+255];
        if (mdct_scaled[i] > 0)
            x_quant[i] = (int)(mdct_scaled[i] * cof_quant + MAGIC_NUMBER);
        else 
            x_quant[i] = -1 * (int)(FA_ABS(mdct_scaled[i]) * cof_quant + MAGIC_NUMBER);

        if (x_quant[i] > 8191) {
            x_quant[i] = 8191;
        } else if(x_quant[i] < -8191) {
            x_quant[i] = -8191;
        }
    }

    /*FA_CLOCK_END(5);*/
    /*FA_CLOCK_COST(5);*/
}





void fa_mdctline_quantdirect(uintptr_t handle, 
                             int common_scalefac,
                             int num_window_groups, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX],
                             int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int i;
    int gr;
    int sfb;
    int sfb_num;
    float *mdct_line;
    float cof_scale;
    float tmp;

    sfb_num    = f->sfb_num;
    mdct_line  = f->mdct_line;

#if  0 
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            cof_scale = pow(2, (1./4.) * (scalefactor[gr][sfb]-common_scalefac));

            for (i = f->sfb_low[gr][sfb]; i <= f->sfb_high[gr][sfb]; i++) {
                tmp = FA_ABS(mdct_line[i]);
                tmp = tmp * cof_scale;
                x_quant[i] = (int)(FA_SQRTF(tmp*FA_SQRTF(tmp)));
                if (mdct_line[i] > 0)
                    x_quant[i] = -x_quant[i];

                if (x_quant[i] > 8191) {
                    x_quant[i] = 8191;
                } else if(x_quant[i] < -8191) {
                    x_quant[i] = -8191;
                }

            }
        }
    }
#else 
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            /*cof_scale = pow(2, (3./16.) * (scalefactor[gr][sfb]-common_scalefac));*/
            /*cof_scale = 1./pow(2, (3./16.) * (common_scalefac - scalefactor[gr][sfb]));*/
            cof_scale = 1./rom_cof_quant[(scalefactor[gr][sfb]-common_scalefac)+255];
            /*cof_scale = rom_cof_quant[(scalefactor[gr][sfb]-common_scalefac)+255];*/
            /*cof_scale = rom_cof_quant[(scalefactor[gr][sfb])+255];*/
            /*cof_scale = 1./pow(2, (3./16.) * (scalefactor[gr][sfb]));*/

            for (i = f->sfb_low[gr][sfb]; i <= f->sfb_high[gr][sfb]; i++) {
                tmp = FA_ABS(f->xr_pow[i]);
                tmp = tmp * cof_scale;
                x_quant[i] = (int)(tmp);
                /*printf("xrpow=%f, sf=%d, tmp=%f,xq=%d\n", f->xr_pow[i], scalefactor[gr][sfb], tmp, x_quant[i]);*/
                /*if (mdct_line[i] > 0)*/
                if (mdct_line[i] < 0)
                    x_quant[i] = -x_quant[i];

                if (x_quant[i] > 8191) {
                    /*printf(">>>>\n");*/
                    x_quant[i] = 8191;
                } else if(x_quant[i] < -8191) {
                    /*printf("<<<<\n");*/
                    x_quant[i] = -8191;
                }

            }
        }
    }
#endif

}



void fa_calculate_quant_noise(uintptr_t handle,
                             int num_window_groups, int *window_group_length,
                             int common_scalefac, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                             int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int i;

    int gr, win;
    int sfb;
    int sfb_num;
    int *swb_low, *swb_high;
    int swb_width;

    float *mdct_line;

    float tmp;

    float inv_x_quant;
    float inv_cof;


    int mdct_line_offset;

    sfb_num  = f->sfb_num;
    swb_low  = f->swb_low;
    swb_high = f->swb_high;

    mdct_line    = f->mdct_line;

    /*calculate scalefactor band error energy*/
    memset(f->error_energy, 0, sizeof(float)*NUM_WINDOW_GROUPS_MAX*NUM_SFB_MAX*NUM_WINDOWS_MAX);

    /*calculate error_energy*/
    mdct_line_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            swb_width = swb_high[sfb] - swb_low[sfb] + 1;
            for (win = 0; win < window_group_length[gr]; win++) {
                int tmp_xq;
                f->error_energy[gr][sfb][win] = 0;
                for (i = 0; i < swb_width; i++) {
                    /*inv_cof = powf(2, 0.25*(common_scalefac - scalefactor[gr][sfb]));*/
                    inv_cof = rom_inv_cof[common_scalefac - scalefactor[gr][sfb]+255];
                    tmp_xq = FA_ABS(x_quant[mdct_line_offset+i]);
                    /*inv_x_quant = powf(tmp_xq, 4./3.) * inv_cof;*/
                    inv_x_quant = (float)(fa_iqtable[tmp_xq] * inv_cof);

                    tmp = FA_ABS(mdct_line[mdct_line_offset+i]) - inv_x_quant;
                    f->error_energy[gr][sfb][win] += tmp*tmp;  
                }
                mdct_line_offset += swb_width;
           }
        }
    }

}

int  fa_fix_quant_noise_single(uintptr_t handle, 
                               int outer_loop_count, int outer_loop_count_max,
                               int num_window_groups, int *window_group_length,
                               int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int gr, win;
    int sfb;
    int sfb_num;

    /*three break condition variable*/
    /*no1*/
    int energy_err_ok_cnt[NUM_WINDOW_GROUPS_MAX];
    int energy_err_ok[NUM_WINDOW_GROUPS_MAX];
    /*no2*/
    int sfb_scale_cnt[NUM_WINDOW_GROUPS_MAX];
    int sfb_allscale[NUM_WINDOW_GROUPS_MAX];
    /*no3*/


    sfb_num  = f->sfb_num;


    memset(energy_err_ok_cnt, 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(energy_err_ok    , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(sfb_scale_cnt    , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(sfb_allscale     , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);

    /*judge if scalefactor increase or not*/
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            for (win = 0; win < window_group_length[gr]; win++) {
                if (f->error_energy[gr][sfb][win] > f->xmin[gr][sfb][win]) {
                    scalefactor[gr][sfb] += 1;
                    scalefactor[gr][sfb] = FA_MIN(scalefactor[gr][sfb], 255);
                    sfb_scale_cnt[gr]++;
                    break;
                } else {
                    energy_err_ok_cnt[gr]++;
                }
            }
        }
    }

    /*every scalefactor band has good energy distorit;*/
    for (gr = 0; gr < num_window_groups; gr++) {
        if (energy_err_ok_cnt[gr] >= sfb_num*window_group_length[gr])
            energy_err_ok[gr] = 1;
        else 
            energy_err_ok[gr] = 0;
#if 1 
        if (sfb_scale_cnt[gr] >= sfb_num) {
            sfb_allscale[gr] = 1;
            /*recover the scalefactor*/
            for (sfb = 0; sfb < sfb_num; sfb++) {
                scalefactor[gr][sfb] -= 1;
            }
        }
        else
            sfb_allscale[gr] = 0;
#endif
    }

    for (gr = 0; gr < num_window_groups; gr++) {
        if ((energy_err_ok[gr] == 0) && (sfb_allscale[gr] == 0)) {
            for (sfb = 1; sfb < sfb_num; sfb++) {
                if (FA_ABS(scalefactor[gr][sfb] - scalefactor[gr][sfb-1]) > 20)
                /*if (FA_ABS(scalefactor[gr][sfb] - scalefactor[gr][sfb-1]) > 40)*/
                    return 1;
                if (outer_loop_count > outer_loop_count_max)
                    return 1;
            }
            return 0;
        }
    }

    return 1;

}


int  fa_fix_quant_noise_couple(uintptr_t handle1, uintptr_t handle2, 
                               int outer_loop_count, int outer_loop_count_max,
                               int num_window_groups, int *window_group_length,
                               int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int scalefactor1[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int *x_quant)
{
    fa_mdctquant_t *f1 = (fa_mdctquant_t *)handle1;
    /*fa_mdctquant_t *f2 = (fa_mdctquant_t *)handle2;*/

    int gr, win;
    int sfb;
    int sfb_num;

    /*three break condition variable*/
    /*no1*/
    int energy_err_ok_cnt[NUM_WINDOW_GROUPS_MAX];
    int energy_err_ok[NUM_WINDOW_GROUPS_MAX];
    /*no2*/
    int sfb_scale_cnt[NUM_WINDOW_GROUPS_MAX];
    int sfb_allscale[NUM_WINDOW_GROUPS_MAX];
    /*no3*/


    sfb_num  = f1->sfb_num;


    memset(energy_err_ok_cnt, 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(energy_err_ok    , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(sfb_scale_cnt    , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);
    memset(sfb_allscale     , 0, sizeof(int)*NUM_WINDOW_GROUPS_MAX);

    /*judge if scalefactor increase or not*/
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            for (win = 0; win < window_group_length[gr]; win++) {
                /*if ((f1->error_energy[gr][sfb][win] > f1->xmin[gr][sfb][win]) ||*/
                   /*(f2->error_energy[gr][sfb][win] > f2->xmin[gr][sfb][win])) {*/
                /*because is common_window, just judge the ms sum channel*/
                if (f1->error_energy[gr][sfb][win] > f1->xmin[gr][sfb][win]) {
                    scalefactor[gr][sfb] += 1;
                    scalefactor1[gr][sfb] += 1;
                    scalefactor[gr][sfb] = FA_MIN(scalefactor[gr][sfb], 255);
                    scalefactor1[gr][sfb] = FA_MIN(scalefactor1[gr][sfb], 255);
                    sfb_scale_cnt[gr]++;
                    break;
                } else {
                    energy_err_ok_cnt[gr]++;
                }
            }
        }
    }

    /*every scalefactor band has good energy distorit;*/
    for (gr = 0; gr < num_window_groups; gr++) {
        if (energy_err_ok_cnt[gr] >= sfb_num*window_group_length[gr])
            energy_err_ok[gr] = 1;
        else 
            energy_err_ok[gr] = 0;

        /*if (sfb_scale_cnt[gr] >= sfb_num) {*/
        if (sfb_scale_cnt[gr] >= sfb_num*window_group_length[gr]) {
            sfb_allscale[gr] = 1;
            /*recover the scalefactor*/
            for (sfb = 0; sfb < sfb_num; sfb++) {
                scalefactor[gr][sfb] -= 1;
                scalefactor1[gr][sfb] -= 1;
            }
        }
        else
            sfb_allscale[gr] = 0;
    }

    for (gr = 0; gr < num_window_groups; gr++) {
        if ((energy_err_ok[gr] == 0) && (sfb_allscale[gr] == 0)) {
            for (sfb = 1; sfb < sfb_num; sfb++) {
                /*if (FA_ABS(scalefactor[gr][sfb] - scalefactor[gr][sfb-1]) > 20)*/
                if (FA_ABS(scalefactor[gr][sfb] - scalefactor[gr][sfb-1]) > 40)
                    return 1;
                if (outer_loop_count > outer_loop_count_max)
                    return 1;
            }

            return 0;
        }
    }

    return 1;

}


int fa_mdctline_get_sfbnum(uintptr_t handle)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    return f->sfb_num;
}


int fa_mdctline_iquantize(uintptr_t handle, 
                          int num_window_groups, int *window_group_length,
                          int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                          int *x_quant)
{
    int i;
    int gr, win;
    int sfb;
    int sfb_num;
    int *swb_low, *swb_high;
    int swb_width;
    float tmp_xq, inv_cof;
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    float *mdct_line = f->mdct_line;
    int mdct_line_offset;
    float inv_x_quant;

    sfb_num = f->sfb_num;
    swb_low = f->swb_low;
    swb_high = f->swb_high;

    mdct_line_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            swb_width = swb_high[sfb] - swb_low[sfb] + 1;
            for (win = 0; win < window_group_length[gr]; win++) {
                for (i = 0; i < swb_width; i++) {
                    /*inv_cof = powf(2, 0.25*(common_scalefac - scalefactor[gr][sfb]));*/
                    inv_cof = pow(2, 0.25*(scalefactor[gr][sfb] - SF_OFFSET));
                    tmp_xq = (float)fabs(x_quant[mdct_line_offset+i]);
                    inv_x_quant = pow(tmp_xq, 4./3.) * inv_cof; 
                    mdct_line[mdct_line_offset+i] = inv_x_quant;
                }
                mdct_line_offset += swb_width;
           }

        }
    }

    return 0;
}

#if  0 

void fa_balance_energe(uintptr_t handle,
                       int num_window_groups, int *window_group_length,
                       int common_scalefac, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                       int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int i;

    int gr, win;
    int sfb;
    int sfb_num;
    int *swb_low, *swb_high;
    int swb_width;

    float *mdct_line;

    float tmp;

    /*const float ifqstep = pow(2.0, 0.25);*/
    /*const float logstep_1 = 1.0 / log2(ifqstep);*/
    const float qstep = 1./0.25;
    float inv_x_quant;
    float inv_cof;

    float en0, enq;
    float en0t, enqt;
    int   shift;

    int mdct_line_offset;

    sfb_num  = f->sfb_num;
    swb_low  = f->swb_low;
    swb_high = f->swb_high;

    mdct_line    = f->mdct_line;

    /*calculate error_energy*/
    mdct_line_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        en0t = 0.0;
        enqt = 0.0;
        for (sfb = 0; sfb < sfb_num; sfb++) {
            swb_width = swb_high[sfb] - swb_low[sfb] + 1;
            en0 = 0.0;
            enq = 0.0;
            for (win = 0; win < window_group_length[gr]; win++) {
                int tmp_xq;
                for (i = 0; i < swb_width; i++) {
                    /*inv_cof = powf(2, 0.25*(common_scalefac - scalefactor[gr][sfb]));*/
                    inv_cof = rom_inv_cof[common_scalefac - scalefactor[gr][sfb]+255];
                    tmp_xq = FA_ABS(x_quant[mdct_line_offset+i]);
                    /*inv_x_quant = powf(tmp_xq, 4./3.) * inv_cof;*/
                    inv_x_quant = (float)(fa_iqtable[tmp_xq] * inv_cof);

                    tmp = FA_ABS(mdct_line[mdct_line_offset+i]) - inv_x_quant;
                    en0 += mdct_line[mdct_line_offset+i] * mdct_line[mdct_line_offset+i];
                    enq += inv_x_quant * inv_x_quant;
                }
                mdct_line_offset += swb_width;
            }
#if 1
            if ((enq == 0.0) || (en0 == 0.0))
                continue;
            /*shift = (int)(log2(sqrt(enq / en0)) * logstep_1 + 1000.5);*/
#if 1 
            shift = (int)(FA_LOG2(FA_SQRTF(enq / en0)) * qstep + 1000.5);
            shift -= 1000;
#else 
            shift = (int)(FA_LOG2(FA_SQRTF(enq / en0)) * qstep);
#endif
/*
            if (shift > 4)
                shift = 4;
            if (shift < -2)
                shift = -2;
              */
            /*printf("shift=%d\n", shift);*/

            shift += scalefactor[gr][sfb];
            scalefactor[gr][sfb] = shift;
#else 
            en0t += en0;
            enqt += enq;
#endif

        }
#if  0 
        shift = (int)(log2(sqrt(enqt / en0t)) * qstep + 1000.5);
        shift -= 1000;
/*
        if (shift > 1)
            shift = 1;
        if (shift < -1)
            shift = -1;
*/
        /*printf("shift=%d\n", shift);*/

        for (sfb = 0; sfb < sfb_num; sfb++) {
            /*shift += scalefactor[gr][sfb];*/
            /*scalefactor[gr][sfb] = shift;*/
            scalefactor[gr][sfb] += shift;
        }
#endif

    }

}


#else 

void fa_balance_energe(uintptr_t handle,
                       int num_window_groups, int *window_group_length,
                       int common_scalefac, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                       int *x_quant)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int i;

    int gr, win;
    int sfb;
    int sfb_num;
    int *swb_low, *swb_high;
    int swb_width;

    float *mdct_line;

    float tmp;

    /*const float ifqstep = pow(2.0, 0.25);*/
    /*const float logstep_1 = 1.0 / log2(ifqstep);*/
    const float qstep = 1./0.25;
    float inv_x_quant;
    float inv_cof;

    float en0, enq;
    float en0t, enqt;
    int   shift;

    int group_offset;
    int index;
    int k;

    int mdct_line_num = f->mdct_line_num;

    sfb_num  = f->sfb_num;
    swb_low  = f->swb_low;
    swb_high = f->swb_high;

    mdct_line    = f->mdct_line;

    /*calculate error_energy*/
    group_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        en0t = 0.0;
        enqt = 0.0;
        k = 0;
        for (sfb = 0; sfb < sfb_num; sfb++) {
            swb_width = swb_high[sfb] - swb_low[sfb] + 1;
            en0 = 0.0;
            enq = 0.0;
            for (win = 0; win < window_group_length[gr]; win++) {
                int tmp_xq;
                for (i = 0; i < swb_width; i++) {
                    index = group_offset + k; 
                    /*inv_cof = powf(2, 0.25*(common_scalefac - scalefactor[gr][sfb]));*/
                    inv_cof = rom_inv_cof[common_scalefac - scalefactor[gr][sfb]+255];
                    tmp_xq = FA_ABS(x_quant[index]);
                    /*inv_x_quant = powf(tmp_xq, 4./3.) * inv_cof;*/
                    inv_x_quant = (float)(fa_iqtable[tmp_xq] * inv_cof);

                    tmp = FA_ABS(mdct_line[index]) - inv_x_quant;
                    en0 += mdct_line[index] * mdct_line[index];
                    enq += inv_x_quant * inv_x_quant;
                    k++;
                }

            }
#if 1
            if ((enq == 0.0) || (en0 == 0.0))
                continue;
            /*shift = (int)(log2(sqrt(enq / en0)) * logstep_1 + 1000.5);*/
#if 1 
            shift = (int)(FA_LOG2(FA_SQRTF(enq / en0)) * qstep + 1000.5);
            shift -= 1000;
#else 
            shift = (int)(FA_LOG2(FA_SQRTF(enq / en0)) * qstep);
#endif
/*
            if (shift > 4)
                shift = 4;
            if (shift < -2)
                shift = -2;
              */
            /*printf("shift=%d\n", shift);*/

            shift += scalefactor[gr][sfb];
            scalefactor[gr][sfb] = shift;
#else 
            en0t += en0;
            enqt += enq;
#endif

        }
#if  0 
        shift = (int)(log2(sqrt(enqt / en0t)) * qstep + 1000.5);
        shift -= 1000;
/*
        if (shift > 1)
            shift = 1;
        if (shift < -1)
            shift = -1;
*/
        /*printf("shift=%d\n", shift);*/

        for (sfb = 0; sfb < sfb_num; sfb++) {
            /*shift += scalefactor[gr][sfb];*/
            /*scalefactor[gr][sfb] = shift;*/
            scalefactor[gr][sfb] += shift;
        }
#endif

        group_offset += mdct_line_num * window_group_length[gr];
    }

}





#endif




void fa_xmin_sfb_arrange(uintptr_t handle, float xmin_swb[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX],
                         int num_window_groups, int *window_group_length)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;
    int sfb_num = f->sfb_num;

    int gr, swb, win;
    int win_offset;

    win_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        for (swb = 0; swb < sfb_num; swb++) {
            for (win = 0; win < window_group_length[gr]; win++) {
                f->xmin[gr][swb][win] = xmin_swb[win_offset+win][swb];
            }
        }
        win_offset += window_group_length[gr];
    }
}

void fa_mdctline_sfb_arrange(uintptr_t handle, float *mdct_line_swb, 
                             int num_window_groups, int *window_group_length)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int mdct_line_num = f->mdct_line_num;
    int sfb_num       = f->sfb_num;
    int *swb_low      = f->swb_low;
    int *swb_high     = f->swb_high;

    float *mdct_line_sfb = f->mdct_line;

    int swb_width;
    int group_offset;
    int gr, swb, win, i, k;
    int sfb;
    int index;

#if 0
    k = 0;
    group_offset = 0;
    /*order rearrage:  swb[gr][win][sfb][k] ---> sfb[gr][sfb][win][k]*/
    for (gr = 0; gr < num_window_groups; gr++) {
        for (swb = 0; swb < sfb_num; swb++) {
            swb_width = swb_high[swb] - swb_low[swb] + 1;
            for (win = 0; win < window_group_length[gr]; win++) {
                for (i = 0; i < swb_width; i++) {
                    index = group_offset + swb_low[swb] + win*mdct_line_num + i;
                    mdct_line_sfb[k++] = mdct_line_swb[index];
                }
            }
        }
        group_offset += mdct_line_num * window_group_length[gr];
    }

#else 
    group_offset = 0;
    /*order rearrage:  swb[gr][win][sfb][k] ---> sfb[gr][sfb][win][k]*/
    for (gr = 0; gr < num_window_groups; gr++) {
        k = 0;
        for (swb = 0; swb < sfb_num; swb++) {
            swb_width = swb_high[swb] - swb_low[swb] + 1;
            for (win = 0; win < window_group_length[gr]; win++) {
                for (i = 0; i < swb_width; i++) {
                    index = group_offset + swb_low[swb] + win*mdct_line_num + i;
                    mdct_line_sfb[group_offset+k] = mdct_line_swb[index];
                    k++;
                }
            }
        }
        group_offset += mdct_line_num * window_group_length[gr];
    }


#endif

    /*calcualte sfb width and sfb_low and sfb_high*/
    group_offset = 0;
    for (gr = 0; gr < num_window_groups; gr++) {
        sfb = 0;
        f->sfb_low[gr][sfb++] = group_offset;
        for (swb = 0; swb < sfb_num; swb++) {
            swb_width           = swb_high[swb]  - swb_low[swb] + 1;
            f->sfb_low[gr][sfb]    = f->sfb_low[gr][sfb-1] + swb_width*window_group_length[gr];
            f->sfb_high[gr][sfb-1] = f->sfb_low[gr][sfb]   - 1;
            sfb++;
        }
        f->sfb_low[gr][sfb_num] = f->sfb_high[gr][sfb_num-1] + 1; 
        group_offset += mdct_line_num * window_group_length[gr];
    }

}


void fa_mdctline_sfb_iarrange(uintptr_t handle, float *mdct_line_swb, int *mdct_line_sign,
                              int num_window_groups, int *window_group_length)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int mdct_line_num = f->mdct_line_num;
    int sfb_num       = f->sfb_num;
    int *swb_low      = f->swb_low;
    int *swb_high     = f->swb_high;

    float *mdct_line_sfb = f->mdct_line;

    int swb_width;
    int group_offset;
    int gr, swb, win, i, k;
    int index;


    k = 0;
    group_offset = 0;

    /*order rearrage:  sfb[gr][sb][win][k] ---> swb[gr][win][sb][k]*/
    for (gr = 0; gr < num_window_groups; gr++) {
        for (swb = 0; swb < sfb_num; swb++) {
            swb_width = swb_high[swb] - swb_low[swb] + 1;
            for (win = 0; win < window_group_length[gr]; win++) {
                for (i = 0; i < swb_width; i++) {
                    index = group_offset + swb_low[swb] + win*mdct_line_num + i;
                    mdct_line_swb[index] = mdct_line_sfb[k++];
                }
            }
        }
        group_offset += mdct_line_num * window_group_length[gr];
    }

    for (i = 0; i < 1024; i++)
        mdct_line_swb[i] = mdct_line_swb[i] * mdct_line_sign[i];

}


int  fa_mdctline_encode(uintptr_t handle, int *x_quant, int num_window_groups, int *window_group_length, 
                        int quant_hufftab_no[8][FA_SWB_NUM_MAX], 
                        int *max_sfb, int *x_quant_code, int *x_quant_bits)
{
    fa_mdctquant_t *f = (fa_mdctquant_t *)handle;

    int sfb_num       = f->sfb_num;
    int *x_quant_gr;
    int *x_quant_code_gr;
    int *x_quant_bits_gr;
    int spectral_count;
    int group_offset;

    int gr;
    int gr_max_sfb;


    x_quant_gr          = x_quant;
    x_quant_code_gr     = x_quant_code;
    x_quant_bits_gr     = x_quant_bits;
    spectral_count = 0;
    *max_sfb = 0;
    gr_max_sfb = 0;
    group_offset = 0;

    /*FA_CLOCK_START(4);*/
    for (gr = 0; gr < num_window_groups; gr++) {
        x_quant_code_gr     += group_offset;
        x_quant_bits_gr     += group_offset;

        fa_noiseless_huffman_bitcount(x_quant_gr, sfb_num,  f->sfb_low[gr],
                                      quant_hufftab_no[gr], x_quant_bits_gr);
        group_offset = fa_huffman_encode_mdctline(x_quant_gr, sfb_num, f->sfb_low[gr], 
                                                     quant_hufftab_no[gr], &gr_max_sfb, x_quant_code_gr, x_quant_bits_gr);
        spectral_count += group_offset;
        *max_sfb = FA_MAX(*max_sfb, gr_max_sfb);
    }

    /*FA_CLOCK_END(4);*/
    /*FA_CLOCK_COST(4);*/
    return spectral_count;
}

               
void fa_mdctline_ms_encode(uintptr_t hl, uintptr_t hr, int num_window_groups,
                           ms_info_t *ms_l, ms_info_t *ms_r)
{
    fa_mdctquant_t *fl = (fa_mdctquant_t *)hl;
    fa_mdctquant_t *fr = (fa_mdctquant_t *)hr;

    int sfb_num       = fl->sfb_num;
    float *mdctline_l = fl->mdct_line;
    float *mdctline_r = fr->mdct_line;

    int gr;
    int *sfb_offset;
    int sfb;

    for (gr = 0; gr < num_window_groups; gr++) {
        sfb_offset = fl->sfb_low[gr];

        for (sfb = 0; sfb < sfb_num; sfb++) {
            int   ms = 0;
            float sum, diff;
            float enrgs, enrgd, enrgl, enrgr;
            float maxs, maxd, maxl, maxr;
            int offset, length;
            int i;

            offset     = sfb_offset[sfb];
            length     = sfb_offset[sfb+1] - sfb_offset[sfb];

            for (i = offset; i < offset+length; i++) {
                float lx = mdctline_l[i];
                float rx = mdctline_r[i];

                sum  = 0.5 * (lx + rx);
                diff = 0.5 * (lx - rx);

                enrgs += sum * sum;
                maxs = FA_MAX(maxs, FA_ABS(sum));

                enrgd += diff * diff;
                maxd = FA_MAX(maxd, FA_ABS(diff));

                enrgl += lx * lx;
                enrgr += rx * rx;

                maxl = FA_MAX(maxl, FA_ABS(lx));
                maxr = FA_MAX(maxr, FA_ABS(rx));

            }

            if ((FA_MIN(enrgs, enrgd) < FA_MIN(enrgl, enrgr))
                 && (FA_MIN(maxs, maxd) < FA_MIN(maxl, maxr)))
                ms = 1;

            //printf("%d:%d\n", sfb, ms);

            ms_l->ms_used[gr][sfb] = ms_r->ms_used[gr][sfb] = ms;
            ms_l->is_present = 1;
            ms_r->is_present = 1;

            if (ms) {
                for (i = offset; i < offset+length; i++) {
                    sum  = mdctline_l[i] + mdctline_r[i];
                    diff = mdctline_l[i] - mdctline_r[i];
                    mdctline_l[i] = 0.5 * sum;
                    mdctline_r[i] = 0.5 * diff;
                }
            }
        }
    }

}


