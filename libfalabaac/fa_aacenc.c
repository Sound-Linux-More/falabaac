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


  filename: fa_aacenc.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "fa_aacenc.h"
#include "fa_aaccfg.h"
#include "fa_aacpsy.h"
#include "fa_swbtab.h"
#include "fa_mdctquant.h"
#include "fa_aacblockswitch.h"
#include "fa_aacfilterbank.h"
#include "fa_bitstream.h"
#include "fa_aacstream.h"
#include "fa_aacms.h"
#include "fa_aacquant.h"
#include "fa_huffman.h"
#include "fa_tns.h"

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#define GAIN_ADJUST   4 //5  

#define BW_MAX        22000.

/* Returns the sample rate index */
int get_samplerate_index(int sample_rate)
{
    if (92017 <= sample_rate) return 0;
    if (75132 <= sample_rate) return 1;
    if (55426 <= sample_rate) return 2;
    if (46009 <= sample_rate) return 3;
    if (37566 <= sample_rate) return 4;
    if (27713 <= sample_rate) return 5;
    if (23004 <= sample_rate) return 6;
    if (18783 <= sample_rate) return 7;
    if (13856 <= sample_rate) return 8;
    if (11502 <= sample_rate) return 9;
    if (9391  <= sample_rate) return 10;

    return 11;
}


/* Max prediction band for backward predictionas function of fs index */
const int _max_pred_sfb[] = { 33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34, 0 };

int get_max_pred_sfb(int sample_rate_index)
{
    return _max_pred_sfb[sample_rate_index];
}

typedef struct _rate_cutoff {
    int bit_rate;
    float qcof;  //quality cof
    float cutoff;
}rate_cutoff_t;

#define QCOF_MIN  (0.6)
#define QCOF_MAX  (1.6)
/*reference to 48000kHz sample rate*/
static rate_cutoff_t rate_cutoff[] = 
{
    {12000, 1.61, 5000.},
    {15000, 1.51, 5000.},
    {23000, 1.41, 8000.},
    {31000, 1.31, 10000.},
    {39000, 1.28, 13000.},
    {45000, 1.18, 15200.},
    {63000, 1.01, 16200.},
    {70000, 0.91, 17000.},
    {80000, 0.81, 17000.},
    {100000, 0.71, 20000.},
    {120000, 0.66, 20000.},
    {160000, 0.61, BW_MAX},
    {0    , 0},
};

typedef struct _bit_thr_cof_t{
    int bit_rate;
    float cof;
}bit_thr_cof_t;

static bit_thr_cof_t bit_thr_cof[] = 
{
    {16000, 0.5},
    {24000, 0.5},
    {32000, 0.6},
    {38000, 0.7},
    {48000, 0.9},
    {64000, 1.3},
    {80000, 1.4},
    {100000, 1.5},
    {120000, 1.8},
    {180000, 7},
    {0    , 0},

};

typedef struct _adj_cof_t {
    int bit_rate;
    float adj;
} adj_cof_t;

static adj_cof_t adj_cof[] = 
{
    {16000, 0.4},
    {24000, 0.4},
    {32000, 0.35},
    {40000, 0.32},
    {48000, 0.3},
    {64000, 0},
    {80000, -0.2},
    {100000, -0.25},
    {120000, -0.3},
    {180000, -0.4},
    {0    , 0},
};

static float get_bandwidth(int chn, int sample_rate, int bit_rate, float *qcof) 
{
    int i;
    int tmpbitrate;
    float bandwidth;
    float ratio;

    ratio = (float) 48000/sample_rate;
    tmpbitrate = bit_rate * ratio;
    tmpbitrate = tmpbitrate/chn;

    for (i = 0; i < rate_cutoff[i].bit_rate; i++) {
        if (rate_cutoff[i].bit_rate >= tmpbitrate)
            break;
    }

    if (i > 0) {
        bandwidth = rate_cutoff[i-1].cutoff;
        *qcof = rate_cutoff[i-1].qcof;
    } else {
        bandwidth = 5000.;
        *qcof = 1.4;
    }

    if (bandwidth == 0.)
        /*bandwidth = 20000;*/
        bandwidth = BW_MAX;
    /*printf("bandwidth = %d\n", bandwidth);*/
    /*assert(bandwidth > 0 && bandwidth <= 20000);*/
    assert(bandwidth > 0. && bandwidth <= BW_MAX);

    return bandwidth;

}


static float get_bandwidth1(int chn, int sample_rate, float qcof, int *bit_rate) 
{
    int i;
    float bandwidth;

    i = 0;
    /*for (i = 0; i < rate_cutoff[i].qcof; i++) {*/
    while (rate_cutoff[i].qcof) {
        if (rate_cutoff[i].qcof <= qcof)
            break;
        i++;
    }

    if (i > 0) {
        *bit_rate = rate_cutoff[i-1].bit_rate;
        bandwidth = rate_cutoff[i-1].cutoff;
    } else {
        *bit_rate = 15000;
        bandwidth = 5000.;
    }

    if (bandwidth == 0.)
        /*bandwidth = 20000;*/
        bandwidth = BW_MAX;
    /*printf("bandwidth = %d\n", bandwidth);*/
    /*assert(bandwidth > 0 && bandwidth <= 20000);*/
    assert(bandwidth > 0. && bandwidth <= BW_MAX);

    return bandwidth;

}


static float get_bit_thr_cof(int chn, int sample_rate, int bit_rate)
{
    int i;
    float thr_cof;
    int tmpbitrate;
    int bandwidth;
    float ratio;

    ratio = (float) 48000/sample_rate;
    tmpbitrate = bit_rate * ratio;
    tmpbitrate = tmpbitrate/chn;

    for (i = 0; i < bit_thr_cof[i].bit_rate; i++) {
        if (bit_thr_cof[i].bit_rate >= tmpbitrate)
            break;
    }

    if (i > 0)
        thr_cof = bit_thr_cof[i-1].cof;
    else 
        thr_cof = 1.0;

    return thr_cof;
}


static float get_adj_cof(int chn, int sample_rate, int bit_rate)
{
    int i;
    float adj;
    int tmpbitrate;
    int bandwidth;
    float ratio;

    ratio = (float) 48000/sample_rate;
    tmpbitrate = bit_rate * ratio;
    tmpbitrate = tmpbitrate/chn;

    for (i = 0; i < adj_cof[i].bit_rate; i++) {
        if (adj_cof[i].bit_rate >= tmpbitrate)
            break;
    }

    if (i > 0)
        adj = adj_cof[i-1].adj;
    else 
        adj = 0.0;

    return adj;
}




static int get_cutoff_line(int sample_rate, int fmax_line_offset, int bandwidth)
{
    float fmax;
    float delta_f;
    int offset;

    fmax = (float)sample_rate/2.;
    delta_f = fmax/fmax_line_offset;

    offset = (int)((float)bandwidth/delta_f);

    return offset;

}

static int get_cutoff_sfb(int sfb_offset_max, int *sfb_offset, int cutoff_line)
{
    int i;

    for (i = 0; i < sfb_offset_max; i++) {
        if (sfb_offset[i] >= cutoff_line)
            break;
    }
#if 0 
    if (i == sfb_offset_max)
        return i;
    else 
        return (i+1);
#else
        return i;
#endif
}

static void fa_aacenc_rom_init()
{
    fa_psychomodel1_rom_init();
    fa_mdctquant_rom_init();
    fa_huffman_rom_init();
    fa_protect_db_rom_init();
}

uintptr_t aacenc_init(int sample_rate, int bit_rate, int chn_num, float qcof, int vbr_flag,
                      int mpeg_version, int aac_objtype, float band_width, int speed_level,
                      int ms_enable, int lfe_enable, int tns_enable, int block_switch_enable, int psy_enable, int psy_model,
                      int blockswitch_method, int quantize_method, int time_resolution_first)
{
    int i;
    int bits_adj;
    int bits_average;
    int bits_res_maxsize;
    int real_band_width;
    float bits_thr_cof;
    float adj;
    fa_aacenc_ctx_t *f = (fa_aacenc_ctx_t *)malloc(sizeof(fa_aacenc_ctx_t));

    chn_info_t chn_info_tmp[MAX_CHANNELS];
/*
    if (bit_rate > 256000 || bit_rate < 32000)
        return (uintptr_t)NULL;
*/
    memset(f, 0, sizeof(fa_aacenc_ctx_t));
    f->speed_level = speed_level;

    /*init rom*/
    fa_aacenc_rom_init();

    /*init configuration*/
    f->cfg.sample_rate   = sample_rate;
    f->cfg.bit_rate      = bit_rate;
    f->cfg.qcof          = qcof;
    f->cfg.vbr_flag      = vbr_flag;
    f->cfg.chn_num       = chn_num;
    f->cfg.mpeg_version  = mpeg_version;
    f->cfg.aac_objtype   = aac_objtype;
    f->cfg.ms_enable     = ms_enable;
    f->cfg.lfe_enable    = lfe_enable;
    f->cfg.tns_enable    = tns_enable;
    f->cfg.sample_rate_index = get_samplerate_index(sample_rate);

    f->sample = (float *)malloc(sizeof(float)*chn_num*AAC_FRAME_LEN);
    memset(f->sample, 0, sizeof(float)*chn_num*AAC_FRAME_LEN);

    f->block_switch_en = block_switch_enable;
    f->psy_enable      = psy_enable;
    f->psy_model       = psy_model;

    if (vbr_flag) {
        f->band_width = get_bandwidth1(chn_num, sample_rate, qcof, &(f->cfg.bit_rate)); 
        /*printf("\nNOTE: final bitrate/chn = %d\n", f->cfg.bit_rate);*/
    } else {
        f->band_width = get_bandwidth(chn_num, sample_rate, bit_rate, &(f->cfg.qcof));
        /*printf("\nNOTE: final qcof = %f\n", f->cfg.qcof);*/
    }

    if (bit_rate >= 200000 && bit_rate < 260000)
        bits_adj = 9000;
    if (bit_rate >= 140000 && bit_rate < 200000)
        bits_adj = 4000;
    if (bit_rate >= 90000  && bit_rate < 110000)
        bits_adj = -2000;

#if 0
    bits_thr_cof  = get_bit_thr_cof(chn_num, sample_rate, bit_rate);
    adj = get_adj_cof(chn_num, sample_rate, bit_rate);
#else 
    bits_thr_cof  = get_bit_thr_cof(chn_num, sample_rate, bit_rate+bits_adj);
    adj = get_adj_cof(chn_num, sample_rate, bit_rate+bits_adj);
#endif
    /*printf("bits thr cof=%f\n", bits_thr_cof);*/

    if (speed_level > 5) {
        if (f->band_width > 10000.)
            f->band_width = 10000.;
    }
    /*if (band_width >= 5000 && band_width <= 20000) {*/
    if (band_width >= 5000. && band_width <= BW_MAX) {
        if (band_width < f->band_width)
            f->band_width = band_width;
    }
    printf("band width= %f kHz\n", f->band_width);

    memset(chn_info_tmp, 0, sizeof(chn_info_t)*MAX_CHANNELS);
    get_aac_chn_info(chn_info_tmp, chn_num, lfe_enable);

    /*bits_average  = (bit_rate*1024)/(sample_rate*chn_num);*/
    bits_average  = ((bit_rate+bits_adj)*1024)/(sample_rate*chn_num);
    bits_res_maxsize = get_aac_bitreservoir_maxsize(bits_average, sample_rate);
    f->h_bitstream = fa_bitstream_init((6144/8)*chn_num);


    switch (blockswitch_method) {
        case BLOCKSWITCH_PSY:
            f->blockswitch_method = BLOCKSWITCH_PSY;
            f->do_blockswitch  = fa_blockswitch_psy;
            break;
        case BLOCKSWITCH_VAR:
            f->blockswitch_method = BLOCKSWITCH_VAR;
            f->do_blockswitch  = fa_blockswitch_var;
            break;
        default:
            f->blockswitch_method = BLOCKSWITCH_VAR;
            f->do_blockswitch  = fa_blockswitch_var;
            break;

    }

    switch (quantize_method) {
        case QUANTIZE_LOOP:
            f->quantize_method = QUANTIZE_LOOP;
            f->do_quantize = fa_quantize_loop;
            break;
        case QUANTIZE_FAST:
            f->quantize_method = QUANTIZE_FAST;
            f->do_quantize = fa_quantize_fast;
            break;
        case QUANTIZE_BEST:
            f->quantize_method = QUANTIZE_BEST;
            f->do_quantize = fa_quantize_best;
            /*f->do_quantize = fa_quantize_fast;*/
            break;
        default:
            f->quantize_method = QUANTIZE_BEST;
            f->do_quantize = fa_quantize_best;

    }

    /*init psy and mdct quant */
    for (i = 0; i < chn_num; i++) {
        f->ctx[i].h_blockctrl = fa_blockswitch_init(2048);
        f->ctx[i].time_resolution_first = time_resolution_first;

        f->ctx[i].pe                = 0.0;
        f->ctx[i].var_max_prev      = 0.0;
        f->ctx[i].block_type        = ONLY_LONG_BLOCK;
        f->ctx[i].psy_enable        = psy_enable;
        f->ctx[i].window_shape      = SINE_WINDOW;
        f->ctx[i].common_scalefac   = 0;
        memset(f->ctx[i].scalefactor, 0, sizeof(int)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].scalefactor_win, 0, sizeof(int)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].maxscale_win,0, sizeof(int)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].xmin, 0, sizeof(float)*8*FA_SWB_NUM_MAX);

        memset(f->ctx[i].miu,0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].miuhalf,0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].miu2, 0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].pdft, 0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].Px, 0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].Tm, 0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].Ti, 0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].Ti1,0, sizeof(float)*8*FA_SWB_NUM_MAX);
        memset(f->ctx[i].G  ,0, sizeof(float)*8*FA_SWB_NUM_MAX);
        f->ctx[i].Pt_long  = 0;
        f->ctx[i].Pt_short = 0;
        f->ctx[i].up = 0;
        f->ctx[i].step_down_db = 0.0;
        f->ctx[i].bit_thr_cof = bits_thr_cof;
        f->ctx[i].adj         = adj;

        f->ctx[i].num_window_groups = 1;
        f->ctx[i].window_group_length[0] = 1;
        f->ctx[i].window_group_length[1] = 0;
        f->ctx[i].window_group_length[2] = 0;
        f->ctx[i].window_group_length[3] = 0;
        f->ctx[i].window_group_length[4] = 0;
        f->ctx[i].window_group_length[5] = 0;
        f->ctx[i].window_group_length[6] = 0;
        f->ctx[i].window_group_length[7] = 0;

        memset(f->ctx[i].lastx, 0, sizeof(int)*8);
        memset(f->ctx[i].avgenergy, 0, sizeof(float)*8);

        f->ctx[i].used_bits= 0;

        f->ctx[i].bits_average     = bits_average;
        f->ctx[i].bits_res_maxsize = bits_res_maxsize;
        f->ctx[i].res_buf          = (unsigned char *)malloc(sizeof(unsigned char)*(bits_res_maxsize/8 + 1));
        memset(f->ctx[i].res_buf, 0, sizeof(unsigned char)*(bits_res_maxsize/8 + 1));
        f->ctx[i].bits_res_size    = 0;
        f->ctx[i].last_common_scalefac = 0;

        f->ctx[i].h_aacpsy        = fa_aacpsy_init(sample_rate);
        f->ctx[i].h_aac_analysis  = fa_aacfilterbank_init();

        f->ctx[i].h_tns           = fa_tns_init(f->cfg.mpeg_version, f->cfg.aac_objtype, f->cfg.sample_rate_index);
        f->ctx[i].tns_active      = 0;

        memcpy(&(f->ctx[i].chn_info), &(chn_info_tmp[i]), sizeof(chn_info_t));
        f->ctx[i].chn_info.common_window = 0;

        if (f->ctx[i].chn_info.lfe == 1) 
            real_band_width = 2000;
        else 
            real_band_width = (int)f->band_width;
        
        switch (sample_rate) {
            case 48000:
                f->ctx[i].cutoff_line_long = get_cutoff_line(48000, 1024, real_band_width);
                f->ctx[i].cutoff_line_short= get_cutoff_line(48000, 128 , real_band_width);
                f->ctx[i].cutoff_sfb_long  = get_cutoff_sfb(FA_SWB_48k_LONG_NUM , fa_swb_48k_long_offset , f->ctx[i].cutoff_line_long);
                f->ctx[i].cutoff_sfb_short = get_cutoff_sfb(FA_SWB_48k_SHORT_NUM, fa_swb_48k_short_offset, f->ctx[i].cutoff_line_short);
                f->ctx[i].h_mdctq_long = fa_mdctquant_init(1024, f->ctx[i].cutoff_sfb_long , fa_swb_48k_long_offset, 1);
                f->ctx[i].h_mdctq_short= fa_mdctquant_init(128 , f->ctx[i].cutoff_sfb_short, fa_swb_48k_short_offset, 8);
                f->ctx[i].sfb_num_long = f->ctx[i].cutoff_sfb_long;
                f->ctx[i].sfb_num_short= f->ctx[i].cutoff_sfb_short;
                f->ctx[i].Pt_long  = fa_protect_db_48k_long;
                f->ctx[i].Pt_short = fa_protect_db_48k_short;
                break;
            case 44100:
                f->ctx[i].cutoff_line_long = get_cutoff_line(44100, 1024, real_band_width);
                f->ctx[i].cutoff_line_short= get_cutoff_line(44100, 128 , real_band_width);
                f->ctx[i].cutoff_sfb_long  = get_cutoff_sfb(FA_SWB_44k_LONG_NUM , fa_swb_44k_long_offset , f->ctx[i].cutoff_line_long);
                f->ctx[i].cutoff_sfb_short = get_cutoff_sfb(FA_SWB_44k_SHORT_NUM, fa_swb_44k_short_offset, f->ctx[i].cutoff_line_short);
                f->ctx[i].h_mdctq_long = fa_mdctquant_init(1024, f->ctx[i].cutoff_sfb_long , fa_swb_44k_long_offset, 1);
                f->ctx[i].h_mdctq_short= fa_mdctquant_init(128 , f->ctx[i].cutoff_sfb_short, fa_swb_44k_short_offset, 8);
                f->ctx[i].sfb_num_long = f->ctx[i].cutoff_sfb_long;
                f->ctx[i].sfb_num_short= f->ctx[i].cutoff_sfb_short;
                f->ctx[i].Pt_long  = fa_protect_db_44k_long;
                f->ctx[i].Pt_short = fa_protect_db_44k_short;
                break;
            case 32000:
                f->ctx[i].cutoff_line_long = get_cutoff_line(32000, 1024, real_band_width);
                f->ctx[i].cutoff_line_short= get_cutoff_line(32000, 128 , real_band_width);
                f->ctx[i].cutoff_sfb_long  = get_cutoff_sfb(FA_SWB_32k_LONG_NUM , fa_swb_32k_long_offset , f->ctx[i].cutoff_line_long);
                f->ctx[i].cutoff_sfb_short = get_cutoff_sfb(FA_SWB_32k_SHORT_NUM, fa_swb_32k_short_offset, f->ctx[i].cutoff_line_short);
                f->ctx[i].h_mdctq_long = fa_mdctquant_init(1024, f->ctx[i].cutoff_sfb_long , fa_swb_32k_long_offset, 1);
                f->ctx[i].h_mdctq_short= fa_mdctquant_init(128 , f->ctx[i].cutoff_sfb_short, fa_swb_32k_short_offset, 8);
                f->ctx[i].sfb_num_long = f->ctx[i].cutoff_sfb_long;
                f->ctx[i].sfb_num_short= f->ctx[i].cutoff_sfb_short;
                f->ctx[i].Pt_long  = fa_protect_db_32k_long;
                f->ctx[i].Pt_short = fa_protect_db_32k_short;
                break;
        }


        memset(f->ctx[i].mdct_line, 0, sizeof(float)*2*AAC_FRAME_LEN);

        /*f->ctx[i].max_pred_sfb = get_max_pred_sfb(f->cfg.sample_rate_index);*/

        f->ctx[i].quant_ok = 0;

        if (f->band_width < BW_MAX) {
            if (time_resolution_first)
                fa_quantqdf_para_init(&(f->ctx[i].qp), 0.9);
            else 
                fa_quantqdf_para_init(&(f->ctx[i].qp), 0.95);
        } else { 
            if (time_resolution_first)
                fa_quantqdf_para_init(&(f->ctx[i].qp), 0.9);
            else 
                fa_quantqdf_para_init(&(f->ctx[i].qp), 1.0);
        }

    }

    /*f->bitres_maxsize = get_aac_bitreservoir_maxsize(f->cfg.bit_rate, f->cfg.sample_rate);*/
    

    return (uintptr_t)f;
}

void fa_aacenc_uninit(uintptr_t handle)
{
    int i;
    int chn_num;
    fa_aacenc_ctx_t *f = (fa_aacenc_ctx_t *)handle;

    chn_num = f->cfg.chn_num;
    if (f) {
        if (f->sample) {
            free(f->sample);
            f->sample = NULL;
        }

        fa_bitstream_uninit(f->h_bitstream);

        for (i = 0; i < chn_num; i++) {
            fa_blockswitch_uninit(f->ctx[i].h_blockctrl);
            free(f->ctx[i].res_buf);
            f->ctx[i].res_buf = NULL;

            fa_aacpsy_uninit(f->ctx[i].h_aacpsy);
            fa_aacfilterbank_uninit(f->ctx[i].h_aac_analysis);
            fa_tns_uninit(f->ctx[i].h_tns);
            fa_mdctquant_uninit(f->ctx[i].h_mdctq_long);
            fa_mdctquant_uninit(f->ctx[i].h_mdctq_short);
        }

        memset(f, 0, sizeof(fa_aacenc_ctx_t));
        free(f);
        f = NULL;
    }

}

#define SPEED_LEVEL_MAX  6 
#if 0
static int speed_level_tab[SPEED_LEVEL_MAX][6] = 
                            { //ms,      tns,     block_switch_en,       psy_en,       blockswitch_method,       quant_method
                                {1,       0,        1,                    1,           BLOCKSWITCH_VAR,          QUANTIZE_LOOP},  //1
                                {0,       0,        1,                    1,           BLOCKSWITCH_VAR,          QUANTIZE_FAST},  //2
                                {1,       0,        1,                    1,           BLOCKSWITCH_VAR,          QUANTIZE_FAST},  //3
                                {1,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_LOOP},  //4
                                {0,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_LOOP},  //5
                                {0,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_LOOP},  //same, but bw=10k
                            };
#else 
static int speed_level_tab[SPEED_LEVEL_MAX][6] = 
                            { //ms,      tns,     block_switch_en,       psy_en,       blockswitch_method,       quant_method
                                {1,       1,        1,                    1,           BLOCKSWITCH_PSY,          QUANTIZE_BEST},  //1
                                {1,       1,        0,                    1,           BLOCKSWITCH_VAR,          QUANTIZE_BEST},  //2
                                {1,       0,        1,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_BEST},  //3
                                {1,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_BEST},  //4
                                {0,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_BEST},  //5
                                {0,       0,        0,                    0,           BLOCKSWITCH_VAR,          QUANTIZE_BEST},  //same, but bw=10k
                            };

#endif

uintptr_t fa_aacenc_init(int sample_rate, int bit_rate, int chn_num, float quality, int vbr_flag,
                         int mpeg_version, int aac_objtype, int lfe_enable,
                         float band_width,
                         int speed_level,
                         int time_resolution_first)
{

    int ms_enable;
    int tns_enable;
    int block_switch_enable;
    int blockswitch_method;
    int quantize_method;
    int psy_enable;
    int psy_model;

    uintptr_t handle;

    int bit_rate_max, bit_rate_min;
    int bit_rate_chn;

    int speed_index;

    float qcof;

    if (speed_level > SPEED_LEVEL_MAX || speed_level < 1)
        return 0;

    speed_index = speed_level - 1;

    ms_enable            = speed_level_tab[speed_index][0];
    tns_enable           = speed_level_tab[speed_index][1];
    block_switch_enable  = speed_level_tab[speed_index][2];
    psy_enable           = speed_level_tab[speed_index][3];
    /*psy_model            = PSYCH1;*/
    psy_model            = PSYCH2;
    blockswitch_method   = speed_level_tab[speed_index][4];
    quantize_method      = speed_level_tab[speed_index][5];

    bit_rate_min = 16;
    bit_rate_max = 160;
    bit_rate_chn = bit_rate/chn_num;

    qcof = QCOF_MAX - quality;
    if (qcof < QCOF_MIN)
        qcof = QCOF_MIN;
    if (qcof > QCOF_MAX)
        qcof = QCOF_MAX;

    if ((bit_rate_chn > bit_rate_max) || (bit_rate_chn < bit_rate_min)) {
        printf("total bitrate=%d, per chn = %d\n", bit_rate, bit_rate_chn);
        printf("bitrate not support, only suporrt [16~160]kbps per chn\n");
        exit(0);
    }

    //2012-12-02
    fa_logtab_init();

    handle = aacenc_init(sample_rate, bit_rate*1000, chn_num, qcof, vbr_flag,
                         mpeg_version, aac_objtype, band_width*1000, speed_level,
                         ms_enable, lfe_enable, tns_enable, block_switch_enable, psy_enable, psy_model,
                         blockswitch_method, quantize_method, time_resolution_first);

    return handle;

}


static void zero_cutoff(float *mdct_line, int mdct_line_num, int cutoff_line)
{
    int i;

    for (i = cutoff_line; i < mdct_line_num; i++)
        mdct_line[i] = 0;

}

static void mdct_line_normarlize(fa_aacenc_ctx_t *f)
{

    int i;
    int j,k;
    int chn_num;
    aacenc_ctx_t *s;

    chn_num = f->cfg.chn_num;

    /*FA_CLOCK_START(6);*/
    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);
        if (s->block_type == ONLY_SHORT_BLOCK) {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_short);
            for (k = 0; k < 8; k++) {
                for (j = 0; j < 128; j++)
                    s->mdct_line[j+k * AAC_BLOCK_SHORT_LEN] /= s->max_mdct_line;
            }
        } else {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_long);
            for (j = 0; j < 1024; j++)
                s->mdct_line[j] /= s->max_mdct_line;
        }
    }
 

}

static void mdctline_reorder(aacenc_ctx_t *s, float xmin[8][FA_SWB_NUM_MAX])
{
    /*use mdct transform*/
    if (s->block_type == ONLY_SHORT_BLOCK) {
        fa_mdctline_sfb_arrange(s->h_mdctq_short, s->mdct_line, 
                                s->num_window_groups, s->window_group_length);
        fa_xmin_sfb_arrange(s->h_mdctq_short, xmin,
                            s->num_window_groups, s->window_group_length);

    } else {
        fa_mdctline_sfb_arrange(s->h_mdctq_long, s->mdct_line, 
                                s->num_window_groups, s->window_group_length);
        fa_xmin_sfb_arrange(s->h_mdctq_long, xmin,
                            s->num_window_groups, s->window_group_length);

    }

}

static void scalefactor_recalculate(fa_aacenc_ctx_t *f, int chn_num)
{
    int i;
    int gr, sfb, sfb_num;
    aacenc_ctx_t *s;

    for (i = 0; i < chn_num ; i++) {
        s = &(f->ctx[i]);
        if (s->block_type == ONLY_SHORT_BLOCK) 
            sfb_num = fa_mdctline_get_sfbnum(s->h_mdctq_short);
        else 
            sfb_num = fa_mdctline_get_sfbnum(s->h_mdctq_long);

        for (gr = 0; gr < s->num_window_groups; gr++) {
            for (sfb = 0; sfb < sfb_num; sfb++) {
                s->scalefactor[gr][sfb] = s->common_scalefac - s->scalefactor[gr][sfb] + GAIN_ADJUST + SF_OFFSET;
                s->scalefactor[gr][sfb] = FA_MAX(s->scalefactor[gr][sfb], 0);
                s->scalefactor[gr][sfb] = FA_MIN(s->scalefactor[gr][sfb], 255);
            }
        }
        s->common_scalefac = s->scalefactor[0][0];
    }


}

void fa_aacenc_encode(uintptr_t handle, unsigned char *buf_in, int inlen, unsigned char *buf_out, int *outlen)
{
    int i,j;
    int chn_num;
    short *sample_in;
    float *sample_buf;
    float sample_psy_buf[2*AAC_FRAME_LEN];
    int ms_enable;
    int tns_enable;
    int block_switch_en;
    int psy_enable;
    int psy_model;
    int speed_level;
    fa_aacenc_ctx_t *f = (fa_aacenc_ctx_t *)handle;
    aacenc_ctx_t *s;
    int block_type;

    float qcof;

    speed_level = f->speed_level;

    tns_enable  = f->cfg.tns_enable;
    ms_enable   = f->cfg.ms_enable;
    chn_num     = f->cfg.chn_num;
    qcof        = f->cfg.qcof;
    /*assert(inlen == chn_num*AAC_FRAME_LEN*2);*/

    /*update sample buffer, ith sample, jth chn*/
    sample_in = (short *)buf_in;
    for (i = 0; i < AAC_FRAME_LEN; i++) 
        for (j = 0; j < chn_num; j++) 
            f->sample[i+j*AAC_FRAME_LEN] = (float)(0.99 * sample_in[i*chn_num+j]);

    block_switch_en = f->block_switch_en;
    psy_enable      = f->psy_enable;
    psy_model       = f->psy_model;

    /*block switch and use filterbank to generate mdctline*/
    block_type = ONLY_LONG_BLOCK;

    /*should use cpe or sce to decide in the future*/
    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);

        /*block switch */
        if (s->time_resolution_first) {
            s->block_type = ONLY_SHORT_BLOCK;
        } else {
            if (block_switch_en) {
#if 0 
                f->do_blockswitch(s);
#else 
                fa_blockswitch_robust(s, f->sample+i*AAC_FRAME_LEN);
#endif 
            } else {
                s->block_type = ONLY_LONG_BLOCK;
                /*s->block_type = ONLY_SHORT_BLOCK;*/
            }
        }
    }

    fa_blocksync(f);

    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);

        memset(s->xmin, 0, sizeof(float)*8*FA_SWB_NUM_MAX);

        /*get the input sample*/
        sample_buf = f->sample+i*AAC_FRAME_LEN;

        /*analysis*/
        fa_aacfilterbank_analysis(s->h_aac_analysis, s->block_type, &(s->window_shape),
                                  sample_buf, s->mdct_line);

        /*cutoff the frequence according to the bitrate*/
        if (s->block_type == ONLY_SHORT_BLOCK) {
            int k;
            for (k = 0; k < 8; k++)
                zero_cutoff(s->mdct_line+k*128, 128, s->cutoff_line_short);
        } else
            zero_cutoff(s->mdct_line, 1024, s->cutoff_line_long);

        /* 
           calculate xmin and pe
           --use current sample_buf calculate pe to decide which block used in the next frame
        */
        if (psy_enable) {
            fa_aacfilterbank_get_xbuf(s->h_aac_analysis, sample_psy_buf);
            if (psy_model == PSYCH1) {
                fa_aacpsy_calculate_xmin_usepsych1(s->h_aacpsy, s->mdct_line, s->block_type, s->xmin);
            } else {
                fa_aacpsy_calculate_pe(s->h_aacpsy, sample_psy_buf, s->block_type, &s->pe, &s->tns_active);
                /*fa_aacpsy_calculate_pe_hp(s->h_aacpsy, sample_psy_buf, s->block_type, &s->pe, &s->tns_active);*/
                fa_aacpsy_calculate_xmin(s->h_aacpsy, s->mdct_line, s->block_type, s->xmin, qcof);
                /*printf("=====>tns_active=%d\n", s->tns_active);*/
            }
            /*if (speed_level == 2 || speed_level == 3)*/
                /*fa_calculate_scalefactor_win(s, s->xmin);*/
        } else {
            /*if (speed_level < 5) {*/
                fa_fastquant_calculate_sfb_avgenergy(s);
                fa_fastquant_calculate_xmin(s, s->xmin, qcof);
                /*fa_calculate_scalefactor_win(s, s->xmin);*/   // if use QUANTIZE_FAST , uncommented
            /*}*/
        }
    }

    fa_tnssync(f);

    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);

        /*if (tns_enable && (!s->chn_info.lfe))*/
        /*if (tns_enable && (!s->chn_info.lfe) && (1 == s->tns_active))*/
        if (tns_enable && (!s->chn_info.lfe) &&
        /*if (tns_enable && (!s->chn_info.lfe) && (1 == s->tns_active) &&*/
            /*((s->block_type == ONLY_SHORT_BLOCK) || (s->block_type == LONG_START_BLOCK) || (s->block_type == LONG_STOP_BLOCK)))*/
           ((s->block_type == ONLY_SHORT_BLOCK))) // || (s->block_type == ONLY_LONG_BLOCK)) )
            fa_tns_encode_frame(s);

        /*if is short block , recorder will arrange the mdctline to sfb-grouped*/
        mdctline_reorder(s, s->xmin);

        /*reset the quantize status*/
        s->quant_ok = 0;
    }

    /*mid/side encoding*/
    if (ms_enable)
        fa_aacmsenc(f);

    /*quantize*/
    f->do_quantize(f);

    /* offset the difference of common_scalefac and scalefactors by SF_OFFSET  */
    scalefactor_recalculate(f, chn_num);

    /*format bitstream*/
    fa_write_bitstream(f);

    *outlen = fa_bitstream_getbufval(f->h_bitstream, buf_out);

    fa_bitstream_reset(f->h_bitstream);

}
