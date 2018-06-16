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


  filename: fa_mdctquant.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_MDCTQUANT_H
#define _FA_MDCTQUANT_H

#include "fa_inttypes.h"
#include "fa_swbtab.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


#define NUM_WINDOW_GROUPS_MAX 8
#define NUM_WINDOWS_MAX       8
#define NUM_SFB_MAX           FA_SWB_NUM_MAX

#define NUM_MDCT_LINE_MAX     1024

#define SF_OFFSET             100 //105 //100


typedef struct _ms_info_t{

    int is_present;

    //int ms_used[MAX_SCFAC_BANDS];
    int ms_used[8][FA_SWB_NUM_MAX];

}ms_info_t;

typedef struct _fa_mdctquant_t {

    int   block_type_cof;
    int   mdct_line_num;

    float mdct_line[NUM_MDCT_LINE_MAX];
    float xr_pow[NUM_MDCT_LINE_MAX];
    float mdct_scaled[NUM_MDCT_LINE_MAX];

    float xmin[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX][NUM_WINDOWS_MAX];
    float error_energy[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX][NUM_WINDOWS_MAX];

    int   sfb_num;
    int   swb_low[FA_SWB_NUM_MAX+1];
    int   swb_high[FA_SWB_NUM_MAX];
    int   sfb_low[NUM_WINDOW_GROUPS_MAX][FA_SWB_NUM_MAX+1];
    int   sfb_high[NUM_WINDOW_GROUPS_MAX][FA_SWB_NUM_MAX];

}fa_mdctquant_t;


void fa_mdctquant_rom_init();

uintptr_t fa_mdctquant_init(int mdct_line_num, int sfb_num, int *swb_low, int block_type_cof);
void      fa_mdctquant_uninit(uintptr_t handle);

float fa_mdctline_getmax(uintptr_t handle);
int fa_get_start_common_scalefac(float max_mdct_line);
void fa_mdctline_pow34(uintptr_t handle);
void fa_mdctline_scaled(uintptr_t handle,
                        int num_window_groups, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX]);

void fa_mdctline_quant(uintptr_t handle, 
                       int common_scalefac, int *x_quant);

void fa_mdctline_quantdirect(uintptr_t handle, 
                             int common_scalefac,
                             int num_window_groups, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX],
                             int *x_quant);

int fa_mdctline_get_sfbnum(uintptr_t handle);

int fa_mdctline_iquantize(uintptr_t handle, 
                          int num_window_groups, int *window_group_length,
                          int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                          int *x_quant);
void fa_xmin_sfb_arrange(uintptr_t handle, float xmin_swb[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX],
                         int num_window_groups, int *window_group_length);

void fa_mdctline_sfb_arrange(uintptr_t handle, float *mdct_line_swb, 
                             int num_window_groups, int *window_group_length);

void fa_mdctline_sfb_iarrange(uintptr_t handle, float *mdct_line_swb, int *mdct_line_sig,
                              int num_window_groups, int *window_group_length);

void fa_calculate_quant_noise(uintptr_t handle,
                             int num_window_groups, int *window_group_length,
                             int common_scalefac, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                             int *x_quant);

int  fa_fix_quant_noise_single(uintptr_t handle, 
                               int outer_loop_count, int outer_loop_count_max,
                               int num_window_groups, int *window_group_length,
                               int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int *x_quant);

int  fa_fix_quant_noise_couple(uintptr_t handle1, uintptr_t handle2, 
                               int outer_loop_count, int outer_loop_count_max,
                               int num_window_groups, int *window_group_length,
                               int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int scalefactor1[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                               int *x_quant);

int  fa_mdctline_encode(uintptr_t handle, int *x_quant, int num_window_groups, int *window_group_length, 
                        int quant_hufftab_no[8][FA_SWB_NUM_MAX], 
                        int *max_sfb, int *x_quant_code, int *x_quant_bits);

void fa_mdctline_ms_encode(uintptr_t hl, uintptr_t hr, int num_window_groups,
                           ms_info_t *ms_l, ms_info_t *ms_r);

void fa_balance_energe(uintptr_t handle,
                       int num_window_groups, int *window_group_length,
                       int common_scalefac, int scalefactor[NUM_WINDOW_GROUPS_MAX][NUM_SFB_MAX], 
                       int *x_quant);

#ifdef __cplusplus 
}
#endif  


#endif
