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


  filename: fa_aacms.c 
  version : v1.0.0
  time    : 2012-10-14 14:52 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include "fa_aacms.h"
#include "fa_mdctquant.h"

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif 

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

void fa_aacmsenc(fa_aacenc_ctx_t *f)
{
    int i, chn;
    int chn_num;
    aacenc_ctx_t *s, *sl, *sr;

    chn_num = f->cfg.chn_num;
 
    i = 0;
    chn = 1;
    while (i < chn_num) {
        s = &(f->ctx[i]);

        if (s->chn_info.cpe == 1) {
            chn = 2;
            sl = s;
            sr = &(f->ctx[i+1]);
            sl->chn_info.ms_info.is_present = 0;
            sr->chn_info.ms_info.is_present = 0;

            if (sl->block_type == sr->block_type) {
                sl->chn_info.common_window = 1;
                sr->chn_info.common_window = 1;
                if (sl->block_type == ONLY_SHORT_BLOCK) {
                    fa_mdctline_ms_encode(sl->h_mdctq_short, sr->h_mdctq_short, sl->num_window_groups,
                                          &(sl->chn_info.ms_info), &(sr->chn_info.ms_info));
                } else {
                    fa_mdctline_ms_encode(sl->h_mdctq_long, sr->h_mdctq_long, sl->num_window_groups,
                                          &(sl->chn_info.ms_info), &(sr->chn_info.ms_info));
                }
            } else {
                sl->chn_info.common_window = 0;
                sr->chn_info.common_window = 0;
            }
        } else {
            chn = 1;
            s->chn_info.common_window = 0;
        }
        i += chn;
    }

}



