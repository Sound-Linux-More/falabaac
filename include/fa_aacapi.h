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


  filename: fa_aacapi.h 
  version : v1.0.0
  time    : 2012/11/24 17:58 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/



#ifndef _FA_AACAPI_H
#define _FA_AACAPI_H 

#include "fa_inttypes.h"

#ifdef __cplusplus 
extern "C"
{ 
#endif  


//typedef unsigned uintptr_t;

#define FA_AACENC_MPEG_VER_DEF   1
#define FA_AACENC_OBJ_TYPE_DEF   2

/*
    For you attention:

    1.sample rate only support 32, 44.1, 48kHz(I think enough, high is no useless, low sample rate sound terrible),
      if you want to support more sample rate, complete fa_swbtab and add code in fa_aacenc_init by yourself, very easy

    2.vbr with quality control (0.1 ~ 1.0), vbr mode is recommend for it adjust bitrate according to the frame feature;
      cbr(roughtly cbr, because if want have accurate bit rate can cost more bitrate control loop and will degrade the 
      speed performance), cbr support 16~160 per channel, means that if encode stereo audio, can support (32 ~320kbps)
      default settings is: -q 0.7, band width is 17kHz, roughtly bitrate is 110 ~ 150kbps according to the audio sample feature
      if you want the best quality and wide band width, use -q 1, the band width extend to 22kHz and encoding whole 
      frequency line using psychmodel
                                     
      you can download the EBU test audio file frome the web site below to do sound quality test:
      http://tech.ebu.ch/public-cations/sqamcd

    3.64 chn can support, lfe support (need you test, lfe I didn't test more)

    4.now only support mpeg_version 2(mpeg2aac), sbr and ps not support now, is developing
        MPEG2       1
        MPEG4       0
      and only ADTS format support 

    5.aac_objtype only support MAIN and LC, and ltp of MAIN can not support(useless, very slow and influece quality little) , SSR is not support 
        MAIN        1
        LOW         2
        SSR         3
        LTP         4

    6.ms encode is support, but if you use fast quantize method(according to you speed_level choice), I close ms encode

    7.band_width you can change by yourself using option -w 
    
    8.I give 6 speed level(1~6), you can choose according to your application , 1 is the lowest but very good quality, 6 is fatest but low quality,
      default is 1(strongly recommend), 
      I think level 3 is a good choice if you want fast encoding, the speed is more than 2 times compare to the speed_level 1. 

    9.Summary----
      Want Best quality (default)       : use -l 1  (best quality, very good sound, smoothly and stable)
      Want Normal quality and fast speed: use -l 3  (it is good choise, normal quality and speed you can tolerant)
      Want Fast speed                   : use -l 5  (if is rock music, can not hear abnormal; but if is quiet or piano audio, maybe not good)

*/
uintptr_t fa_aacenc_init(int sample_rate, int bit_rate, int chn_num, float quality, int vbr_flag,
                         int mpeg_version, int aac_objtype, int lfe_enable,
                         float band_width,
                         int speed_level,
                         int time_resolution_first);

void fa_aacenc_uninit(uintptr_t handle);

//WARN: the inlen must be 1024*chn_num*2 (2 means 2 bytes for per sample, so your sample should be 16 bits short type), 
void fa_aacenc_encode(uintptr_t handle, unsigned char *buf_in, int inlen, unsigned char *buf_out, int *outlen);


#ifdef __cplusplus 
}
#endif  



#endif
