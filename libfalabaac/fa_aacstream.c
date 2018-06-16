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


  filename: fa_aacstream.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include "fa_aacstream.h"
#include "fa_bitstream.h"
#include "fa_huffman.h"
#include "fa_huffmantab.h"
#include "fa_tns.h"
#include "fa_fastmath.h"

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif


#define BYTE_NUMBIT 8       /* bits in byte (char) */
#define LONG_NUMBIT 32      /* bits in unsigned long */
#define bit2byte(a) (((a)+BYTE_NUMBIT-1)/BYTE_NUMBIT)


static int write_adtsheader(uintptr_t h_bs, aaccfg_t *c, int used_bytes, int write_flag);
static int write_icsinfo(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag, 
                        int objtype,
                        int common_window);
static int write_ics(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag,
                    int objtype,
                    int common_window);
static int write_cpe(uintptr_t h_bs, aacenc_ctx_t *s, aacenc_ctx_t *sr, int aac_objtype, int write_flag);
static int write_sce(uintptr_t h_bs, aacenc_ctx_t *s, int aac_objtype, int write_flag);
static int write_lfe(uintptr_t h_bs, aacenc_ctx_t *s, int aac_objtype, int write_flag);

static int find_grouping_bits(aacenc_ctx_t *s);
#if 0
static int write_ltp_predictor_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_predictor_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
#endif
static int write_pulse_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_tns_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_gaincontrol_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_spectral_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_aac_fillbits(uintptr_t h_bs, aacenc_ctx_t *s, int num_bits, int write_flag);
static int write_hufftab_no(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag);
static int write_scalefactor(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag) ;
static int write_bits_for_bytealign(uintptr_t h_bs, aacenc_ctx_t *s, int bits_sofar, int write_flag);

/* returns the maximum bitrate per channel for certain sample rate*/
int get_aac_max_bitrate(long sample_rate)
{
    /*maximum of 6144 bit for a channel*/
    return (int)(6144.0 * (float)sample_rate/(float)1024 + .5);
}

/* returns the minimum bitrate per channel*/
int get_aac_min_bitrate()
{
    return 8000;
}

int get_avaiable_bits(int average_bits, int more_bits, int bitres_bits, int bitres_max_size)
{
    int available_bits;

    if (more_bits >= 0) {
        available_bits = average_bits + FA_MIN(more_bits, bitres_bits);
    } else if (more_bits < 0) {
        available_bits = average_bits + FA_MAX(more_bits, bitres_bits-bitres_max_size);
    }

    if (available_bits == 1) {
        available_bits += 1;
    }

    return available_bits;
}



/* calculate bit_allocation based on PE */
int calculate_bit_allocation(float pe, int block_type)
{
    float pe1;
    float pe2;
    float bit_allocation;
    int bits_alloc;

    if (block_type == ONLY_SHORT_BLOCK) {
        pe1 = 0.6;
        pe2 = 24.0;
    } else {
        pe1 = 0.3;
        pe2 = 6.0;
    }
    bit_allocation = pe1 * pe + pe2 * fa_fast_sqrtf(pe);
    bit_allocation = FA_MIN(FA_MAX(0.0, bit_allocation), 6144.0);

    bits_alloc = (int)(bit_allocation + 0.5);
    if (bits_alloc > 3000)
        bits_alloc = 3000;

    return bits_alloc;
}


/* returns the maximum bit reservoir size */
int get_aac_bitreservoir_maxsize(int bit_rate, int sample_rate)
{
    return (6144 - (int)((float)bit_rate/(float)sample_rate*1024));
}

int fa_write_bitstream_onechn(uintptr_t h_bs, aaccfg_t *c, aacenc_ctx_t *s, aacenc_ctx_t *sr)
{

    chn_info_t *p_chn_info = &(s->chn_info);
    /*int channel;*/
    int bits = 0;
    /*int bits_left_afterfill, num_fill_bits;*/

    /*printf("------shape=%d\n", s->window_shape);*/

    if (p_chn_info->present) {
        /* Write out a single_channel_element */
        if (!p_chn_info->cpe) {
            if (p_chn_info->lfe) {
                /* Write out lfe */
                bits += write_lfe(h_bs, s, c->aac_objtype, 1);
            } else {
                /* Write out sce */
                bits += write_sce(h_bs, s, c->aac_objtype, 1);
            }
        } else {
            if (p_chn_info->ch_is_left) {
                /* Write out cpe */
                bits += write_cpe(h_bs, s, sr, c->aac_objtype, 1);
            }
        }
    }
#if 0
    /* Compute how many fill bits are needed to avoid overflowing bit reservoir */
    /* Save room for ID_END terminator */
    if (bits < (8 - LEN_SE_ID) ) {
        exit(0);
        num_fill_bits = 8 - LEN_SE_ID - bits;
    } else {
        num_fill_bits = 0;
    }

    /* Write AAC fill_elements, smallest fill element is 7 bits. */
    /* Function may leave up to 6 bits left after fill, so tell it to fill a few extra */
    num_fill_bits += 6;
    bits_left_afterfill = write_aac_fillbits(h_bs, s, num_fill_bits, 1);
    bits += (num_fill_bits - bits_left_afterfill);

    /* Write ID_END terminator */


    /* Now byte align the bitstream */
    /*bits += write_bits_for_bytealign(h_bs, s, bits, 1);*/
#else 
/*
    if (s->bits_res_size == s->bits_res_maxsize) {
        if (bits < s->bits_average) {
            num_fill_bits = (s->bits_average - bits - 6);
            bits_left_afterfill = write_aac_fillbits(h_bs, s, num_fill_bits, 1);
            bits += num_fill_bits - bits_left_afterfill;
        }
    }
*/
#endif 

    s->used_bits = bits; //bit2byte(bits);

    return bits;
}


int fa_write_bitstream(fa_aacenc_ctx_t *f)
{
    int i;
    int total_bits;
    int chn_num;
    int chn;
    int write_bits;
    aacenc_ctx_t *s, *sl, *sr;

    total_bits = 56;
    i   = 0;
    chn = 1;
    chn_num = f->cfg.chn_num;

    while (i < chn_num) {
        s = &(f->ctx[i]);

        if (s->chn_info.cpe == 1) {
            chn = 2;
            /*total_bits += s->used_bits;*/
            total_bits += fa_bits_count(f->h_bitstream, &f->cfg, &(f->ctx[i]), &(f->ctx[i+1]));

        } else if (s->chn_info.sce == 1) {
            chn = 1;
            /*total_bits += s->used_bits;*/
            total_bits += fa_bits_count(f->h_bitstream, &f->cfg, &(f->ctx[i]), NULL);
        } else {
            chn = 1;
            total_bits += s->used_bits;
        }

        i += chn;
    } 

    total_bits += LEN_SE_ID;
    total_bits += write_bits_for_bytealign(f->h_bitstream, s, total_bits, 0);
    f->used_bytes = bit2byte(total_bits);

    write_bits = 0;
    write_bits += write_adtsheader(f->h_bitstream, &f->cfg, f->used_bytes, 1);
    i = 0;
    chn = 1;
    while (i < chn_num) {
        s = &(f->ctx[i]);

        if (s->chn_info.cpe == 1) {
            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            write_bits += fa_write_bitstream_onechn(f->h_bitstream, &f->cfg, sl, sr);
        } else if (s->chn_info.sce == 1) {
            chn = 1;
            write_bits += fa_write_bitstream_onechn(f->h_bitstream, &f->cfg, s, NULL);

        } else {
            chn = 1;
            write_bits += fa_write_bitstream_onechn(f->h_bitstream, &f->cfg, s, NULL);
        }

        i += chn;
    } 

    write_bits += fa_bitstream_putbits(f->h_bitstream, ID_END, LEN_SE_ID);
    write_bits += write_bits_for_bytealign(f->h_bitstream, s, write_bits, 1);

    return total_bits;
}

int fa_bits_sideinfo_est(int chn_num)
{
    int bits;
/*
    int adts_bits;
    int end_bits;
    int bytsalign_bits;

    adts_bits = 56;
    end_bits  = 3;
    bytsalign_bits = 5;
*/
    bits = 64/chn_num; 

    return bits;
}

int fa_bits_count(uintptr_t h_bs, aaccfg_t *c, aacenc_ctx_t *s, aacenc_ctx_t *sr)
{
    chn_info_t *p_chn_info = &(s->chn_info);
    /*int channel;*/
    int bits = 0;
    /*int bits_left_afterfill, num_fill_bits;*/


    /*bits += write_adtsheader(c, s, 0);*/

    if (p_chn_info->present) {
        /* Write out a single_channel_element */
        if (!p_chn_info->cpe) {
            if (p_chn_info->lfe) {
                /* Write out lfe */
                bits += write_lfe(h_bs, s, c->aac_objtype, 0);
            } else {
                /* Write out sce */
                bits += write_sce(h_bs, s, c->aac_objtype, 0);
            }
        } else {
            if (p_chn_info->ch_is_left) {
                /* Write out cpe */
                bits += write_cpe(h_bs, s, sr, c->aac_objtype, 0);
            }
        }
    }
#if 0
    /* Compute how many fill bits are needed to avoid overflowing bit reservoir */
    /* Save room for ID_END terminator */
    if (bits < (8 - LEN_SE_ID) ) {
        num_fill_bits = 8 - LEN_SE_ID - bits;
    } else {
        num_fill_bits = 0;
    }

    /* Write AAC fill_elements, smallest fill element is 7 bits. */
    /* Function may leave up to 6 bits left after fill, so tell it to fill a few extra */
    num_fill_bits += 6;
    bits_left_afterfill = write_aac_fillbits(h_bs, s, num_fill_bits, 0);
    bits += (num_fill_bits - bits_left_afterfill);


    /* Now byte align the bitstream */
    /*bits += write_bits_for_bytealign(h_bs, s, bits, 0);*/
#else 
/*
    if (s->bits_res_size == s->bits_res_maxsize) {
        if (bits < s->bits_average) {
            num_fill_bits = (s->bits_average - bits - 6);
            bits_left_afterfill = write_aac_fillbits(h_bs, s, num_fill_bits, 0);
            bits += num_fill_bits - bits_left_afterfill;
        }
    }

*/
#endif 

    s->used_bits = bits; //bit2byte(bits);

    return bits;
}


static int write_adtsheader(uintptr_t h_bs, aaccfg_t *c, int used_bytes, int write_flag)
{
    /*aaccfg_t *c = &(f->cfg);*/
    /*unsigned long h_bs = s->h_bitstream;*/
    int bits = 56;

    if (write_flag) {
        /* Fixed ADTS header */
        fa_bitstream_putbits(h_bs, 0xFFFF, 12);                     /* 12 bit Syncword */
        //fa_bitstream_putbits(h_bs, c->mpeg_version, 1);         /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
        fa_bitstream_putbits(h_bs, 1, 1);         /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
        fa_bitstream_putbits(h_bs, 0, 2);                           /* layer == 0 */
        fa_bitstream_putbits(h_bs, 1, 1);                           /* protection absent */
        fa_bitstream_putbits(h_bs, c->aac_objtype-1, 2);        /* profile */
        fa_bitstream_putbits(h_bs, c->sample_rate_index, 4);    /* sampling rate */
        fa_bitstream_putbits(h_bs, 0, 1);                           /* private bit */
        fa_bitstream_putbits(h_bs, c->chn_num, 3);              /* ch. config (must be > 0) */
        fa_bitstream_putbits(h_bs, 0, 1);                           /* original/copy */
        fa_bitstream_putbits(h_bs, 0, 1);                           /* home */

        /* Variable ADTS header */
        fa_bitstream_putbits(h_bs, 0, 1);                           /* copyr. id. bit */
        fa_bitstream_putbits(h_bs, 0, 1);                           /* copyr. id. start */
        fa_bitstream_putbits(h_bs, used_bytes, 13);
        fa_bitstream_putbits(h_bs, 0x7FF, 11);                      /* buffer fullness (0x7FF for VBR) */
        fa_bitstream_putbits(h_bs, 0, 2);                           /* raw data blocks (0+1=1) */
    }

    return bits;
}

static int write_icsinfo(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag, 
                        int objtype,
                        int common_window)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int grouping_bits;
    int bits = 0;

    if (write_flag) {
        fa_bitstream_putbits(h_bs, 0, LEN_ICS_RESERV);   /* ics reserved bit*/
        fa_bitstream_putbits(h_bs, s->block_type, LEN_WIN_SEQ);  /* window sequence, block type */
        fa_bitstream_putbits(h_bs, s->window_shape, LEN_WIN_SH);  /* window shape */
    }
    bits += LEN_ICS_RESERV;
    bits += LEN_WIN_SEQ;
    bits += LEN_WIN_SH;

    /* For short windows, write out max_sfb and scale_factor_grouping */
    if (s->block_type == ONLY_SHORT_BLOCK){
        /*s->max_sfb = s->sfb_num_short;*/
        if (write_flag) {
            fa_bitstream_putbits(h_bs, s->max_sfb, LEN_MAX_SFBS);
            grouping_bits = find_grouping_bits(s);
            fa_bitstream_putbits(h_bs, grouping_bits, MAX_SHORT_WINDOWS - 1);  /* the grouping bits */
        }
        bits += LEN_MAX_SFBS;
        bits += MAX_SHORT_WINDOWS - 1;
    } else { /* Otherwise, write out max_sfb and predictor data */
        /*s->max_sfb = s->sfb_num_long;*/
        if (write_flag) {
            fa_bitstream_putbits(h_bs, s->max_sfb, LEN_MAX_SFBL);
        }
        bits += LEN_MAX_SFBL;
#if 0 
        if (objtype == LTP) {
            bits++;
            if (write_flag)
                fa_bitstream_putbits(h_bs, s->ltpInfo.global_pred_flag, 1); /* Prediction Global used */

            bits += write_ltp_predictor_data(h_bs, s, write_flag);
            if (common_window)
                bits += write_ltp_predictor_data(h_bs, s, write_flag);
        } else {
            bits++;
            if (write_flag)
                fa_bitstream_putbits(h_bs, s->pred_global_flag, LEN_PRED_PRES);  /* predictor_data_present */

            bits += write_predictor_data(h_bs, s, write_flag);
        }
#else 
        if (objtype != LTP) {
            bits++;
            if (write_flag)
                fa_bitstream_putbits(h_bs, 0, LEN_PRED_PRES);  /* predictor_data_present */
        } else {
            printf("LTP not support! exit now\n");
            exit(0);
        }

#endif
    }

    return bits;
}

static int write_ics(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag,
                    int objtype,
                    int common_window)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    /* this function writes out an individual_channel_stream to the bitstream and */
    /* returns the number of bits written to the bitstream */
    int bits = 0;

    /* Write the 8-bit global_gain */

    if (write_flag)
        fa_bitstream_putbits(h_bs, s->common_scalefac, LEN_GLOB_GAIN);
    bits += LEN_GLOB_GAIN;

    /* Write ics information */
    if (!common_window) {
        bits += write_icsinfo(h_bs, s, write_flag, objtype, common_window);
    }

    bits += write_hufftab_no(h_bs, s, write_flag);
    bits += write_scalefactor(h_bs, s, write_flag);

    bits += write_pulse_data(h_bs, s, write_flag);
    bits += write_tns_data(h_bs, s, write_flag);
    bits += write_gaincontrol_data(h_bs, s, write_flag);

    bits += write_spectral_data(h_bs, s, write_flag);

    return bits;
}



static int write_cpe(uintptr_t h_bs, aacenc_ctx_t *s, aacenc_ctx_t *sr, int aac_objtype, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    chn_info_t *p_chn_info = &(s->chn_info);
    int bits = 0;

    if (write_flag) {
        /* write ID_CPE, single_element_channel() identifier */
        fa_bitstream_putbits(h_bs, ID_CPE, LEN_SE_ID);

        /* write the element_identifier_tag */
        fa_bitstream_putbits(h_bs, p_chn_info->tag, LEN_TAG);

        /* common_window? */
        fa_bitstream_putbits(h_bs, p_chn_info->common_window, LEN_COM_WIN);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;
    bits += LEN_COM_WIN;

    /* if common_window, write ics_info */
    if (p_chn_info->common_window) {
        int num_windows, max_sfb;

        bits += write_icsinfo(h_bs, s, write_flag, aac_objtype, p_chn_info->common_window);
        num_windows = s->num_window_groups;
        max_sfb = s->max_sfb;

        if (write_flag) {
            fa_bitstream_putbits(h_bs, p_chn_info->ms_info.is_present, LEN_MASK_PRES);
            if (p_chn_info->ms_info.is_present == 1) {
                int gr;
                int b;
                for (gr = 0; gr < num_windows; gr++) {
                    for (b = 0; b < max_sfb; b++) {
                        /*fa_bitstream_putbits(h_bs, p_chn_info->ms_info.ms_used[gr*max_sfb+b], LEN_MASK);*/
                        fa_bitstream_putbits(h_bs, p_chn_info->ms_info.ms_used[gr][b], LEN_MASK);
                    }
                }
            }
        }
        bits += LEN_MASK_PRES;
        if (p_chn_info->ms_info.is_present == 1)
            bits += (num_windows*max_sfb*LEN_MASK);
    }

    /* Write individual_channel_stream elements */
    bits += write_ics(h_bs, s , write_flag, aac_objtype, p_chn_info->common_window);
    bits += write_ics(h_bs, sr, write_flag, aac_objtype, p_chn_info->common_window);

    return bits;
}


static int write_sce(uintptr_t h_bs, aacenc_ctx_t *s, int aac_objtype, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    chn_info_t *p_chn_info = &(s->chn_info);
    int bits = 0;

    if (write_flag) {
        /* write Single Element Channel (SCE) identifier */
        fa_bitstream_putbits(h_bs, ID_SCE, LEN_SE_ID);

        /* write the element identifier tag */
        fa_bitstream_putbits(h_bs, p_chn_info->tag, LEN_TAG);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;

    /* Write an Individual Channel Stream element */
    bits += write_ics(h_bs, s, write_flag, aac_objtype, 0);

    return bits;
}


static int write_lfe(uintptr_t h_bs, aacenc_ctx_t *s, int aac_objtype, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    chn_info_t *p_chn_info = &(s->chn_info);
    int bits = 0;

    if (write_flag) {
        /* write ID_LFE, lfe_element_channel() identifier */
        fa_bitstream_putbits(h_bs, ID_LFE, LEN_SE_ID);

        /* write the element_identifier_tag */
        fa_bitstream_putbits(h_bs, p_chn_info->tag, LEN_TAG);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;

    /* Write an individual_channel_stream element */
    bits += write_ics(h_bs, s, write_flag, aac_objtype, 0);

    return bits;
}


static int find_grouping_bits(aacenc_ctx_t *s)
{
    /* This function inputs the grouping information and outputs the seven bit
    'grouping_bits' field that the AAC decoder expects.  */

    int grouping_bits = 0;
    int tmp[8];
    int i, j;
    int index = 0;

    for (i = 0; i < s->num_window_groups; i++){
        for (j = 0; j < s->window_group_length[i]; j++){
            tmp[index++] = i;
        }
    }

    for (i = 1; i < 8; i++){
        grouping_bits = grouping_bits << 1;
        if (tmp[i] == tmp[i-1]) {
            grouping_bits++;
        }
    }

    return grouping_bits;
}

#if 0
static int write_ltp_predictor_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int i, last_band;
    int bits;
    LtpInfo *ltpInfo = &s->ltpInfo;

    bits = 0;

    if (ltpInfo->global_pred_flag)
    {

        if (write_flag)
            fa_bitstream_putbits(h_bs, 1, 1); /* LTP used */
        bits++;

        switch (s->block_type)
        {
        case ONLY_LONG_BLOCK:
        case LONG_START_BLOCK:
        case LONG_STOP_BLOCK:
            bits += LEN_LTP_LAG;
            bits += LEN_LTP_COEF;
            if (write_flag)
            {
                fa_bitstream_putbits(h_bs, ltpInfo->delay[0], LEN_LTP_LAG);
                fa_bitstream_putbits(h_bs, ltpInfo->weight_idx,  LEN_LTP_COEF);
            }

            last_band = ((s->max_sfb < MAX_LT_PRED_LONG_SFB) ?
                s->max_sfb : MAX_LT_PRED_LONG_SFB);

            bits += last_band;
            if (write_flag)
                for (i = 0; i < last_band; i++)
                    fa_bitstream_putbits(h_bs, ltpInfo->sfb_prediction_used[i], LEN_LTP_LONG_USED);
            break;

        default:
            break;
        }
    }

    return (bits);
}

static int write_predictor_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int bits = 0;

    /* Write global predictor data present */
    short predictorDataPresent = s->pred_global_flag;
    int numBands = FA_MIN(s->max_pred_sfb, s->max_sfb);

    if (write_flag) {
        if (predictorDataPresent) {
            int b;
            if (s->reset_group_number == -1) {
                fa_bitstream_putbits(h_bs, 0, LEN_PRED_RST); /* No prediction reset */
            } else {
                fa_bitstream_putbits(h_bs, 1, LEN_PRED_RST);
                fa_bitstream_putbits(h_bs, (unsigned long)s->reset_group_number,
                    LEN_PRED_RSTGRP);
            }

            for (b=0;b<numBands;b++) {
                fa_bitstream_putbits(h_bs, s->pred_sfb_flag[b], LEN_PRED_ENAB);
            }
        }
    }
    bits += (predictorDataPresent) ?
        (LEN_PRED_RST +
        ((s->reset_group_number)!=-1)*LEN_PRED_RSTGRP +
        numBands*LEN_PRED_ENAB) : 0;

    return bits;
}

#endif


static int write_pulse_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int bits = 0;

    if (write_flag) {
        fa_bitstream_putbits(h_bs, 0, LEN_PULSE_PRES);  /* no pulse_data_present */
    }

    bits += LEN_PULSE_PRES;

    return bits;
}


static int write_tns_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int bits = 0;
    int numWindows;
    int len_tns_nfilt;
    int len_tns_length;
    int len_tns_order;
    int filtNumber;
    int resInBits;
    int bitsToTransmit;
    unsigned long unsignedIndex;
    int w;

    tns_info_t *tns_info = (tns_info_t *)s->h_tns;

    if (write_flag) {
        /*fa_bitstream_putbits(h_bs,tnsInfoPtr->tnsDataPresent,LEN_TNS_PRES);*/
        fa_bitstream_putbits(h_bs, tns_info->tns_data_present ,LEN_TNS_PRES);
    }
    bits += LEN_TNS_PRES;

    /* If TNS is not present, bail */
    if (!tns_info->tns_data_present) {
        return bits;
    }

    /* Set window-dependent TNS parameters */
    if (s->block_type == ONLY_SHORT_BLOCK) {
        numWindows = MAX_SHORT_WINDOWS;
        len_tns_nfilt = LEN_TNS_NFILTS;
        len_tns_length = LEN_TNS_LENGTHS;
        len_tns_order = LEN_TNS_ORDERS;
    }
    else {
        numWindows = 1;
        len_tns_nfilt = LEN_TNS_NFILTL;
        len_tns_length = LEN_TNS_LENGTHL;
        len_tns_order = LEN_TNS_ORDERL;
    }

    /* Write TNS data */
    bits += (numWindows * len_tns_nfilt);
    for (w = 0; w < numWindows; w++) {
        tns_win_t *tns_win = &(tns_info->tns_win[w]);
        int numFilters = tns_win->num_flt;
        if (write_flag) {
            fa_bitstream_putbits(h_bs,numFilters,len_tns_nfilt); /* n_filt[] = 0 */
        }
        if (numFilters) {
            bits += LEN_TNS_COEFF_RES;
            resInBits = tns_win->coef_resolution;
            if (write_flag) {
                fa_bitstream_putbits(h_bs,resInBits-DEF_TNS_RES_OFFSET,LEN_TNS_COEFF_RES);
            }
            bits += numFilters * (len_tns_length+len_tns_order);
            for (filtNumber=0;filtNumber<numFilters;filtNumber++) {
                tns_flt_t * tns_flt = &tns_win->tns_flt[filtNumber];
                int order = tns_flt->order;
                if (write_flag) {
                    fa_bitstream_putbits(h_bs,tns_flt->length,len_tns_length);
                    fa_bitstream_putbits(h_bs,order,len_tns_order);
                }
                if (order) {
                    bits += (LEN_TNS_DIRECTION + LEN_TNS_COMPRESS);
                    if (write_flag) {
                        fa_bitstream_putbits(h_bs,tns_flt->direction,LEN_TNS_DIRECTION);
                        fa_bitstream_putbits(h_bs,tns_flt->coef_compress,LEN_TNS_COMPRESS);
                    }
                    bitsToTransmit = resInBits - tns_flt->coef_compress;
                    bits += order * bitsToTransmit;
                    if (write_flag) {
                        int i;
                        for (i=1;i<=order;i++) {
                            unsignedIndex = (unsigned long) (tns_flt->index[i])&(~(~0<<bitsToTransmit));
                            fa_bitstream_putbits(h_bs,unsignedIndex,bitsToTransmit);
                        }
                    }
                }
            }
        }
    }
    return bits;
}



static int write_gaincontrol_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int bits = 0;

    if (write_flag) {
        fa_bitstream_putbits(h_bs, 0, LEN_GAIN_PRES);
    }

    bits += LEN_GAIN_PRES;

    return bits;
}


static int write_spectral_data(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int i, bits = 0;

    /* set up local pointers to data and len */
    /* data array contains data to be written */
    /* len array contains lengths of data words */
    int* x_quant_code = s->x_quant_code;
    int* x_quant_bits = s->x_quant_bits;

    if (write_flag) {
        for (i = 0; i < s->spectral_count; i++) {
            if (x_quant_bits[i] > 0) {  /* only send out non-zero codebook data */
                fa_bitstream_putbits(h_bs, x_quant_code[i], x_quant_bits[i]); /* write data */
                bits += x_quant_bits[i];
            }
        }
    } else {
        for (i = 0; i < s->spectral_count; i++) {
            bits += x_quant_bits[i];
        }
    }

    return bits;
}


static int write_aac_fillbits(uintptr_t h_bs, aacenc_ctx_t *s, int num_bits, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int numberOfBitsLeft = num_bits;

    /* Need at least (LEN_SE_ID + LEN_F_CNT) bits for a fill_element */
    int minNumberOfBits = LEN_SE_ID + LEN_F_CNT;

    while (numberOfBitsLeft >= minNumberOfBits)
    {
        int numberOfBytes;
        int maxCount;

        if (write_flag) {
            fa_bitstream_putbits(h_bs, ID_FIL, LEN_SE_ID);   /* Write fill_element ID */
        }
        numberOfBitsLeft -= minNumberOfBits;    /* Subtract for ID,count */

        numberOfBytes = (int)(numberOfBitsLeft/LEN_BYTE);
        maxCount = (1<<LEN_F_CNT) - 1;  /* Max count without escaping */

        /* if we have less than maxCount bytes, write them now */
        if (numberOfBytes < maxCount) {
            int i;
            if (write_flag) {
                fa_bitstream_putbits(h_bs, numberOfBytes, LEN_F_CNT);
                for (i = 0; i < numberOfBytes; i++) {
                    fa_bitstream_putbits(h_bs, 0, LEN_BYTE);
                }
            }
            /* otherwise, we need to write an escape count */
        }
        else {
            int maxEscapeCount, maxNumberOfBytes, escCount;
            int i;
            if (write_flag) {
                fa_bitstream_putbits(h_bs, maxCount, LEN_F_CNT);
            }
            maxEscapeCount = (1<<LEN_BYTE) - 1;  /* Max escape count */
            maxNumberOfBytes = maxCount + maxEscapeCount;
            numberOfBytes = (numberOfBytes > maxNumberOfBytes ) ? (maxNumberOfBytes) : (numberOfBytes);
            escCount = numberOfBytes - maxCount;
            if (write_flag) {
                fa_bitstream_putbits(h_bs, escCount, LEN_BYTE);
                for (i = 0; i < numberOfBytes-1; i++) {
                    fa_bitstream_putbits(h_bs, 0, LEN_BYTE);
                }
            }
        }
        numberOfBitsLeft -= LEN_BYTE*numberOfBytes;
    }

    return numberOfBitsLeft;
}

static int write_hufftab_no(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/

    int repeat_counter;
    int bit_count = 0;
    int previous;
    int max, bit_len/*,sfbs*/;
    int gr, sfb;
    int sect_cb_bits = 4;
    int sfb_num;

    /* Set local pointers to coderInfo elements */
    /*max is the escape value = (1<<bit_len)-1*/
    if (s->block_type == ONLY_SHORT_BLOCK){
        max = 7;
        bit_len = 3;
        /*sfb_num = s->sfb_num_short;*/
        sfb_num = s->max_sfb;
    } else {  /* the block_type is a long,start, or stop window */
        max = 31;
        bit_len = 5;
        /*sfb_num = s->sfb_num_long;*/
        sfb_num = s->max_sfb;
    }

    for (gr = 0; gr < s->num_window_groups; gr++) {
        repeat_counter=1;
        previous = s->hufftab_no[gr][0];
        if (write_flag) {
            fa_bitstream_putbits(h_bs, s->hufftab_no[gr][0],sect_cb_bits);
        }
        bit_count += sect_cb_bits;

        for (sfb = 1; sfb < sfb_num; sfb++) {
            if ((s->hufftab_no[gr][sfb] != previous)) {
                if (write_flag) {
                    fa_bitstream_putbits(h_bs, repeat_counter, bit_len);
                }
                bit_count += bit_len;

                if (repeat_counter == max){  /* in case you need to terminate an escape sequence */
                    if (write_flag)
                        fa_bitstream_putbits(h_bs, 0, bit_len);
                    bit_count += bit_len;
                }

                if (write_flag)
                    fa_bitstream_putbits(h_bs, s->hufftab_no[gr][sfb], sect_cb_bits);
                bit_count += sect_cb_bits;
                previous = s->hufftab_no[gr][sfb];
                repeat_counter=1;
            }
            /* if the length of the section is longer than the amount of bits available in */
            /* the bitsream, "max", then start up an escape sequence */
            else if ((s->hufftab_no[gr][sfb] == previous) && (repeat_counter == max)) {
                if (write_flag) {
                    fa_bitstream_putbits(h_bs, repeat_counter, bit_len);
                }
                bit_count += bit_len;
                repeat_counter = 1;
            }
            else {
                repeat_counter++;
            }
        }

        if (write_flag)
            fa_bitstream_putbits(h_bs, repeat_counter, bit_len);
        bit_count += bit_len;

        if (repeat_counter == max) {  /* special case if the last section length is an */
            /* escape sequence */
            if (write_flag)
                fa_bitstream_putbits(h_bs, 0, bit_len);
            bit_count += bit_len;
        }
    }  /* Bottom of group iteration */

    return bit_count;
}



static int write_scalefactor(uintptr_t h_bs, aacenc_ctx_t *s, int write_flag) 
{
    /* this function takes care of counting the number of bits necessary */
    /* to encode the scalefactors.  In addition, if the writeFlag == 1, */
    /* then the scalefactors are written out the bitStream output bit */
    /* stream.  it returns k, the number of bits written to the bitstream*/

    /*unsigned long h_bs = s->h_bitstream;*/
    int gr, sfb;

    int bit_count=0;
    int diff,length,codeword;
    int previous_scale_factor;
    int previous_is_factor;       /* Intensity stereo */
    int sfb_num;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        /*sfb_num = s->sfb_num_short;*/
        sfb_num = s->max_sfb;
    } else {
        /*sfb_num = s->sfb_num_long;*/
        sfb_num = s->max_sfb;
    }

    previous_scale_factor = s->common_scalefac;
    previous_is_factor = 0;

    for (gr = 0; gr < s->num_window_groups; gr++) {
        for (sfb = 0; sfb < sfb_num; sfb++) {
            /* test to see if any codebooks in a group are zero */
            if ((s->hufftab_no[gr][sfb] == INTENSITY_HCB) ||
                (s->hufftab_no[gr][sfb] == INTENSITY_HCB2) ) {
                /* only send scalefactors if using non-zero codebooks */
                diff = s->scalefactor[gr][sfb] - previous_is_factor;
                if ((diff < 60)&&(diff >= -60))
                    length = fa_hufftab12[diff+60][0];
                else 
                    length = 0;
                bit_count += length;
                previous_is_factor = s->scalefactor[gr][sfb];
                if (write_flag) {
                    codeword = fa_hufftab12[diff+60][1];
                    fa_bitstream_putbits(h_bs, codeword, length);
                }
            } else if (s->hufftab_no[gr][sfb]) {
                /* only send scalefactors if using non-zero codebooks */
                diff = s->scalefactor[gr][sfb] - previous_scale_factor;
                if ((diff < 60)&&(diff >= -60))
                    length = fa_hufftab12[diff+60][0];
                else 
                    length = 0;
                bit_count+=length;
                previous_scale_factor = s->scalefactor[gr][sfb];
                if (write_flag) {
                    codeword = fa_hufftab12[diff+60][1];
                    fa_bitstream_putbits(h_bs, codeword, length);
                }
            }
        }
    }

    return bit_count;
}

static int write_bits_for_bytealign(uintptr_t h_bs, aacenc_ctx_t *s, int bits_sofar, int write_flag)
{
    /*unsigned long h_bs = s->h_bitstream;*/
    int len, i,j;

    if (write_flag) {
        len = fa_bitstream_getbits_num(h_bs);
    } else {
        len = bits_sofar;
    }

    j = (8 - (len%8))%8;

    if ((len % 8) == 0) j = 0;
    if (write_flag) {
        for (i = 0; i < j; i++) {
            fa_bitstream_putbits(h_bs, 0, 1);
        }
    }

    return j;
}

