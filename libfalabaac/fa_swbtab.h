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


  filename: fa_swbtab.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_SWBTAB_H
#define _FA_SWBTAB_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  


#define FA_SWB_NUM_MAX       51

#define FA_SWB_32k_LONG_NUM  51
#define FA_SWB_32k_SHORT_NUM 14

#define FA_SWB_44k_LONG_NUM  49 
#define FA_SWB_44k_SHORT_NUM 14

#define FA_SWB_48k_LONG_NUM  49 
#define FA_SWB_48k_SHORT_NUM 14

extern int  fa_swb_32k_long_offset[FA_SWB_32k_LONG_NUM+1];
extern int  fa_swb_32k_short_offset[FA_SWB_32k_SHORT_NUM+1];
extern int  fa_swb_44k_long_offset[FA_SWB_44k_LONG_NUM+1];
extern int  fa_swb_44k_short_offset[FA_SWB_44k_SHORT_NUM+1];
extern int  fa_swb_48k_long_offset[FA_SWB_48k_LONG_NUM+1];
extern int  fa_swb_48k_short_offset[FA_SWB_48k_SHORT_NUM+1];

#ifdef __cplusplus 
}
#endif  


#endif
