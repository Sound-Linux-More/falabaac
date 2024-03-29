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


  filename: fa_psychomodel1.h
  version : 2.1.0.229
  time    : 2019/07/14
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: https://github.com/Sound-Linux-More/falabaac
*/

#ifndef _FA_PSYCHOMODEL_H
#define _FA_PSYCHOMODEL_H

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef WIN32
    typedef unsigned int uintptr_t;
#else
    typedef unsigned long long int uintptr_t;
#endif

#define CBANDS_NUM        25

    void fa_psychomodel1_rom_init();

    uintptr_t fa_psychomodel1_init(int fs, int fft_len);
    void fa_psychomodel1_uninit(uintptr_t handle);
    void fa_psy_global_threshold(uintptr_t handle, float *fft_buf, float *gth);
    void fa_psy_global_threshold_usemdct(uintptr_t handle, float *mdct_buf, float *gth);


#ifdef __cplusplus
}
#endif

#endif //_FA_PSYCHOMODEL_H//
