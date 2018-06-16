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


  filename: fa_aacfilterbank.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "fa_aaccfg.h"
#include "fa_aacpsy.h"
#include "fa_aacfilterbank.h"
#include "fa_aacstream.h"
#include "fa_mdct.h"

uintptr_t fa_aacfilterbank_init()
{
    int   i;
    float *sin_win_long;
    float *kbd_win_long; 
    float *sin_win_short;
    float *kbd_win_short;

    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)malloc(sizeof(fa_aacfilterbank_t));
    memset(f, 0, sizeof(fa_aacfilterbank_t));

    /*initial the long and short block window*/
    sin_win_long  = (float *)malloc(sizeof(float)*2*AAC_BLOCK_LONG_LEN);
    kbd_win_long  = (float *)malloc(sizeof(float)*2*AAC_BLOCK_LONG_LEN);
    sin_win_short = (float *)malloc(sizeof(float)*2*AAC_BLOCK_SHORT_LEN);
    kbd_win_short = (float *)malloc(sizeof(float)*2*AAC_BLOCK_SHORT_LEN);
    memset(sin_win_long , 0, sizeof(float)*2*AAC_BLOCK_LONG_LEN);
    memset(kbd_win_long , 0, sizeof(float)*2*AAC_BLOCK_LONG_LEN);
    memset(sin_win_short, 0, sizeof(float)*2*AAC_BLOCK_SHORT_LEN);
    memset(kbd_win_short, 0, sizeof(float)*2*AAC_BLOCK_SHORT_LEN);

    fa_mdct_sine(sin_win_long , 2*AAC_BLOCK_LONG_LEN);
    /*fa_mdct_kbd(kbd_win_long  , 2*AAC_BLOCK_LONG_LEN , 4);*/
    fa_mdct_kbd(kbd_win_long  , 2*AAC_BLOCK_LONG_LEN , 6);
    fa_mdct_sine(sin_win_short, 2*AAC_BLOCK_SHORT_LEN);
    /*fa_mdct_kbd(kbd_win_short , 2*AAC_BLOCK_SHORT_LEN, 6);*/
    fa_mdct_kbd(kbd_win_short , 2*AAC_BLOCK_SHORT_LEN, 7);

    for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) {
        f->sin_win_long_left[i]   = sin_win_long[i];
        f->sin_win_long_right[i]  = sin_win_long[i+AAC_BLOCK_LONG_LEN];
        f->kbd_win_long_left[i]   = kbd_win_long[i];
        f->kbd_win_long_right[i]  = kbd_win_long[i+AAC_BLOCK_LONG_LEN];
    }

    for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++) {
        f->sin_win_short_left[i]  = sin_win_short[i];
        f->sin_win_short_right[i] = sin_win_short[i+AAC_BLOCK_SHORT_LEN];
        f->kbd_win_short_left[i]  = kbd_win_short[i];
        f->kbd_win_short_right[i] = kbd_win_short[i+AAC_BLOCK_SHORT_LEN];
    }

    free(sin_win_long);
    free(kbd_win_long);
    free(sin_win_short);
    free(kbd_win_short);

    memset(f->x_buf, 0, sizeof(float)*2*AAC_FRAME_LEN);

    memset(f->mdct_long_buf , 0, sizeof(float)*2*AAC_BLOCK_LONG_LEN);
    memset(f->mdct_short_buf, 0, sizeof(float)*2*AAC_BLOCK_SHORT_LEN);
    f->h_mdct_long  = fa_mdct_init(MDCT_FFT4, 2*AAC_BLOCK_LONG_LEN);
    f->h_mdct_short = fa_mdct_init(MDCT_FFT4, 2*AAC_BLOCK_SHORT_LEN);

    return (uintptr_t)f;
}

void fa_aacfilterbank_uninit(uintptr_t handle)
{
    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)handle;

    if (f) {
        fa_mdct_uninit(f->h_mdct_long);
        fa_mdct_uninit(f->h_mdct_short);

        free(f);
        f = NULL;
    }
}

#if  0 
/*used in encode, kbd is used for short block, sine is used for long block*/
void fa_aacfilterbank_analysis(uintptr_t handle, int block_type, int *window_shape, 
                               float *x, float *mdct_line)
{
    int i,k;
    int offset;
    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)handle;

    float *win_left, *win_right;

    /*update x_buf, 50% overlap, copy the remain half data to the beginning position*/
    for (i = 0; i < AAC_FRAME_LEN; i++) 
        f->x_buf[i] = f->x_buf[i+AAC_FRAME_LEN];
    for (i = 0; i < AAC_FRAME_LEN; i++)
        f->x_buf[i+AAC_FRAME_LEN] = x[i];

    /*window shape the input x*/
    switch (*window_shape) {
        case SINE_WINDOW:
            if (block_type == ONLY_LONG_BLOCK || block_type == LONG_START_BLOCK)
                win_left = f->sin_win_long_left;
            else 
                win_left = f->sin_win_short_left;
            break;
        case KBD_WINDOW:
            if (block_type == ONLY_LONG_BLOCK || block_type == LONG_START_BLOCK)
                win_left = f->kbd_win_long_left;
            else 
                win_left = f->kbd_win_short_left;
            break;
    }

    switch (block_type) {
        case ONLY_LONG_BLOCK:
            win_right = f->sin_win_long_right;
            *window_shape = SINE_WINDOW;
            break;
        case LONG_START_BLOCK:
            win_right = f->kbd_win_short_right;
            *window_shape = KBD_WINDOW;
            break;
        case ONLY_SHORT_BLOCK:
            win_right = f->kbd_win_short_right;
            *window_shape = KBD_WINDOW;
            break;
        case LONG_STOP_BLOCK:
            win_right = f->sin_win_long_right;
            *window_shape = SINE_WINDOW;
            break;
    }

    switch (block_type) {
        case ONLY_LONG_BLOCK:
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) {
                f->mdct_long_buf[i]        = f->x_buf[i]        * win_left[i];
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            }
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
        case LONG_START_BLOCK:
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) 
                f->mdct_long_buf[i] = f->x_buf[i] * win_left[i];
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = 0;
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
        case ONLY_SHORT_BLOCK:
            offset = AAC_BLOCK_TRANS_LEN;
            for (k = 0; k < 8; k++) {
                for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++) {
                    f->mdct_short_buf[i]                     = f->x_buf[i+offset]                     * win_left[i];
                    f->mdct_short_buf[i+AAC_BLOCK_SHORT_LEN] = f->x_buf[i+offset+AAC_BLOCK_SHORT_LEN] * win_right[i];
                }
                offset += AAC_BLOCK_SHORT_LEN;
                fa_mdct(f->h_mdct_short, f->mdct_short_buf, mdct_line+k*AAC_BLOCK_SHORT_LEN);
            }
            break;
        case LONG_STOP_BLOCK:
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i] = 0;
            offset = AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_left[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
    }

}

#else 

/*used in encode, kbd is used for long block, sine is used for short block*/
void fa_aacfilterbank_analysis(uintptr_t handle, int block_type, int *window_shape, 
                               float *x, float *mdct_line)
{
    int i,k;
    int offset;
    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)handle;

    float *win_left, *win_right;

    /*update x_buf, 50% overlap, copy the remain half data to the beginning position*/
    for (i = 0; i < AAC_FRAME_LEN; i++) 
        f->x_buf[i] = f->x_buf[i+AAC_FRAME_LEN];
    for (i = 0; i < AAC_FRAME_LEN; i++)
        f->x_buf[i+AAC_FRAME_LEN] = x[i];

    /*window shape the input x*/
    switch (*window_shape) {
        case SINE_WINDOW:
            if (block_type == ONLY_LONG_BLOCK || block_type == LONG_START_BLOCK)
                win_left = f->sin_win_long_left;
            else 
                win_left = f->sin_win_short_left;
            break;
        case KBD_WINDOW:
            if (block_type == ONLY_LONG_BLOCK || block_type == LONG_START_BLOCK)
                win_left = f->kbd_win_long_left;
            else 
                win_left = f->kbd_win_short_left;
            break;
    }

    switch (block_type) {
        case ONLY_LONG_BLOCK:
            win_right = f->kbd_win_long_right;
            *window_shape = KBD_WINDOW;
            break;
        case LONG_START_BLOCK:
            win_right = f->sin_win_short_right;
            *window_shape = SINE_WINDOW;
            break;
        case ONLY_SHORT_BLOCK:
            win_right = f->sin_win_short_right;
            *window_shape = SINE_WINDOW;
            break;
        case LONG_STOP_BLOCK:
            win_right = f->kbd_win_long_right;
            *window_shape = KBD_WINDOW;
            break;
    }

    switch (block_type) {
        case ONLY_LONG_BLOCK:
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) {
                f->mdct_long_buf[i]        = f->x_buf[i]        * win_left[i];
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            }
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
        case LONG_START_BLOCK:
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) 
                f->mdct_long_buf[i] = f->x_buf[i] * win_left[i];
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = 0;
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
        case ONLY_SHORT_BLOCK:
            offset = AAC_BLOCK_TRANS_LEN;
            for (k = 0; k < 8; k++) {
                for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++) {
                    f->mdct_short_buf[i]                     = f->x_buf[i+offset]                     * win_left[i];
                    f->mdct_short_buf[i+AAC_BLOCK_SHORT_LEN] = f->x_buf[i+offset+AAC_BLOCK_SHORT_LEN] * win_right[i];
                }
                offset += AAC_BLOCK_SHORT_LEN;
                fa_mdct(f->h_mdct_short, f->mdct_short_buf, mdct_line+k*AAC_BLOCK_SHORT_LEN);
            }
            break;
        case LONG_STOP_BLOCK:
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i] = 0;
            offset = AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_left[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->mdct_long_buf[i+offset] = f->x_buf[i+offset] * win_right[i];
            fa_mdct(f->h_mdct_long, f->mdct_long_buf, mdct_line);
            break;
    }

}



#endif


/*used in decode*/
void fa_aacfilterbank_synthesis(uintptr_t handle, int block_type,
                                float *mdct_line, float *x)
{
    int i,k;
    int offset;
    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)handle;

    float *win_left, *win_right;

    switch (block_type) {
        case ONLY_LONG_BLOCK:
            win_left  = f->sin_win_long_left;
            win_right = f->sin_win_long_right;
            fa_imdct(f->h_mdct_long, mdct_line, f->mdct_long_buf);
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) {
                f->x_buf[i]        += f->mdct_long_buf[i] * win_left[i];
                f->x_buf[i+offset] =  f->mdct_long_buf[i+offset] * win_right[i];
            }

            for (i = 0; i < AAC_FRAME_LEN; i++)
                x[i] = f->x_buf[i];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) 
                f->x_buf[i] = f->x_buf[i+AAC_BLOCK_LONG_LEN];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i+AAC_BLOCK_LONG_LEN] = 0;

            break;
        case LONG_START_BLOCK:
            win_left  = f->sin_win_long_left;
            win_right = f->kbd_win_short_right;
            fa_imdct(f->h_mdct_long, mdct_line, f->mdct_long_buf);
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i] += f->mdct_long_buf[i] * win_left[i];
            offset = AAC_BLOCK_LONG_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->x_buf[i+offset] = f->mdct_long_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->x_buf[i+offset] = f->mdct_long_buf[i+offset] * win_right[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->x_buf[i+offset] = 0;

            for (i = 0; i < AAC_FRAME_LEN; i++)
                x[i] = f->x_buf[i];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i] = f->x_buf[i+AAC_BLOCK_LONG_LEN];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i+AAC_BLOCK_LONG_LEN] = 0;

            break;
        case ONLY_SHORT_BLOCK:
            win_left  = f->kbd_win_short_left;
            win_right = f->kbd_win_short_right;
            offset = AAC_BLOCK_TRANS_LEN;
            for (k = 0; k < 8; k++) {
                for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++) {
                    fa_imdct(f->h_mdct_short, mdct_line+k*AAC_BLOCK_SHORT_LEN, f->mdct_short_buf);
                    f->x_buf[i+offset] += f->mdct_short_buf[i] * win_left[i];
                    f->x_buf[i+offset+AAC_BLOCK_SHORT_LEN] = f->mdct_short_buf[i+AAC_BLOCK_SHORT_LEN] * win_right[i];
                }
                offset += AAC_BLOCK_SHORT_LEN;
            }
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->x_buf[i+offset+AAC_BLOCK_SHORT_LEN] = 0;

            for (i = 0; i < AAC_FRAME_LEN; i++)
                x[i] = f->x_buf[i];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++) 
                f->x_buf[i] = f->x_buf[i+AAC_BLOCK_LONG_LEN];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i+AAC_BLOCK_LONG_LEN] = 0;
            break;
        case LONG_STOP_BLOCK:
            win_left  = f->kbd_win_short_left;
            win_right = f->sin_win_long_right;
            offset = AAC_BLOCK_TRANS_LEN;
            fa_imdct(f->h_mdct_long, mdct_line, f->mdct_long_buf);
            for (i = 0; i < AAC_BLOCK_SHORT_LEN; i++)
                f->x_buf[i+offset] += f->mdct_long_buf[i+offset] * win_left[i];
            offset += AAC_BLOCK_SHORT_LEN;
            for (i = 0; i < AAC_BLOCK_TRANS_LEN; i++)
                f->x_buf[i+offset] = f->mdct_long_buf[i+offset];
            offset += AAC_BLOCK_TRANS_LEN;
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i+offset] = f->mdct_long_buf[i+offset] * win_right[i];

            for (i = 0; i < AAC_FRAME_LEN; i++)
                x[i] = f->x_buf[i];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i] = f->x_buf[i+AAC_BLOCK_LONG_LEN];
            for (i = 0; i < AAC_BLOCK_LONG_LEN; i++)
                f->x_buf[i+AAC_BLOCK_LONG_LEN] = 0;

            break;
    }

}


void fa_aacfilterbank_get_xbuf(uintptr_t handle, float *x)
{
    int i;
    fa_aacfilterbank_t *f = (fa_aacfilterbank_t *)handle;

    for (i = 0; i < 2*AAC_FRAME_LEN; i++)
        x[i] = f->x_buf[i];

}
