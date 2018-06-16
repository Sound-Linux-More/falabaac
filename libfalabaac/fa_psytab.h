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


  filename: fa_psytab.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_PSYTAB_H
#define _FA_PSYTAB_H

#ifdef __cplusplus 
extern "C"
{ 
#endif  


#define FA_PSY_32k_LONG_NUM   66
#define FA_PSY_32k_SHORT_NUM  44 

#define FA_PSY_44k_LONG_NUM   70 
#define FA_PSY_44k_SHORT_NUM  42 

#define FA_PSY_48k_LONG_NUM   69 
#define FA_PSY_48k_SHORT_NUM  42 

extern int   fa_psy_32k_long_wlow[FA_PSY_32k_LONG_NUM+1];
extern float fa_psy_32k_long_barkval[FA_PSY_32k_LONG_NUM];
extern float fa_psy_32k_long_qsthr[FA_PSY_32k_LONG_NUM];
extern int   fa_psy_32k_short_wlow[FA_PSY_32k_SHORT_NUM+1];
extern float fa_psy_32k_short_barkval[FA_PSY_32k_SHORT_NUM];
extern float fa_psy_32k_short_qsthr[FA_PSY_32k_SHORT_NUM];

extern int   fa_psy_44k_long_wlow[FA_PSY_44k_LONG_NUM+1];
extern float fa_psy_44k_long_barkval[FA_PSY_44k_LONG_NUM];
extern float fa_psy_44k_long_qsthr[FA_PSY_44k_LONG_NUM];
extern int   fa_psy_44k_short_wlow[FA_PSY_44k_SHORT_NUM+1];
extern float fa_psy_44k_short_barkval[FA_PSY_44k_SHORT_NUM];
extern float fa_psy_44k_short_qsthr[FA_PSY_44k_SHORT_NUM];

extern int   fa_psy_48k_long_wlow[FA_PSY_48k_LONG_NUM+1];
extern float fa_psy_48k_long_barkval[FA_PSY_48k_LONG_NUM];
extern float fa_psy_48k_long_qsthr[FA_PSY_48k_LONG_NUM];
extern int   fa_psy_48k_short_wlow[FA_PSY_48k_SHORT_NUM+1];
extern float fa_psy_48k_short_barkval[FA_PSY_48k_SHORT_NUM];
extern float fa_psy_48k_short_qsthr[FA_PSY_48k_SHORT_NUM];

#ifdef __cplusplus 
}
#endif  


#endif

