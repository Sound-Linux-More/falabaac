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


  filename: fa_aacquant.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/11/24 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "fa_aaccfg.h"
#include "fa_aacquant.h"
#include "fa_aacpsy.h"
#include "fa_swbtab.h"
#include "fa_mdctquant.h"
#include "fa_aacstream.h"
/*#include "fa_timeprofile.h"*/
#include "fa_fastmath.h"


/*#define USE_PDF_IMPROVE*/

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef FA_ABS 
#define FA_ABS(a)    ( (a) > 0 ? (a) : (-(a)) )
#endif

/*#define SF_MAGIC_NUM  (0.148148148148) //(0.449346777778)*/
#define SF_MAGIC_NUM  (4./27) //((16./9.)*(1./12)) //(0.449346777778)
//#define SF_MAGIC_NUM  ((16./9.)*(1./12)) //(0.449346777778)

static void calculate_start_common_scalefac(fa_aacenc_ctx_t *f)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;

    chn_num = f->cfg.chn_num;

    i = 0;
    chn = 1;
    while (i < chn_num) {
        s = &(f->ctx[i]);
        /*s->chn_info.common_window = 0;*/

        if (s->chn_info.cpe == 1) {
            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            if (sl->chn_info.common_window == 1) {
                float max_mdct_line;
                max_mdct_line = FA_MAX(sl->max_mdct_line, sr->max_mdct_line);
                sl->start_common_scalefac = fa_get_start_common_scalefac(max_mdct_line);
                sr->start_common_scalefac = sl->start_common_scalefac;
            } else {
                sl->start_common_scalefac = fa_get_start_common_scalefac(sl->max_mdct_line);
                sr->start_common_scalefac = fa_get_start_common_scalefac(sr->max_mdct_line);
            }
        } else if (s->chn_info.sce == 1) {
            chn = 1;
            s->start_common_scalefac = fa_get_start_common_scalefac(s->max_mdct_line);
        } else  { //lfe
            chn = 1;
            s->start_common_scalefac = fa_get_start_common_scalefac(s->max_mdct_line);
        }

        i += chn;
    }

}


static void init_quant_change(int outer_loop_count, aacenc_ctx_t *s, int fast)
{
    if (outer_loop_count == 0) {
        if (!fast) {
            s->common_scalefac = s->start_common_scalefac;
            s->quant_change = 64;
        } else {
            //2012-12-01
#if 0 
            if (s->block_type == ONLY_LONG_BLOCK) {
                s->common_scalefac = FA_MAX(s->start_common_scalefac, s->last_common_scalefac);
                s->quant_change = 2;
            } else {
                s->common_scalefac = s->start_common_scalefac;
                s->quant_change = 64;
            }
#else 
            s->common_scalefac = s->start_common_scalefac;
            s->quant_change = 64;
#endif
        }
    } else {
        /*s->common_scalefac = FA_MAX(s->start_common_scalefac, s->last_common_scalefac);*/
        s->quant_change = 2;
    }

}


static int choose_search_step(int delta_bits)
{
	int step;

	if (delta_bits < 128)
		step = 1;
	else if (delta_bits < 256)
		step = 2;
	else if (delta_bits < 512)
		step = 4;
	else if (delta_bits < 768)
		step = 16;
	else if (delta_bits < 1024)
		step = 32;
	else 
		step = 64;

	return step;

}

static void quant_innerloop(fa_aacenc_ctx_t *f, int outer_loop_count, int fast)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;
    int counted_bits;
    int available_bits;
    int available_bits_l, available_bits_r;
    int find_globalgain;
    int head_end_sideinfo_avg;
    int inner_loop_cnt = 0;

    int delta_bits;

    chn_num = f->cfg.chn_num;
    head_end_sideinfo_avg = fa_bits_sideinfo_est(chn_num);

    i = 0;
    chn = 1;
    while (i < chn_num) {
        inner_loop_cnt = 0;
        s = &(f->ctx[i]);

        if (s->chn_info.cpe == 1) {
            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            find_globalgain = 0;
#if 0
            init_quant_change(outer_loop_count, sl);
            sr->common_scalefac = sl->common_scalefac;
#else 
            //2012-12-01
            if (sl->chn_info.common_window == 1) {
                init_quant_change(outer_loop_count, sl, fast);
                sr->common_scalefac = sl->common_scalefac;
            } else {
                init_quant_change(outer_loop_count, sl, fast);
                init_quant_change(outer_loop_count, sr, fast);
            }

#endif
        } else {
            chn = 1;
            find_globalgain = 0;
            init_quant_change(outer_loop_count, s, fast);
        }

        do {
            if (s->chn_info.cpe == 1) {
                if (sl->chn_info.common_window == 1) {
                    if (sl->quant_ok && sr->quant_ok) 
                        break;

                    if (s->block_type == ONLY_SHORT_BLOCK) {
                        fa_mdctline_quant(sl->h_mdctq_short, sl->common_scalefac, sl->x_quant);
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_short, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                        fa_mdctline_quant(sr->h_mdctq_short, sr->common_scalefac, sr->x_quant);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_short, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    } else {
                        fa_mdctline_quant(sl->h_mdctq_long, sl->common_scalefac, sl->x_quant);
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_long, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                        fa_mdctline_quant(sr->h_mdctq_long, sr->common_scalefac, sr->x_quant);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_long, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    }
                    
                    sr->max_sfb = FA_MAX(sr->max_sfb, sl->max_sfb);
                    sl->max_sfb = sr->max_sfb;

                } else {
                    if (sl->quant_ok && sr->quant_ok)
                        break;

                    if (sl->block_type == ONLY_SHORT_BLOCK) {
                        fa_mdctline_quant(sl->h_mdctq_short, sl->common_scalefac, sl->x_quant);
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_short, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                    } else {
                        fa_mdctline_quant(sl->h_mdctq_long, sl->common_scalefac, sl->x_quant);
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_long, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                    }

                    if (sr->block_type == ONLY_SHORT_BLOCK) {
                        fa_mdctline_quant(sr->h_mdctq_short, sr->common_scalefac, sr->x_quant);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_short, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    } else {
                        fa_mdctline_quant(sr->h_mdctq_long, sr->common_scalefac, sr->x_quant);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_long, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    }

                }

                counted_bits  = fa_bits_count(f->h_bitstream, &f->cfg, sl, sr) + head_end_sideinfo_avg*2;
                available_bits_l = get_avaiable_bits(sl->bits_average, sl->bits_more, sl->bits_res_size, sl->bits_res_maxsize);
                available_bits_r = get_avaiable_bits(sr->bits_average, sr->bits_more, sr->bits_res_size, sr->bits_res_maxsize);
                available_bits = available_bits_l + available_bits_r;

                delta_bits = counted_bits - available_bits;
                delta_bits = FA_ABS(delta_bits);

                if (inner_loop_cnt == 0) 
                    sl->quant_change = sr->quant_change = choose_search_step(delta_bits);

                if (counted_bits > available_bits) { 
                    sl->common_scalefac += sl->quant_change;
                    sr->common_scalefac += sr->quant_change;
                    sl->common_scalefac = FA_MIN(sl->common_scalefac, 255);
                    sr->common_scalefac = FA_MIN(sr->common_scalefac, 255);
                } else {
                    sl->common_scalefac -= sl->quant_change;
                    sl->common_scalefac = FA_MAX(sl->common_scalefac, sl->start_common_scalefac);
                    sr->common_scalefac -= sr->quant_change;
                    sr->common_scalefac = FA_MAX(sr->common_scalefac, sl->start_common_scalefac);
                }

                sl->quant_change >>= 1;
                sr->quant_change >>= 1;

                if ((sl->quant_change == 0 && sr->quant_change == 0) && 
                    (counted_bits > available_bits)) {
                   sl->quant_change = 1;
                   sr->quant_change = 1;
                }

                if ((sl->quant_change == 0 && sr->quant_change == 0) 
                    /*|| (delta_bits < 20)*/
                    /*|| (inner_loop_cnt >= 6 && delta_bits < 200)*/
                    || (inner_loop_cnt >= 20)
                    )
                    find_globalgain = 1;
                else 
                    find_globalgain = 0;

            } else if (s->chn_info.sce == 1 || s->chn_info.lfe == 1) {
                chn = 1;
                if (s->quant_ok)
                    break;
                if (s->block_type == ONLY_SHORT_BLOCK) {
                    fa_mdctline_quant(s->h_mdctq_short, s->common_scalefac, s->x_quant);
                    s->spectral_count = fa_mdctline_encode(s->h_mdctq_short, s->x_quant, s->num_window_groups, s->window_group_length, 
                                                           s->hufftab_no, &(s->max_sfb), s->x_quant_code, s->x_quant_bits);
                } else {
                    fa_mdctline_quant(s->h_mdctq_long, s->common_scalefac, s->x_quant);
                    s->spectral_count = fa_mdctline_encode(s->h_mdctq_long, s->x_quant, s->num_window_groups, s->window_group_length, 
                                                           s->hufftab_no, &(s->max_sfb), s->x_quant_code, s->x_quant_bits);
                }


                counted_bits  = fa_bits_count(f->h_bitstream, &f->cfg, s, NULL) + head_end_sideinfo_avg;

                available_bits = get_avaiable_bits(s->bits_average, s->bits_more, s->bits_res_size, s->bits_res_maxsize);

                delta_bits = counted_bits - available_bits;
                delta_bits = FA_ABS(delta_bits);

                if (inner_loop_cnt == 0) 
                    s->quant_change = choose_search_step(delta_bits);


                if (counted_bits > available_bits) {
                    s->common_scalefac += s->quant_change;
                    s->common_scalefac = FA_MIN(s->common_scalefac, 255);
                }
                else {
                    s->common_scalefac -= s->quant_change;
                    s->common_scalefac = FA_MAX(s->common_scalefac, s->start_common_scalefac);
                }

                s->quant_change >>= 1;

                if (s->quant_change == 0 && 
                   counted_bits>available_bits)
                    s->quant_change = 1;
 
                if (s->quant_change == 0)
                    find_globalgain = 1;
                else 
                    find_globalgain = 0;

            } else {
                ; // lfe left
            }

            inner_loop_cnt++;
        } while (find_globalgain == 0);
        /*printf("the %d chn inner loop fast cnt=%d\n", i, inner_loop_cnt);*/
     
        if (s->chn_info.cpe == 1) {
            sl = s;
            sr = &(f->ctx[i+1]);
            sl->used_bits = counted_bits>>1;
            sl->bits_res_size = available_bits_l - sl->used_bits;
            if (sl->bits_res_size < -sl->bits_res_maxsize)
                sl->bits_res_size = -sl->bits_res_maxsize;
            if (sl->bits_res_size >= sl->bits_res_maxsize)
                sl->bits_res_size = sl->bits_res_maxsize;
            sr->used_bits = counted_bits - sl->used_bits;
            sr->bits_res_size = available_bits_r - sr->used_bits;
            if (sr->bits_res_size < -sr->bits_res_maxsize)
                sr->bits_res_size = -sr->bits_res_maxsize;
            if (sr->bits_res_size >= sr->bits_res_maxsize)
                sr->bits_res_size = sr->bits_res_maxsize;

            sl->last_common_scalefac = sl->common_scalefac;
            sr->last_common_scalefac = sl->common_scalefac;
        } else {
            s->used_bits = counted_bits;
            s->bits_res_size = available_bits - counted_bits;
            if (s->bits_res_size >= s->bits_res_maxsize)
                s->bits_res_size = s->bits_res_maxsize;
            s->last_common_scalefac = s->common_scalefac;
        }

        i += chn;
    } 


}

static void quant_outerloop(fa_aacenc_ctx_t *f, int fast)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;
    int quant_ok_cnt;
    int outer_loop_count;
    int outer_loop_count_max;

    chn_num = f->cfg.chn_num;
    switch(f->speed_level) {
        case 1:
            outer_loop_count_max = 70;//40;
            break;
        case 2:
            outer_loop_count_max = 15;//15;//30;//15;
            break;
        case 3:
            outer_loop_count_max = 10;
            break;
        case 4:
            outer_loop_count_max = 1;
            break;
        case 5:
            outer_loop_count_max = 1;
            break;
        case 6:
            outer_loop_count_max = 1;
            break;
        default:
            outer_loop_count_max = 15;
    }

    quant_ok_cnt = 0;
    outer_loop_count = 0;

    do {
        /*scale the mdctline firstly using scalefactor*/
        i = 0;
        chn = 1;
        while (i < chn_num) {
            s = &(f->ctx[i]);

            if (s->chn_info.cpe == 1) {
                chn = 2;
                sl = s;
                sr = &(f->ctx[i+1]);
                if (s->chn_info.common_window == 1) {
                    if (!sl->quant_ok || !sr->quant_ok) {
                        if (s->block_type == ONLY_SHORT_BLOCK) {
                            fa_mdctline_scaled(sl->h_mdctq_short, s->num_window_groups, s->scalefactor);
                            fa_mdctline_scaled(sr->h_mdctq_short, s->num_window_groups, s->scalefactor);
                        } else {
                            fa_mdctline_scaled(sl->h_mdctq_long, s->num_window_groups, s->scalefactor);
                            fa_mdctline_scaled(sr->h_mdctq_long, s->num_window_groups, s->scalefactor);
                        }
                    }
                } else {
#if 0
                    if (!sl->quant_ok || !sr->quant_ok) {
                        if (sl->block_type == ONLY_SHORT_BLOCK)
                            fa_mdctline_scaled(sl->h_mdctq_short, sl->num_window_groups, sl->scalefactor);
                        else 
                            fa_mdctline_scaled(sl->h_mdctq_long, sl->num_window_groups, sl->scalefactor);
                        if (sr->block_type == ONLY_SHORT_BLOCK)
                            fa_mdctline_scaled(sr->h_mdctq_short, sr->num_window_groups, sr->scalefactor);
                        else 
                            fa_mdctline_scaled(sr->h_mdctq_long, sr->num_window_groups, sr->scalefactor);
                    }
#else 
                    //2012-12-01 
                    if (!sl->quant_ok) {
                        if (sl->block_type == ONLY_SHORT_BLOCK)
                            fa_mdctline_scaled(sl->h_mdctq_short, sl->num_window_groups, sl->scalefactor);
                        else 
                            fa_mdctline_scaled(sl->h_mdctq_long, sl->num_window_groups, sl->scalefactor);
                    }

                    if (!sr->quant_ok) {
                        if (sr->block_type == ONLY_SHORT_BLOCK)
                            fa_mdctline_scaled(sr->h_mdctq_short, sr->num_window_groups, sr->scalefactor);
                        else 
                            fa_mdctline_scaled(sr->h_mdctq_long, sr->num_window_groups, sr->scalefactor);
                    }

#endif
                }
            } else if (s->chn_info.sce == 1 || s->chn_info.lfe == 1) {
                chn = 1;
                if (!s->quant_ok) {
                    if (s->block_type == ONLY_SHORT_BLOCK) 
                        fa_mdctline_scaled(s->h_mdctq_short, s->num_window_groups, s->scalefactor);
                    else 
                        fa_mdctline_scaled(s->h_mdctq_long, s->num_window_groups, s->scalefactor);
                }
            } else {
                chn = 1;
            }

            i += chn;
        }

        /*inner loop, search common_scalefac to fit the available_bits*/
        quant_innerloop(f, outer_loop_count, fast);

        /*calculate quant noise */
        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
            if (!s->quant_ok) {
                if (s->block_type == ONLY_SHORT_BLOCK) {
                    fa_calculate_quant_noise(s->h_mdctq_short,
                                             s->num_window_groups, s->window_group_length,
                                             s->common_scalefac, s->scalefactor, 
                                             s->x_quant);
                } else {
                    fa_calculate_quant_noise(s->h_mdctq_long,
                                             s->num_window_groups, s->window_group_length,
                                             s->common_scalefac, s->scalefactor, 
                                             s->x_quant);
                }
            }
        }

        /*search scalefactor to fix quant noise*/
        i = 0;
        chn = 1;
        quant_ok_cnt = 0;
        while (i < chn_num) {
            s = &(f->ctx[i]);

            if (s->chn_info.cpe == 1) {
                chn = 2;
                sl = s;
                sr = &(f->ctx[i+1]);
                if (s->chn_info.common_window == 1) {
                    if (!sl->quant_ok || !sr->quant_ok) {
                        if (s->block_type == ONLY_SHORT_BLOCK) {
                            sl->quant_ok = fa_fix_quant_noise_couple(sl->h_mdctq_short, sr->h_mdctq_short, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     s->num_window_groups, s->window_group_length,
                                                                     sl->scalefactor, sr->scalefactor, 
                                                                     s->x_quant);
                            sr->quant_ok = sl->quant_ok;
                            quant_ok_cnt += sl->quant_ok * 2;
                        } else {
                            sl->quant_ok = fa_fix_quant_noise_couple(sl->h_mdctq_long, sr->h_mdctq_long, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     s->num_window_groups, s->window_group_length,
                                                                     sl->scalefactor, sr->scalefactor,
                                                                     s->x_quant);
                            sr->quant_ok = sl->quant_ok;
                            quant_ok_cnt += sl->quant_ok * 2;
                        }
                    } else {
                        quant_ok_cnt += 2;
                    }
                } else {
#if 0
                    if (!sl->quant_ok || !sr->quant_ok) {
                        if (sl->block_type == ONLY_SHORT_BLOCK) {
                            sl->quant_ok = fa_fix_quant_noise_single(sl->h_mdctq_short, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sl->num_window_groups, sl->window_group_length,
                                                                     sl->scalefactor, 
                                                                     sl->x_quant);
                            quant_ok_cnt += sl->quant_ok;
                        } else {
                            sl->quant_ok = fa_fix_quant_noise_single(sl->h_mdctq_long, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sl->num_window_groups, sl->window_group_length,
                                                                     sl->scalefactor, 
                                                                     sl->x_quant);
                            quant_ok_cnt += sl->quant_ok;
                        }
                        if (sr->block_type == ONLY_SHORT_BLOCK) {
                            sr->quant_ok = fa_fix_quant_noise_single(sr->h_mdctq_short, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sr->num_window_groups, sr->window_group_length,
                                                                     sr->scalefactor, 
                                                                     sr->x_quant);
                            quant_ok_cnt += sr->quant_ok;
                        } else {
                            sr->quant_ok = fa_fix_quant_noise_single(sr->h_mdctq_long, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sr->num_window_groups, sr->window_group_length,
                                                                     sr->scalefactor, 
                                                                     sr->x_quant);
                            quant_ok_cnt += sr->quant_ok;
                        }

                    } else {
                        quant_ok_cnt += 2;
                    }
#else 
                    //2012-12-01
                    if (!sl->quant_ok) {
                        if (sl->block_type == ONLY_SHORT_BLOCK) {
                            sl->quant_ok = fa_fix_quant_noise_single(sl->h_mdctq_short, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sl->num_window_groups, sl->window_group_length,
                                                                     sl->scalefactor, 
                                                                     sl->x_quant);
                            quant_ok_cnt += sl->quant_ok;
                        } else {
                            sl->quant_ok = fa_fix_quant_noise_single(sl->h_mdctq_long, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sl->num_window_groups, sl->window_group_length,
                                                                     sl->scalefactor, 
                                                                     sl->x_quant);
                            quant_ok_cnt += sl->quant_ok;
                        }
                    } else {
                        quant_ok_cnt += 1;
                    }

                    if (!sr->quant_ok) {
                        if (sr->block_type == ONLY_SHORT_BLOCK) {
                            sr->quant_ok = fa_fix_quant_noise_single(sr->h_mdctq_short, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sr->num_window_groups, sr->window_group_length,
                                                                     sr->scalefactor, 
                                                                     sr->x_quant);
                            quant_ok_cnt += sr->quant_ok;
                        } else {
                            sr->quant_ok = fa_fix_quant_noise_single(sr->h_mdctq_long, 
                                                                     outer_loop_count, outer_loop_count_max,
                                                                     sr->num_window_groups, sr->window_group_length,
                                                                     sr->scalefactor, 
                                                                     sr->x_quant);
                            quant_ok_cnt += sr->quant_ok;
                        }
                    } else {
                        quant_ok_cnt += 1;
                    }

#endif
                }
            } else if (s->chn_info.sce == 1 || s->chn_info.lfe == 1) {
                chn = 1;
                if (!s->quant_ok) {
                    if (s->block_type == ONLY_SHORT_BLOCK) {
                        s->quant_ok = fa_fix_quant_noise_single(s->h_mdctq_short, 
                                                                outer_loop_count, outer_loop_count_max,
                                                                s->num_window_groups, s->window_group_length,
                                                                s->scalefactor, 
                                                                s->x_quant);
                        quant_ok_cnt += sr->quant_ok;
                    } else {
                        s->quant_ok = fa_fix_quant_noise_single(s->h_mdctq_long, 
                                                                outer_loop_count, outer_loop_count_max,
                                                                s->num_window_groups, s->window_group_length,
                                                                s->scalefactor, 
                                                                s->x_quant);
                        quant_ok_cnt += s->quant_ok;
                    }
                } else {
                    quant_ok_cnt += 1;
                }
            } else {
                chn = 1;
            }

            i += chn;
        }

        outer_loop_count++;

    } while (quant_ok_cnt < chn_num);

    /*printf("outer loop cnt= %d\n", outer_loop_count);*/
}


void fa_quantize_loop(fa_aacenc_ctx_t *f)
{
    int i;
    int chn_num;
    aacenc_ctx_t *s;

    chn_num = f->cfg.chn_num;

    /*FA_CLOCK_START(6);*/
    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);
        if (s->block_type == ONLY_SHORT_BLOCK) {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_short);
            fa_mdctline_pow34(s->h_mdctq_short);
            memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
        } else {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_long);
            fa_mdctline_pow34(s->h_mdctq_long);
            memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
        }
    }
    /*FA_CLOCK_END(6);*/
    /*FA_CLOCK_COST(6);*/
     
    /*check common_window and start_common_scalefac*/
    calculate_start_common_scalefac(f);

    /*outer loop*/

    /*FA_CLOCK_START(2);*/
    quant_outerloop(f, 0);
    /*FA_CLOCK_END(2);*/
    /*FA_CLOCK_COST(2);*/

#if  0 
    {
        int i,j;

        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
            printf("chn=i, common_scalefac=%d\n", s->common_scalefac);
            for (j = 0; j < 40; j++) {
                printf("sf%d=%d, ", j, s->scalefactor[0][j]);
            }

        }
    
    }

#endif
}


/*this is the fast quantize, maybe is not very right, just for your test */

void  fa_fastquant_calculate_sfb_avgenergy(aacenc_ctx_t *s)
{
    int   k;
    int   i;
    int   last = 0;
    float energy_sum = 0.0;
    float tmp;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        for (k = 0; k < 8; k++) {
            last = 0;
            energy_sum = 0.0;
            for (i = 0; i < 128; i++) {
                tmp = s->mdct_line[k*128+i]; 
                if (tmp) {
                    last = i;
                    energy_sum += tmp * tmp;
                }
            }
            last++;
            s->lastx[k]     = last;
            s->avgenergy[k] = energy_sum / last;
        }
    } else {
        last = 0;
        energy_sum = 0.0;
        for (i = 0; i < 1024; i++) {
            tmp = s->mdct_line[i]; 
            if (tmp) {
                last = i;
                energy_sum += tmp * tmp;
            }
        }
        last++;
        s->lastx[0]     = last;
        s->avgenergy[0] = energy_sum / last;

    }

}


void fa_fastquant_calculate_xmin(aacenc_ctx_t *s, float xmin[8][NUM_SFB_MAX], float qcof)
{
    int k,i,j;

    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int start;
    int end;
    int lastsb;

    float tmp;
    float energy;

    memset(xmin, 0, sizeof(float)*8*NUM_SFB_MAX);

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {

            lastsb = 0;
            for (i = 0; i < swb_num; i++) {
                if (s->lastx[k] > swb_low[i])
                    lastsb = i;
            }

            for (i = 0; i < swb_num; i++) {
                if (i > lastsb) {
                    xmin[k][i] = 0;
                    continue;
                }

                start = swb_low[i];
                end   = swb_high[i] + 1;

                energy = 0.0;
                for (j = start; j < end; j++) {
                    tmp = s->mdct_line[k*128+j];
                    energy += tmp * tmp;
                }

                /*xmin[k][i] = energy/9;*/
                xmin[k][i] = qcof*energy/12;
            }
        }
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;
         
        lastsb = 0;
        for (i = 0; i < swb_num; i++) {
            if (s->lastx[0] > swb_low[i])
                lastsb = i;
        }

        for (i = 0; i < swb_num; i++) {
            if (i > lastsb) {
                xmin[0][i] = 0;
                continue;
            }

            start = swb_low[i];
            end   = swb_high[i]+1;

            energy = 0.0;
            for (j = start; j < end; j++) {
                tmp = s->mdct_line[j];
                energy += tmp * tmp;
            }

            /*xmin[0][i] = energy/7;*/
            xmin[0][i] = qcof*energy/10;
        }
    }

}


void fa_calculate_scalefactor_win(aacenc_ctx_t *s, float xmin[8][NUM_SFB_MAX])
{
    int   k;
    int   i, j;
    float tmp;
    float adjcof = 1; //20; //0.3; //0.5; //4;//1.2;

    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int start;
    int end;

    float sfb_sqrenergy = 0.0;
    float max_ratio = 0.0;
    float gl = 0.0;
    float xmin_sqrenergy_ratio[FA_SWB_NUM_MAX];

    memset(s->scalefactor_win, 0, sizeof(float)*8*FA_SWB_NUM_MAX);

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {
            max_ratio = 0.;
            gl = 0.;
            memset(xmin_sqrenergy_ratio, 0, sizeof(float)*FA_SWB_NUM_MAX);
            for (i = 0; i < swb_num; i++) {
                start = swb_low[i];
                end   = swb_high[i] + 1;

                sfb_sqrenergy = 0.0;
                for (j = start; j < end; j++) {
                    tmp = FA_ABS(s->mdct_line[128*k+j]);
                    sfb_sqrenergy += FA_SQRTF(tmp);
                }
                if (sfb_sqrenergy == 0.) {
                    xmin_sqrenergy_ratio[i] = 0.;
                    /*xmin_sqrenergy_ratio[i] = 20.;*/
                } else {
                    xmin_sqrenergy_ratio[i] = adjcof*xmin[k][i]/(SF_MAGIC_NUM*sfb_sqrenergy);
/*
                    printf("xmin_sqrte_ratio=%f, xmin=%f, sfb_sqrenergy=%f\n", 
                            xmin_sqrenergy_ratio[i], xmin[k][i], sfb_sqrenergy);
*/
                }
                

                if (xmin_sqrenergy_ratio[i] > max_ratio) {
                    max_ratio = xmin_sqrenergy_ratio[i];
                    gl = 8./3. * FA_LOG2(max_ratio);
                }
            }

            for (i = 0; i < swb_num; i++) {
                if (xmin_sqrenergy_ratio[i] == 0)
                    s->scalefactor_win[k][i] = 0;
                else {
                    tmp = gl + 8./3 * FA_LOG2(1./xmin_sqrenergy_ratio[i]);
                    /*if (2*tmp < 15)*/
                        /*tmp = 2*tmp;*/
                    s->scalefactor_win[k][i] = FA_MAX(0, (int)tmp);
                }
            }
        }
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;

        max_ratio = 0.;
        gl = 0.;
        memset(xmin_sqrenergy_ratio, 0, sizeof(float)*FA_SWB_NUM_MAX);

        for (i = 0; i < swb_num; i++) {
            start = swb_low[i];
            end   = swb_high[i] + 1;

            sfb_sqrenergy = 0.0;
            for (j = start; j < end; j++) {
                tmp = FA_ABS(s->mdct_line[j]);
                sfb_sqrenergy += FA_SQRTF(tmp);
                /*if (i == swb_num-1)*/
                    /*printf("sf%d=%f\t", i, s->mdct_line[j]);*/
            }
            if (sfb_sqrenergy == 0)
                xmin_sqrenergy_ratio[i] = 0;
                /*xmin_sqrenergy_ratio[i] = 20;*/
            else 
                /*xmin_sqrenergy_ratio[i] = adjcof*(end-start)*xmin[0][i]/(SF_MAGIC_NUM*sfb_sqrenergy);*/
                xmin_sqrenergy_ratio[i] = adjcof*xmin[0][i]/(SF_MAGIC_NUM*sfb_sqrenergy);

            if (xmin_sqrenergy_ratio[i] > max_ratio) {
                max_ratio = xmin_sqrenergy_ratio[i];
                gl = (8./3.) * FA_LOG2(max_ratio);
            }
        }

        for (i = 0; i < swb_num; i++) {
            if (xmin_sqrenergy_ratio[i] == 0)
                s->scalefactor_win[0][i] = 0;
            else {
                tmp = gl + (8./3) * FA_LOG2(1./xmin_sqrenergy_ratio[i]);
                if (2*tmp < 15)
                    tmp = 2*tmp;
                s->scalefactor_win[0][i] = FA_MAX(0, (int)tmp);
            }
            /*printf("sf%d=%d\t", i, s->scalefactor_win[0][i]);*/
            /*printf("sf%d=%f\t", i, xmin[0][i]/sfb_sqrenergy);*/

        }
    }

    /*printf("\n\n");*/

}


static void calculate_scalefactor(aacenc_ctx_t *s)
{
    int i;
    int gr;
    int win;
    int scalefactor;
 
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);


    if (s->block_type == ONLY_SHORT_BLOCK) {
        for (gr = 0; gr < s->num_window_groups; gr++) {
            for (i = 0; i < fs->sfb_num; i++) {
                for (win = 0; win < s->window_group_length[gr]; win++) {
                    scalefactor = s->scalefactor_win[win][i];
                    /*s->scalefactor[gr][i] = FA_MAX(s->scalefactor[gr][i], scalefactor);*/
                    s->scalefactor[gr][i] = FA_MIN(s->scalefactor[gr][i], scalefactor);
                    s->scalefactor[gr][i] = FA_MIN(50, s->scalefactor[gr][i]);
                }
            }
        }
    } else {
        for (i = 0; i < fl->sfb_num; i++) {
            /*do not change the paremeters below, I tested, I think it is good, if change, maybe wores quality*/
/*#if  1 */
            /*if (i < 40)*/
                /*s->scalefactor[0][i] = FA_MIN(40, 15+s->scalefactor_win[0][i]);*/
            /*else */
                /*s->scalefactor[0][i] = FA_MIN(45, 10+s->scalefactor_win[0][i]);*/
/*#else */
            /*s->scalefactor[0][i] = FA_MIN(20, s->scalefactor_win[0][i]);*/

/*#endif*/

            s->scalefactor[0][i] = s->scalefactor_win[0][i];
        }
        /*printf("\n\n");*/
    }

}

void fa_quantize_fast(fa_aacenc_ctx_t *f)
{
    int i;
    int chn_num;
    aacenc_ctx_t *s;

    chn_num = f->cfg.chn_num;

    /*FA_CLOCK_START(6);*/
    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);
        if (s->block_type == ONLY_SHORT_BLOCK) {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_short);
            fa_mdctline_pow34(s->h_mdctq_short);
            memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
            /*calculate_scalefactor(s);*/    //short type ,this estimate maybe not accurate, so I commented
        } else {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_long);
            fa_mdctline_pow34(s->h_mdctq_long);
            memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
            if (s->block_type == ONLY_LONG_BLOCK)
                calculate_scalefactor(s);
        }
    }
    /*FA_CLOCK_END(6);*/
    /*FA_CLOCK_COST(6);*/

    calculate_start_common_scalefac(f);

    /*FA_CLOCK_START(2);*/
    quant_outerloop(f, 1);
    /*FA_CLOCK_END(2);*/
    /*FA_CLOCK_COST(2);*/

    fa_adjust_scalefactor(f);

#if  0 
    {
        int i,j;

        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
#if 1 
            printf("chn=i, common_scalefac=%d\n", s->common_scalefac);
            for (j = 0; j < 40; j++) {
                printf("sf%d=%d, ", j, s->scalefactor[0][j]);
            }
#else 
                for (j = 0; j < 40; j++) {
                    if (s->scalefactor[0][j] > 255) {
                        printf("chn=i, common_scalefac=%d\n", s->common_scalefac);
                        printf("sf%d=%d, ", j, s->scalefactor[0][j]);
                    }
                }

#endif

        }
    
    }

#endif

}


static void calculate_scalefactor_usespecific(aacenc_ctx_t *s)
{
    int i;
    float miu, miuhalf;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int kmin;
    int kmax;

    float sfb_sqrenergy = 0.0;
    float xmin_sqrenergy_ratio;
    int sf;

    s->common_scalefac = 0;

    if (s->block_type == ONLY_SHORT_BLOCK) {
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;
#if 0
        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];

            miu = fa_get_subband_abspower(s->mdct_line, kmin, kmax)/(kmax-kmin+1);
            miuhalf = fa_get_subband_sqrtpower(s->mdct_line, kmin, kmax)/(kmax-kmin+1);
            sf = fa_estimate_sf(1.0*s->Ti[0][i], kmax-kmin+1, s->qp.beta, s->qp.a2, s->qp.a4, miu, miuhalf);
            s->scalefactor[0][i] = s->common_scalefac - sf;
            s->scalefactor[0][i] = FA_MAX(0, s->scalefactor[0][i]);
        }
#else 
        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];

            miuhalf = fa_get_subband_sqrtpower(s->mdct_line, kmin, kmax);///(kmax-kmin+1);
            if (miuhalf > 0)
                sf = 8./3. * FA_LOG2(27.0*s->Ti[0][i]/(4.0*miuhalf));
            else 
                sf = 0;
            /*printf("sf=%d\n",sf);*/
            s->common_scalefac = FA_MAX(s->common_scalefac, sf);
            /*gl = FA_MAX(gl, sf);*/
            s->scalefactor[0][i] = sf; 
        }

        for (i = 0; i < swb_num; i++) {
            s->scalefactor[0][i] = s->common_scalefac - s->scalefactor[0][i];
            /*s->scalefactor[0][i] = gl - s->scalefactor[0][i];*/
            s->scalefactor[0][i] = FA_MAX(0, s->scalefactor[0][i]);
        }

#endif
    }

}

static void calculate_pow_miu(aacenc_ctx_t *s)
{
    int i, k;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int kmin;
    int kmax;

    float sfb_sqrenergy = 0.0;
    float xmin_sqrenergy_ratio;
    int sf;

    float tmp1, tmp2;
    int width;
    float miu, miuhalf, a2, a4, diff;

    s->common_scalefac = 0;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {
            for (i = 0; i < swb_num; i++) {
                kmin = swb_low[i];
                kmax = swb_high[i];
                fa_get_subband_abssqrtpower(s->mdct_line+k*128, kmin, kmax, &tmp1, &tmp2);
                width = kmax-kmin+1;
                miu = s->miu[k][i] = tmp1/width;
                miuhalf = s->miuhalf[k][i] = tmp2/width;
#if  1 
                a2 = s->qp.a2;
                a4 = s->qp.a4;
#else 
                if (i <= 5) {
                    a2 = 3*s->qp.a2;
                    a4 = 9*s->qp.a4;
                } else {
                    a2 = s->qp.a2;
                    a4 = s->qp.a4;
                }
#endif
                diff = a4*miu - a2*a2*miuhalf*miuhalf; 
                /*assert(diff>=0);*/
                s->pdft[k][i] = width*a2*miuhalf+s->qp.beta*FA_SQRTF(2*width*diff);
            }
        }
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;
        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];
            fa_get_subband_abssqrtpower(s->mdct_line, kmin, kmax, &tmp1, &tmp2);
            width = kmax-kmin+1;
            miu = s->miu[0][i] = tmp1/width;
            miuhalf = s->miuhalf[0][i] = tmp2/width;
            a2 = s->qp.a2;
            a4 = s->qp.a4;
            diff = a4*miu - a2*a2*miuhalf*miuhalf; 
            s->pdft[0][i] = width*a2*miuhalf+s->qp.beta*FA_SQRTF(2*width*diff);
        }
    }

}


static void calculate_pow_miu_improve(aacenc_ctx_t *s)
{
    int i, k;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int kmin;
    int kmax;

    float sfb_sqrenergy = 0.0;
    float xmin_sqrenergy_ratio;
    int sf;

    float tmp1, tmp2, tmp3;
    int width;
    float miu, miuhalf, a2, a4, diff;

    s->common_scalefac = 0;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {
            for (i = 0; i < swb_num; i++) {
                kmin = swb_low[i];
                kmax = swb_high[i];
                fa_get_subband_abssqrtpower_improve(s->mdct_line+k*128, kmin, kmax, &tmp1, &tmp2, &tmp3);
                width = kmax-kmin+1;
                miu = s->miu[k][i] = tmp1/width;
                miuhalf = s->miuhalf[k][i] = tmp2/width;
                s->miu2[k][i] = tmp3/width;
                a2 = s->qp.a2;
                a4 = s->qp.a4;
                diff = a4*miu - a2*a2*miuhalf*miuhalf; 
                /*assert(diff>=0);*/
                s->pdft[k][i] = width*a2*miuhalf+s->qp.beta*FA_SQRTF(2*width*diff);
            }
        }
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;
        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];
            fa_get_subband_abssqrtpower_improve(s->mdct_line, kmin, kmax, &tmp1, &tmp2, &tmp3);
            width = kmax-kmin+1;
            miu = s->miu[0][i] = tmp1/width;
            miuhalf = s->miuhalf[0][i] = tmp2/width;
            s->miu2[0][i] = tmp3/width;
            a2 = s->qp.a2;
            a4 = s->qp.a4;
            diff = a4*miu - a2*a2*miuhalf*miuhalf; 
            s->pdft[0][i] = width*a2*miuhalf+s->qp.beta*FA_SQRTF(2*width*diff);
        }
    }

}


static void calculate_scalefactor_usepdf(aacenc_ctx_t *s, float ti_adj)
{
    int i, k;
    float miu, miuhalf;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int kmin;
    int kmax;

    float sfb_sqrenergy = 0.0;
    float xmin_sqrenergy_ratio;
    int gl;
    int sf;
    int sf_min;
    int scalefactor;

    int gr, win;
    int group_offset;
    float adj;

    s->common_scalefac = 0; //255;//0;
    adj = s->adj;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {
            gl = 0;
            sf_min = 255;
            for (i = 0; i < swb_num; i++) {
                kmin = swb_low[i];
                kmax = swb_high[i];
#ifdef USE_PDF_IMPROVE
                sf = fa_estimate_sf_fast_improve((2.0+adj*3)*s->Ti[k][i], s->pdft[k][i], s->miu2[k][i]);
#else 
                sf = fa_estimate_sf_fast((2.5+adj*3+ti_adj)*s->Ti[k][i], s->pdft[k][i]);
                /*sf = fa_estimate_sf_fast((1.0+ti_adj)*s->Ti[k][i], s->pdft[k][i]);*/
#endif
                /*assert(sf>=0);*/
                gl = FA_MAX(gl, sf);
                sf_min = FA_MIN(sf, sf_min);
                s->scalefactor_win[k][i] = sf; 
            }
#if 0 
            for (i = 0; i < swb_num; i++) {
                s->scalefactor_win[k][i] = gl - s->scalefactor_win[k][i];
                /*printf("sf[%d][%d]=%d\t", k,i,s->scalefactor_win[k][i]);*/
                s->scalefactor_win[k][i] = FA_MAX(s->scalefactor_win[k][i], 0);
                /*s->scalefactor_win[k][i] = FA_MIN(s->scalefactor_win[k][i], 40);*/
                /*s->scalefactor_win[k][i] = FA_MAX(s->scalefactor_win[k][i], s->start_common_scalefac);*/
            }
#endif

            s->common_scalefac = FA_MAX(s->common_scalefac, gl);
            /*s->common_scalefac = 0;//FA_MIN(s->common_scalefac, gl);*/
        }
        /*s->common_scalefac = FA_MAX(s->common_scalefac, s->start_common_scalefac);*/
/*
        if (s->common_scalefac-sf_min > 30) {
            [>printf("gl=%d, sfmin=%d, diff=%d\n", s->common_scalefac, sf_min, s->common_scalefac-sf_min);<]
            s->common_scalefac = sf_min + 29;
        }
*/
#if  1 
        for (k = 0; k < 8; k++) {
            for (i = 0; i < swb_num; i++) {
                int tmp;
                s->scalefactor_win[k][i] = s->common_scalefac - s->scalefactor_win[k][i];
                /*s->scalefactor_win[k][i] = s->scalefactor_win[k][i];*/
                /*printf("sgl=%d, gl=%d, sf[%d][%d]=%d\n", s->start_common_scalefac, s->common_scalefac, k,i,s->scalefactor_win[k][i]);*/
                /*if (s->scalefactor_win[k][i] > 255)*/
                    /*assert(0);*/

                tmp = s->common_scalefac-s->scalefactor_win[k][i];
                if (tmp <= s->start_common_scalefac) {
                    s->scalefactor_win[k][i] = s->common_scalefac - s->start_common_scalefac;
                }


                /*s->scalefactor_win[k][i] = FA_MAX(s->scalefactor_win[k][i], 0);*/
/*
                if (i-1 >= 0) {
                    if (s->scalefactor_win[k][i] - s->scalefactor_win[k][i-1] >= 40 ) 
                        s->scalefactor_win[k][i-1] = s->scalefactor_win[k][i] - 40;
                    if (s->scalefactor_win[k][i-1] - s->scalefactor_win[k][i] >= 40 ) 
                        s->scalefactor_win[k][i] = s->scalefactor_win[k][i-1] - 40;
                }
*/
                /*s->scalefactor_win[k][i] = FA_MIN(s->scalefactor_win[k][i], 30);*/
            }
        }
        /*printf("\n");*/
#endif
        group_offset = 0;
        for (gr = 0; gr < s->num_window_groups; gr++) {
            for (i = 0; i < fs->sfb_num; i++) {
                /*s->scalefactor[gr][i] = 255;*/
                for (win = 0; win < s->window_group_length[gr]; win++) {
                    scalefactor = s->scalefactor_win[group_offset+win][i];
                    s->scalefactor[gr][i] = FA_MAX(s->scalefactor[gr][i], scalefactor);
                    /*printf("sf=%d\n", s->scalefactor[gr][i]);*/
                    /*s->scalefactor[gr][i] = FA_MIN(s->scalefactor[gr][i], 50);*/
                }
            }
            group_offset += s->window_group_length[gr];
        }

        /*s->common_scalefac = 0;*/

    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;

        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];
#ifdef USE_PDF_IMPROVE
            sf = fa_estimate_sf_fast_improve((1.2+adj)*s->Ti[0][i], s->pdft[0][i], s->miu2[0][i]);
#else 
            /*sf = fa_estimate_sf_fast((1.0+adj+ti_adj)*s->Ti[0][i], s->pdft[0][i]);*/
            sf = fa_estimate_sf_fast((1.0+ti_adj)*s->Ti[0][i], s->pdft[0][i]);
#endif
            s->common_scalefac = FA_MAX(s->common_scalefac, sf);
            s->scalefactor[0][i] = sf; 
        }

        for (i = 0; i < swb_num; i++) {
            int tmp;
            s->scalefactor[0][i] = s->common_scalefac - s->scalefactor[0][i];

            tmp = s->common_scalefac-s->scalefactor[0][i];
            if (tmp <= s->start_common_scalefac) {
                s->scalefactor[0][i] = s->common_scalefac - s->start_common_scalefac;
                /*s->scalefactor[0][i] = FA_MIN(s->scalefactor[0][i], 20);*/
                /*s->common_scalefac = s->scalefactor[0][i] + s->start_common_scalefac;*/
            }
/*
            if (i-1 >= 0) {
                if (s->scalefactor_win[0][i] - s->scalefactor_win[0][i-1] >= 20 ) 
                    s->scalefactor_win[0][i-1] = s->scalefactor_win[0][i] - 20;
                if (s->scalefactor_win[0][i-1] - s->scalefactor_win[0][i] >= 20 ) 
                    s->scalefactor_win[0][i] = s->scalefactor_win[0][i-1] - 20;
            }

*/
        }

    }

}

static void init_pdf_para(aacenc_ctx_t *s)
{
    int i, k;
    float miu, miuhalf;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;
    int *swb_low;
    int *swb_high;

    int kmin;
    int kmax;

    float tmp;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        swb_low  = fs->swb_low;
        swb_high = fs->swb_high;

        for (k = 0; k < 8; k++) {
            for (i = 0; i < swb_num; i++) {
                kmin = swb_low[i];
                kmax = swb_high[i];

                tmp = fa_get_subband_power(s->mdct_line+128*k, kmin, kmax);
                s->Px[k][i] = 10*FA_LOG10(tmp);
                s->Tm[k][i] = 10*FA_LOG10(s->xmin[k][i]);
                s->Tm[k][i] = FA_MAX(s->Tm[k][i], 0);
                s->G[k][i]  = s->Px[k][i] - s->Pt_short[i];
                s->G[k][i]  = FA_MAX(s->G[k][i], 0);

                s->Ti[k][i] = s->xmin[k][i];
                s->Ti1[k][i] = 1;
            }
        }
    } else {
        swb_num  = fl->sfb_num;
        swb_low  = fl->swb_low;
        swb_high = fl->swb_high;

        for (i = 0; i < swb_num; i++) {
            kmin = swb_low[i];
            kmax = swb_high[i];

            tmp = fa_get_subband_power(s->mdct_line, kmin, kmax);
            s->Px[0][i] = 10*FA_LOG10(tmp);
            s->Tm[0][i] = 10*FA_LOG10(s->xmin[0][i]);
            s->Tm[0][i] = FA_MAX(s->Tm[0][i], 0);
            s->G[0][i]  = s->Px[0][i] - s->Pt_long[i];
            s->G[0][i]  = FA_MAX(s->G[0][i], 0);

            s->Ti[0][i] = s->xmin[0][i];
            s->Ti1[0][i] = 1;
        }
    }

}

static void adjust_noise_thr(aacenc_ctx_t *s)
{
    int i, k;
    float miu, miuhalf;
    fa_mdctquant_t *fs = (fa_mdctquant_t *)(s->h_mdctq_short);
    fa_mdctquant_t *fl = (fa_mdctquant_t *)(s->h_mdctq_long);

    int swb_num;

    if (s->block_type == ONLY_SHORT_BLOCK) {
        swb_num  = fs->sfb_num;
        for (k = 0; k < 8; k++) {
            float Px[FA_SWB_NUM_MAX];
            float Ti[FA_SWB_NUM_MAX];
            float Tm[FA_SWB_NUM_MAX];
            float Ti1[FA_SWB_NUM_MAX];
            float G[FA_SWB_NUM_MAX];

            for (i = 0; i < swb_num; i++) {
                Px[i] = s->Px[k][i];
                Tm[i] = s->Tm[k][i];
                G[i]   = s->G[k][i]; 

                Ti[i] = 10*FA_LOG10(s->Ti[k][i]);
                Ti[i] = FA_MAX(Ti[i], 0);
                Ti1[i]= 10*FA_LOG10(s->Ti1[k][i]);
                Ti1[i] = FA_MAX(Ti1[i], 0);
            }

            if (s->up)
                fa_adjust_thr(swb_num, Px, Tm, G, Ti, Ti1);
            else {
                for (i = 0; i < swb_num; i++) {
                    Ti[i] -= s->step_down_db; //0.25;
                    Ti[i] = FA_MAX(Ti[i], 0);
                    Ti1[i] -= s->step_down_db; //0.25;
                    Ti1[i] = FA_MAX(Ti1[i], 0);
                }
            }

            for (i = 0; i < swb_num; i++) {
                /*printf("index=%d,Ti=%f, Ti1 = %f\n", i, Ti[i], Ti1[i]);*/
                s->Ti[k][i] = pow(10, 0.1*Ti[i]);
                s->Ti1[k][i] = pow(10, 0.1*Ti1[i]);
                /*printf("index=%d,Ti=%f, Ti1 = %f\n", i, s->Ti[0][i], s->Ti1[0][i]);*/
            }
        }
    } else {
        float Px[FA_SWB_NUM_MAX];
        float Ti[FA_SWB_NUM_MAX];
        float Tm[FA_SWB_NUM_MAX];
        float Ti1[FA_SWB_NUM_MAX];
        float G[FA_SWB_NUM_MAX];

        swb_num  = fl->sfb_num;

        for (i = 0; i < swb_num; i++) {
            Px[i] = s->Px[0][i];
            Tm[i] = s->Tm[0][i];
            G[i]   = s->G[0][i]; 

            Ti[i] = 10*FA_LOG10(s->Ti[0][i]);
            Ti[i] = FA_MAX(Ti[i], 0);
            Ti1[i]= 10*FA_LOG10(s->Ti1[0][i]);
            Ti1[i] = FA_MAX(Ti1[i], 0);
        }

        if (s->up)
            fa_adjust_thr(swb_num, Px, Tm, G, Ti, Ti1);
        else {
            for (i = 0; i < swb_num; i++) {
                Ti[i] -= s->step_down_db; //0.25;
                Ti[i] = FA_MAX(Ti[i], 0);
                Ti1[i] -= s->step_down_db; //0.25;
                Ti1[i] = FA_MAX(Ti1[i], 0);
            }
        }

        for (i = 0; i < swb_num; i++) {
            /*printf("index=%d,Ti=%f, Ti1 = %f\n", i, Ti[i], Ti1[i]);*/
            s->Ti[0][i] = pow(10, 0.1*Ti[i]);
            s->Ti1[0][i] = pow(10, 0.1*Ti1[i]);
            /*printf("index=%d,Ti=%f, Ti1 = %f\n", i, s->Ti[0][i], s->Ti1[0][i]);*/
        }
    }

}


static float choose_stepsize_db(int delta_bits, float cof)
{
	float step;
#if 0

	/*if (delta_bits < 160*cof)*/
	if (delta_bits < 140*cof)
		step = 0.25;
	else if (delta_bits < 300*cof)
		step = 0.4;
	else if (delta_bits < 700*cof)
		step = 1;
	else if (delta_bits < 1000*cof)
		step = 3;
	else if (delta_bits < 1500*cof)
        step = 4;
	else if (delta_bits < 2500*cof)
        step = 6;
	else 
		step = 8;

#else 

    if (delta_bits < 140*cof)
        step = 0.25;
    else if (delta_bits < 300*cof)
        step = 0.4;
    else if (delta_bits < 700*cof)
        step = 0.8;
    else if (delta_bits < 1000*cof)
        step = 2;
    else if (delta_bits < 1500*cof)
        step = 3;
    else if (delta_bits < 2500*cof)
        step = 5;
    else 
        step = 6;

    /*step = 0.25;*/

#endif

	return step;

}

static int mdctline_enc(fa_aacenc_ctx_t *f)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;
    int counted_bits;
    int available_bits;
    int available_bits_l, available_bits_r;
    int find_globalgain;
    int head_end_sideinfo_avg;

    int delta_bits;
    int delta_bits_total;

    int total_bits;

    int bit_thr;

    chn_num = f->cfg.chn_num;
    head_end_sideinfo_avg = fa_bits_sideinfo_est(chn_num);

    i = 0;
    chn = 1;
    delta_bits_total = 0;
    total_bits = 0;

    while (i < chn_num) {
        s = &(f->ctx[i]);
        /*bit_thr = 160.0*s->bit_thr_cof;*/
        bit_thr = 140.0*s->bit_thr_cof;

        if (s->chn_info.cpe == 1) {
            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            find_globalgain = 0;
        } else {
            chn = 1;
            find_globalgain = 0;
        }

        do {
            if (s->chn_info.cpe == 1) {
                if (sl->chn_info.common_window == 1) {
                    if (sl->quant_ok && sr->quant_ok)
                        break;

                    if (s->block_type == ONLY_SHORT_BLOCK) {
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_short, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_short, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    } else {
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_long, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_long, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    }
                    
                    sr->max_sfb = FA_MAX(sr->max_sfb, sl->max_sfb);
                    sl->max_sfb = sr->max_sfb;

                } else {
                    if (sl->quant_ok && sr->quant_ok)
                        break;

                    if (sl->block_type == ONLY_SHORT_BLOCK) {
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_short, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                    } else {
                        sl->spectral_count = fa_mdctline_encode(sl->h_mdctq_long, sl->x_quant, sl->num_window_groups, sl->window_group_length, 
                                                                sl->hufftab_no, &(sl->max_sfb), sl->x_quant_code, sl->x_quant_bits);
                    }

                    if (sr->block_type == ONLY_SHORT_BLOCK) {
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_short, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    } else {
                        sr->spectral_count = fa_mdctline_encode(sr->h_mdctq_long, sr->x_quant, sr->num_window_groups, sr->window_group_length, 
                                                                sr->hufftab_no, &(sr->max_sfb), sr->x_quant_code, sr->x_quant_bits);
                    }

                }

                counted_bits  = fa_bits_count(f->h_bitstream, &f->cfg, sl, sr) + head_end_sideinfo_avg*2;
                available_bits_l = get_avaiable_bits(sl->bits_average, sl->bits_more, sl->bits_res_size, sl->bits_res_maxsize);
                available_bits_r = get_avaiable_bits(sr->bits_average, sr->bits_more, sr->bits_res_size, sr->bits_res_maxsize);
                available_bits = available_bits_l + available_bits_r;

                delta_bits = counted_bits - available_bits;
                delta_bits = FA_ABS(delta_bits);

#if 0 
                if (counted_bits < available_bits) {
                    sl->quant_ok = sr->quant_ok = 1;
                } else {
                    sl->up = sr->up = 1;
                }
#else 
                if (delta_bits < bit_thr) {
                    sl->quant_ok = sr->quant_ok = 1;
                    /*printf("delta bits=%d\n", delta_bits);*/
                } else {
                    if (counted_bits > available_bits) {
                        sl->up = sr->up = 1;
                    } else  {
                        sl->up = sr->up = 0;
                        /*printf("delta bits=%d\n", delta_bits);*/

                        if (sl->block_type == ONLY_SHORT_BLOCK)
                            sl->step_down_db = sr->step_down_db = 0.2;
                            /*sl->step_down_db = sr->step_down_db = choose_stepsize_db(delta_bits, s->bit_thr_cof);*/
                        else
                            sl->step_down_db = sr->step_down_db = choose_stepsize_db(delta_bits, s->bit_thr_cof);
                    }
                }
#endif

            } else if (s->chn_info.sce == 1 || s->chn_info.lfe == 1) {
                chn = 1;
                if (s->quant_ok)
                    break;

                if (s->block_type == ONLY_SHORT_BLOCK) {
                    s->spectral_count = fa_mdctline_encode(s->h_mdctq_short, s->x_quant, s->num_window_groups, s->window_group_length, 
                                                           s->hufftab_no, &(s->max_sfb), s->x_quant_code, s->x_quant_bits);
                } else {
                    s->spectral_count = fa_mdctline_encode(s->h_mdctq_long, s->x_quant, s->num_window_groups, s->window_group_length, 
                                                           s->hufftab_no, &(s->max_sfb), s->x_quant_code, s->x_quant_bits);
                }

                counted_bits  = fa_bits_count(f->h_bitstream, &f->cfg, s, NULL) + head_end_sideinfo_avg;
                available_bits = get_avaiable_bits(s->bits_average, s->bits_more, s->bits_res_size, s->bits_res_maxsize);

                delta_bits = counted_bits - available_bits;
                delta_bits = FA_ABS(delta_bits);
                if (counted_bits < available_bits) {
                    s->quant_ok = 1;
                    /*printf("delta bits=%d\n", delta_bits);*/
                }

            } else {
                ; // lfe left
            }

        } while (0);
        /*printf("the %d chn inner loop fast cnt=%d\n", i, inner_loop_cnt);*/
#if 1         
        if (s->chn_info.cpe == 1) {
            if (sl->quant_ok == 1 && sr->quant_ok == 1) {
                sl = s;
                sr = &(f->ctx[i+1]);
                sl->used_bits = counted_bits>>1;
                sl->bits_res_size = available_bits_l - sl->used_bits;
                if (sl->bits_res_size < -sl->bits_res_maxsize)
                    sl->bits_res_size = -sl->bits_res_maxsize;
                if (sl->bits_res_size >= sl->bits_res_maxsize)
                    sl->bits_res_size = sl->bits_res_maxsize;
                sr->used_bits = counted_bits - sl->used_bits;
                sr->bits_res_size = available_bits_r - sr->used_bits;
                if (sr->bits_res_size < -sr->bits_res_maxsize)
                    sr->bits_res_size = -sr->bits_res_maxsize;
                if (sr->bits_res_size >= sr->bits_res_maxsize)
                    sr->bits_res_size = sr->bits_res_maxsize;

                sl->last_common_scalefac = sl->common_scalefac;
                sr->last_common_scalefac = sl->common_scalefac;
                /*printf("-----update res, sl->bits_res_size=%d\n", sl->bits_res_size);*/
            }
        } else {
            if (s->quant_ok == 1) {
                s->used_bits = counted_bits;
                s->bits_res_size = available_bits - counted_bits;
                if (s->bits_res_size >= s->bits_res_maxsize)
                    s->bits_res_size = s->bits_res_maxsize;
                s->last_common_scalefac = s->common_scalefac;
            }
        }
#endif

        i += chn;
        total_bits += counted_bits;
    } 

    return total_bits;

}


void fa_adjust_scalefactor(fa_aacenc_ctx_t *f)
{
    int i;
    int chn_num;
    aacenc_ctx_t *s;

    chn_num = f->cfg.chn_num;

    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);
        if (s->block_type == ONLY_SHORT_BLOCK) {
            fa_balance_energe(s->h_mdctq_short,
                    s->num_window_groups, s->window_group_length,
                    s->common_scalefac, s->scalefactor, 
                    s->x_quant);
        } else {
            fa_balance_energe(s->h_mdctq_long,
                    s->num_window_groups, s->window_group_length,
                    s->common_scalefac, s->scalefactor, 
                    s->x_quant);
        }
    }

}


void fa_quantize_best(fa_aacenc_ctx_t *f)
{
    int i;
    int j;
    int chn_num;
    aacenc_ctx_t *s;
    int quant_ok_cnt;
    int max_loop_cnt;
    int cur_cnt;
    int gl_adj;
    float ti_adj;
    int res_maxsize;
    int cur_bits;
    int vbr_flag;

    chn_num = f->cfg.chn_num;
    vbr_flag = f->cfg.vbr_flag;

    for (i = 0; i < chn_num; i++) {
        s = &(f->ctx[i]);
        init_pdf_para(s);
        if (s->block_type == ONLY_SHORT_BLOCK) {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_short);
            fa_mdctline_pow34(s->h_mdctq_short);
        } else {
            s->max_mdct_line = fa_mdctline_getmax(s->h_mdctq_long);
            fa_mdctline_pow34(s->h_mdctq_long);
        }
#ifdef USE_PDF_IMPROVE
        calculate_pow_miu_improve(s);
#else 
        calculate_pow_miu(s);
#endif
    }
    res_maxsize = s->bits_res_maxsize;

    calculate_start_common_scalefac(f);

    max_loop_cnt = 10; //4;//7;
    cur_cnt = 0;
    gl_adj = 0;
    ti_adj = 0.0;
    cur_bits = 0;

    while (1) {
        cur_cnt++;

        if (cur_cnt > max_loop_cnt) {
            if (cur_bits > res_maxsize) {
                gl_adj++;
                ti_adj += 0.2;
                if (cur_cnt > 20)
                    gl_adj++;
            } else 
                break;
        } 

        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);

            /*s->quant_ok = 0;*/
            if (s->block_type == ONLY_SHORT_BLOCK) {
                memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
                calculate_scalefactor_usepdf(s, ti_adj);

                s->common_scalefac += gl_adj;
                fa_mdctline_quantdirect(s->h_mdctq_short, 
                        s->common_scalefac,
                        s->num_window_groups, s->scalefactor,
                        s->x_quant);

            } else {
                memset(s->scalefactor, 0, 8*FA_SWB_NUM_MAX*sizeof(int));
                calculate_scalefactor_usepdf(s, ti_adj);
                /*calculate_scalefactor_usespecific(s);*/

                s->common_scalefac += gl_adj;
                fa_mdctline_quantdirect(s->h_mdctq_long, 
                        s->common_scalefac,
                        s->num_window_groups, s->scalefactor,
                        s->x_quant);

            }
        }

        fa_adjust_scalefactor(f);
        cur_bits = mdctline_enc(f);

        /*if vbr mode, we break */
        if (vbr_flag) {
            if (cur_bits < res_maxsize)
                break;
        }

        quant_ok_cnt = 0;
        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
            quant_ok_cnt += s->quant_ok;
            /*printf("qcnt = %d\n", quant_ok_cnt);*/
        }

        if (quant_ok_cnt >= chn_num)
            break;

        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
            /*printf("adj direction sup=%d\n", s->up);*/
            adjust_noise_thr(s);
        }
    }
    /*printf("loop cnt = %d\n", cur_cnt);*/


#if  0 
    {
        int i,j;

        for (i = 0; i < chn_num; i++) {
            s = &(f->ctx[i]);
#if 1 
            printf("chn=i, common_scalefac=%d\n", s->common_scalefac);
            for (j = 0; j < 40; j++) {
                printf("sf%d=%d, ", j, s->scalefactor[0][j]);
            }
#else 
                for (j = 0; j < 40; j++) {
                    if (s->scalefactor[0][j] > 255) {
                        printf("chn=i, common_scalefac=%d\n", s->common_scalefac);
                        printf("sf%d=%d, ", j, s->scalefactor[0][j]);
                    }
                }

#endif

        }
    
    }

#endif

}





