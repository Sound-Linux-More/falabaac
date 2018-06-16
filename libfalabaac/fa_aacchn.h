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


  filename: fa_aacchn.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_AACCHN_H
#define _FA_AACCHN_H 

#include "fa_inttypes.h"
#include "fa_aaccfg.h"
#include "fa_mdctquant.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  

typedef struct _chn_info_t{

    int present;

    int tag;

    int ch_is_left;

    int paired_ch;

    int common_window;

    int cpe;

    int sce;

    int lfe;

    ms_info_t ms_info;

}chn_info_t;

void get_aac_chn_info(chn_info_t *chn_info, int nchn, int lfe_enable);


#ifdef __cplusplus 
}
#endif  



#endif
