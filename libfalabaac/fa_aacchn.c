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


  filename: fa_aacchn.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#include <stdio.h>
#include <stdlib.h>

#include "fa_aacchn.h"

/* If LFE present                                                       */
/*  Num channels       # of SCE's       # of CPE's       #of LFE's      */
/*  ============       ==========       ==========       =========      */
/*      1                  1                0               0           */
/*      2                  0                1               0           */
/*      3                  1                1               0           */
/*      4                  1                1               1           */
/*      5                  1                2               0           */
/* For more than 5 channels, use the following elements:                */
/*      2*N                1                2*(N-1)         1           */
/*      2*N+1              1                2*N             0           */
/*                                                                      */
/* Else:                                                                */
/*                                                                      */
/*  Num channels       # of SCE's       # of CPE's       #of LFE's      */
/*  ============       ==========       ==========       =========      */
/*      1                  1                0               0           */
/*      2                  0                1               0           */
/*      3                  1                1               0           */
/*      4                  2                1               0           */
/*      5                  1                2               0           */
/* For more than 5 channels, use the following elements:                */
/*      2*N                2                2*(N-1)         0           */
/*      2*N+1              1                2*N             0           */

void get_aac_chn_info(chn_info_t *chn_info, int nchn, int lfe_enable)
{
    int sce_tag = 0;
    int lfe_tag = 0;
    int cpe_tag = 0;
    int nchn_left = nchn;


    /* First element is sce, except for 2 channel case */
    if (nchn_left != 2) {
        chn_info[nchn-nchn_left].present = 1;
        chn_info[nchn-nchn_left].tag = sce_tag++;
        chn_info[nchn-nchn_left].sce = 1;
        chn_info[nchn-nchn_left].cpe = 0;
        chn_info[nchn-nchn_left].lfe = 0;
        nchn_left--;
    }

    /* Next elements are cpe's */
    while (nchn_left > 1) {
        /* Left channel info */
        chn_info[nchn-nchn_left].present = 1;
        chn_info[nchn-nchn_left].tag = cpe_tag++;
        chn_info[nchn-nchn_left].cpe = 1;
        chn_info[nchn-nchn_left].common_window = 0;
        chn_info[nchn-nchn_left].ch_is_left = 1;
        chn_info[nchn-nchn_left].paired_ch = nchn-nchn_left+1;
        chn_info[nchn-nchn_left].sce = 0;
        chn_info[nchn-nchn_left].lfe = 0;
        nchn_left--;

        /* Right channel info */
        chn_info[nchn-nchn_left].present = 1;
        chn_info[nchn-nchn_left].cpe = 1;
        chn_info[nchn-nchn_left].common_window = 0;
        chn_info[nchn-nchn_left].ch_is_left = 0;
        chn_info[nchn-nchn_left].paired_ch = nchn-nchn_left-1;
        chn_info[nchn-nchn_left].sce = 0;
        chn_info[nchn-nchn_left].lfe = 0;
        nchn_left--;
    }

    /* Is there another channel left ? */
    if (nchn_left) {
        if (lfe_enable) {
            chn_info[nchn-nchn_left].present = 1;
            chn_info[nchn-nchn_left].tag = lfe_tag++;
            chn_info[nchn-nchn_left].sce = 0;
            chn_info[nchn-nchn_left].cpe = 0;
            chn_info[nchn-nchn_left].lfe = 1;
        } else {
            chn_info[nchn-nchn_left].present = 1;
            chn_info[nchn-nchn_left].tag = sce_tag++;
            chn_info[nchn-nchn_left].sce = 1;
            chn_info[nchn-nchn_left].cpe = 0;
            chn_info[nchn-nchn_left].lfe = 0;
        }
        nchn_left--;
    }
}
