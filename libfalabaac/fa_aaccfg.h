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


  filename: fa_aaccfg.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#ifndef _FA_AACCFG_H
#define _FA_AACCFG_H 

#ifdef __cplusplus 
extern "C"
{ 
#endif  


/*-------------------------- configuration define -----------------------------------*/
//MPEG ID's 
#define MPEG2       1
#define MPEG4       0

//AAC object types 
#define MAIN        1
#define LOW         2
#define SSR         3
#define LTP         4

//channel ID
#define ID_SCE      0
#define ID_CPE      1
#define ID_CCE      2
#define ID_LFE      3
#define ID_DSE      4
#define ID_PCE      5
#define ID_FIL      6
#define ID_END      7

//channels
#define MAX_CHANNELS            64


/*-------------------------- encoding/decoding define -------------------------------*/
//length for bitstream define
#define LEN_SE_ID           3
#define LEN_TAG             4
#define LEN_GLOB_GAIN       8
#define LEN_COM_WIN         1
#define LEN_ICS_RESERV      1
#define LEN_WIN_SEQ         2
#define LEN_WIN_SH          1
#define LEN_MAX_SFBL        6
#define LEN_MAX_SFBS        4
#define LEN_CB              4
#define LEN_SCL_PCM         8
#define LEN_PRED_PRES       1
#define LEN_PRED_RST        1
#define LEN_PRED_RSTGRP     5
#define LEN_PRED_ENAB       1
#define LEN_MASK_PRES       2
#define LEN_MASK            1
#define LEN_PULSE_PRES      1

#define LEN_TNS_PRES        1
#define LEN_TNS_NFILTL      2
#define LEN_TNS_NFILTS      1
#define LEN_TNS_COEFF_RES   1
#define LEN_TNS_LENGTHL     6
#define LEN_TNS_LENGTHS     4
#define LEN_TNS_ORDERL      5
#define LEN_TNS_ORDERS      3
#define LEN_TNS_DIRECTION   1
#define LEN_TNS_COMPRESS    1
#define LEN_GAIN_PRES       1

#define LEN_NEC_NPULSE      2
#define LEN_NEC_ST_SFB      6
#define LEN_NEC_POFF        5
#define LEN_NEC_PAMP        4
#define NUM_NEC_LINES       4
#define NEC_OFFSET_AMP      4

#define LEN_NCC             3
#define LEN_IS_CPE          1
#define LEN_CC_LR           1
#define LEN_CC_DOM          1
#define LEN_CC_SGN          1
#define LEN_CCH_GES         2
#define LEN_CCH_CGP         1
#define LEN_D_CNT           4
#define LEN_D_ESC           12
#define LEN_F_CNT           4
#define LEN_F_ESC           8
#define LEN_BYTE            8
#define LEN_PAD_DATA        8

#define LEN_PC_COMM         8

//scale factor band
#define NSFB_LONG               51
#define NSFB_SHORT              15
#define MAX_SHORT_WINDOWS       8
#define MAX_SCFAC_BANDS         ((NSFB_SHORT+1)*MAX_SHORT_WINDOWS)

//block type define
enum {
    ONLY_LONG_BLOCK  = 0,
    LONG_START_BLOCK = 1,   //long  to short
    ONLY_SHORT_BLOCK = 2,
    LONG_STOP_BLOCK  = 3,   //short to long
};

enum {
    LONG_CODING_BLOCK  = 0,
    SHORT_CODING_BLOCK = 1,
};

#define AAC_FRAME_LEN       1024
#define AAC_BLOCK_LONG_LEN  1024
#define AAC_BLOCK_SHORT_LEN 128
#define AAC_BLOCK_TRANS_LEN 448   // (1024-128)/2=448




typedef struct _aaccfg_t  {
    /* copyright string */
    char *copyright;

    /* MPEG version, 2 or 4 */
    int mpeg_version;

    /* AAC object type */
    int aac_objtype;

    /* mid/side coding */
    int ms_enable;

    /* Use one of the channels as LFE channel */
    int lfe_enable;

    /* Use Temporal Noise Shaping */
    int tns_enable;

    int chn_num;
    /* bitrate / channel of AAC file */
    int bit_rate;

    float qcof;

    int vbr_flag;

    int sample_rate;

    int sample_rate_index;
}aaccfg_t;


#ifdef __cplusplus 
}
#endif  



#endif
