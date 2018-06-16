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


  filename: fa_aacenc.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_AACENC_H
#define _FA_AACENC_H 

#include "fa_inttypes.h"
#include "fa_aacapi.h"
#include "fa_aaccfg.h"
#include "fa_aacchn.h"
#include "fa_swbtab.h"
#include "fa_quantpdf.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


typedef struct _aacenc_ctx_t{

    //chn map
    chn_info_t  chn_info;

    uintptr_t h_blockctrl;

    //psy and block type var
    uintptr_t h_aacpsy;
    float pe;
    float var_max_prev;
    int   psy_enable;
    int   block_type;
    int   window_shape;
    int   num_window_groups;
    int   window_group_length[8];

    int   time_resolution_first;


    //sfb var 
    int   sfb_num_long;
    int   sfb_num_short;
    int   max_sfb;
    int   sfb_offset[250];
    int   lastx[8];
    float avgenergy[8];

    //aac filterbank analysis and mdct process var
    uintptr_t h_aac_analysis;
    uintptr_t h_mdctq_long, h_mdctq_short;
    float max_mdct_line;
    float mdct_line[2*AAC_FRAME_LEN];
    int   cutoff_line_long;
    int   cutoff_sfb_long;
    int   cutoff_line_short;
    int   cutoff_sfb_short;

    //tns handle
    uintptr_t h_tns;
    int   tns_active;

    //aac quantize var
    int   spectral_count;
    int   scalefactor_win[8][FA_SWB_NUM_MAX];  // 8 is the max number of window
    int   scalefactor[8][FA_SWB_NUM_MAX];      // 8 is the max number of group
    int   start_common_scalefac;
    int   last_common_scalefac;
    int   common_scalefac;
    int   quant_change;
    int   x_quant[1024];
    int   mdct_line_sign[1024];
    int   quant_ok;

    float xmin[8][FA_SWB_NUM_MAX];
    int   maxscale_win[8][FA_SWB_NUM_MAX];

    //para for new quant method probility density function quantize method (quant pdf)
    float miu[8][FA_SWB_NUM_MAX];
    float miuhalf[8][FA_SWB_NUM_MAX];
    float miu2[8][FA_SWB_NUM_MAX];
    float pdft[8][FA_SWB_NUM_MAX];
    float Px[8][FA_SWB_NUM_MAX];
    float Tm[8][FA_SWB_NUM_MAX];
    float Ti[8][FA_SWB_NUM_MAX];
    float Ti1[8][FA_SWB_NUM_MAX];
    float G[8][FA_SWB_NUM_MAX];
    float adj;
    int   *Pt_long;
    int   *Pt_short;
    fa_qpdf_para_t qp;
    int   up;
    float step_down_db;
    float bit_thr_cof;

    //huffman var
    int   hufftab_no[8][FA_SWB_NUM_MAX];
    int   x_quant_code[5*1024];
    int   x_quant_bits[5*1024];

    //bitstream var
    unsigned char *res_buf;
    int   used_bits;
    int   bits_alloc;
    int   bits_average;
    int   bits_more;
    int   bits_res_maxsize;
    int   bits_res_size;

}aacenc_ctx_t;

enum {
    BLOCKSWITCH_PSY = 0,
    BLOCKSWITCH_VAR,
};

enum {
    QUANTIZE_LOOP = 0,
    QUANTIZE_FAST,
    QUANTIZE_BEST,
};

enum {
    PSYCH1 = 0,
    PSYCH2,
};

typedef struct _fa_aacenc_ctx_t{

    int speed_level;

    float *sample;

    int block_switch_en;
    int psy_enable;
    int psy_model;

    aaccfg_t cfg;

    aacenc_ctx_t ctx[MAX_CHANNELS];

    float band_width;

    uintptr_t h_bitstream;
    int used_bytes;

    int  blockswitch_method;
    int  quantize_method;
    int  (* do_blockswitch)(aacenc_ctx_t *s);
    void (* do_quantize)(struct _fa_aacenc_ctx_t * f);

}fa_aacenc_ctx_t;

#ifdef __cplusplus 
}
#endif  

#endif
