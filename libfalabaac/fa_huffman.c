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


  filename: fa_huffman.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fa_huffmantab.h"
#include "fa_huffman.h"
#include "fa_bitbuffer.h"

#ifndef FA_ABS
#define FA_ABS(A)    ((A) < 0 ? (-(A)) : (A))
#endif

#ifndef FA_MIN
#define FA_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef FA_MAX
#define FA_MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif


static float rom_exp2_N[20];

void fa_huffman_rom_init()
{
    int i;

    for (i = 0; i < 20; i++)
        rom_exp2_N[i] = pow(2, i-1);
}
/*
   This function takes an element that is larger than 16 and generates the base10 value of the
   equivalent escape sequence.  It returns the escape sequence in the variable, 'output'.  It
   also passed the length of the escape sequence through the parameter, 'len_esc_sequence'.
*/
static int calculate_esc_sequence(int input, int *len_esc_sequence)
{
#if 0//1 
    float x, y;
    int   output;
    int   N;

    N = -1;
    y = (float)FA_ABS(input);
    x = y / 16;

    while (x >= 1) {
        N++;
        x = x/2;
    }

    *len_esc_sequence = 2*N + 5;  /* the length of the escape sequence in bits */

    output = (int)((pow(2,N) - 1)*pow(2,N+5) + y - pow(2,N+4));

#else 
    int   x, y;
    int   output;
    int   N;

    N = -1;
    y = FA_ABS(input);
    x = y >> 4;

    while (x >= 1) {
        N++;
        x = x >> 1;
    }

    *len_esc_sequence = 2*N + 5;  /* the length of the escape sequence in bits */

    /*output = (int)((pow(2,N) - 1)*pow(2,N+5) + y - pow(2,N+4));*/
    output = (int)((rom_exp2_N[N+1] - 1)*rom_exp2_N[N+6] + y - rom_exp2_N[N+5]);


#endif

    return(output);
}


static int calculate_huff_bits(int hufftab_no, int *x_quant, int offset, int length)
{
    int len_esc;
    int index;
    int bits = 0;
    int i, j;

    switch(hufftab_no) {
        case 1: //sign table
            for (i = offset; i < offset+length; i += 4) {
                index =  27*x_quant[i] + 9*x_quant[i+1] + 3*x_quant[i+2] + x_quant[i+3] + 40;
                bits  += fa_hufftab1[index][0];
            }
            return bits;
        case 2: //sign table
            for (i = offset; i < offset+length; i += 4) {
                index =  27*x_quant[i] + 9*x_quant[i+1] + 3*x_quant[i+2] + x_quant[i+3] + 40;
                bits  += fa_hufftab2[index][0];
            }
            return bits;
        case 3: //unsigned table
            for (i = offset; i < offset+length; i += 4){
                index =  27*FA_ABS(x_quant[i]) + 9*FA_ABS(x_quant[i+1]) + 3*FA_ABS(x_quant[i+2]) + FA_ABS(x_quant[i+3]);
                bits  += fa_hufftab3[index][0];
                for (j = 0; j < 4; j++) {
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 4: //unsigned table
            for (i = offset; i < offset+length; i += 4){
                index =  27*FA_ABS(x_quant[i]) + 9*FA_ABS(x_quant[i+1]) + 3*FA_ABS(x_quant[i+2]) + FA_ABS(x_quant[i+3]);
                bits  += fa_hufftab4[index][0];
                for (j = 0; j < 4; j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 5: //sign table
            for (i = offset; i < offset+length; i += 2){
                index =  9*(x_quant[i]) + (x_quant[i+1]) + 40;
                bits  += fa_hufftab5[index][0];
            }
            return bits;
        case 6: //sign table
            for (i = offset; i < offset+length; i += 2){
                index =  9*(x_quant[i]) + (x_quant[i+1]) + 40;
                bits  += fa_hufftab6[index][0];
            }
            return bits;
        case 7: //unsigned table
            for (i = offset; i < offset+length; i += 2){
                index =  8*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                bits  += fa_hufftab7[index][0];
                for (j = 0; j < 2; j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 8: //unsigned table
            for (i = offset; i < offset+length; i += 2){
                index =  8*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                bits  += fa_hufftab8[index][0];
                for (j = 0; j < 2; j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 9: //unsigned table
            for (i = offset; i < offset+length; i += 2){
                index =  13*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                bits  += fa_hufftab9[index][0];
                for (j = 0; j < 2; j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 10: //unsigned table
            for (i = offset; i < offset+length; i += 2){
                index =  13*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                bits  += fa_hufftab10[index][0];
                for (j = 0; j < 2; j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }
            }
            return bits;
        case 11: //unsigned table
            /* First, calculate the indecies into the huffman tables */
            for (i = offset; i < offset+length; i += 2){
                if ((FA_ABS(x_quant[i]) >= 16) && (FA_ABS(x_quant[i+1]) >= 16)) {  /* both codewords were above 16 */
                    /* first, code the orignal pair, with the larger value saturated to +/- 16 */
                    index = 17*16 + 16;
                } else if (FA_ABS(x_quant[i]) >= 16) {  /* the first codeword was above 16, not the second one */
                    /* first, code the orignal pair, with the larger value saturated to +/- 16 */
                    index = 17*16 + FA_ABS(x_quant[i+1]);
                } else if (FA_ABS(x_quant[i+1]) >= 16) { /* the second codeword was above 16, not the first one */
                    index = 17*FA_ABS(x_quant[i]) + 16;
                } else {  /* there were no values above 16, so no escape sequences */
                    index = 17*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                }

                /* write out the codewords */
                bits += fa_hufftab11[index][0];

                /* Take care of the sign bits */
                for (j=0;j<2;j++){
                    if (x_quant[i+j] != 0) 
                        bits += 1; /* only for non-zero spectral coefficients */
                }

                /* write out the escape sequences */
                if ((FA_ABS(x_quant[i]) >= 16) && (FA_ABS(x_quant[i+1]) >= 16)) {  /* both codewords were above 16 */
                    /* code and transmit the first escape_sequence */
                    calculate_esc_sequence(x_quant[i],&len_esc);
                    bits += len_esc;

                    /* then code and transmit the second escape_sequence */
                    calculate_esc_sequence(x_quant[i+1],&len_esc);
                    bits += len_esc;
                } else if (FA_ABS(x_quant[i]) >= 16) {  /* the first codeword was above 16, not the second one */
                    /* code and transmit the escape_sequence */
                    calculate_esc_sequence(x_quant[i],&len_esc);
                    bits += len_esc;
                } else if (FA_ABS(x_quant[i+1]) >= 16) { /* the second codeword was above 16, not the first one */
                    /* code and transmit the escape_sequence */
                    calculate_esc_sequence(x_quant[i+1],&len_esc);
                    bits += len_esc;
                }
            }
            return bits;
    }

    return 0;
}


int fa_noiseless_huffman_bitcount(int *x_quant, int sfb_num,  int *sfb_offset,
                                  int *quant_hufftab_no, int *quant_bits)
{
    int i, j;
    int max_sfb_quant;
    int tmp_quant;
    int quant_hufftab_no1, quant_bits1;
    int quant_hufftab_no2, quant_bits2;
    int quant_hufftab_best, quant_bits_best;
    int sfb_width;
    int total_bits;

    total_bits = 0;

    for (i = 0; i < sfb_num; i++) {
        sfb_width = sfb_offset[i+1] - sfb_offset[i];
        /*first get the max value of x_quant, and use this to decide which huffman tab will be use*/
        max_sfb_quant = 0;
        for (j = sfb_offset[i]; j < sfb_offset[i+1]; j++) {
            tmp_quant = FA_ABS(x_quant[j]);
            if (tmp_quant > max_sfb_quant) 
                max_sfb_quant = tmp_quant; 
        }

        /*using max_sfb_quant to decide which huffman table will be used*/
        if (max_sfb_quant == 0) {
            quant_bits_best    = calculate_huff_bits(0, x_quant, sfb_offset[i], sfb_width);
            quant_hufftab_best = 0;
        }else if (max_sfb_quant < 2) {
            quant_hufftab_no1 = 1;
            quant_hufftab_no2 = 2;
        }else if (max_sfb_quant < 3) {
            quant_hufftab_no1 = 3;
            quant_hufftab_no2 = 4;
        }else if (max_sfb_quant < 5) {
            quant_hufftab_no1 = 5;
            quant_hufftab_no2 = 6;
        }else if (max_sfb_quant < 8) {
            quant_hufftab_no1 = 7;
            quant_hufftab_no2 = 8;
        }else if (max_sfb_quant < 13) {
            quant_hufftab_no1 = 9;
            quant_hufftab_no2 = 10;
        }else {
            quant_bits_best    = calculate_huff_bits(11, x_quant, sfb_offset[i], sfb_width);
            quant_hufftab_best = 11;
        }

        if (max_sfb_quant > 0 && max_sfb_quant < 13) {
            quant_bits1 = calculate_huff_bits(quant_hufftab_no1, x_quant, sfb_offset[i], sfb_width);
            quant_bits2 = calculate_huff_bits(quant_hufftab_no2, x_quant, sfb_offset[i], sfb_width);
            quant_bits_best = FA_MIN(quant_bits1, quant_bits2);
            if (quant_bits_best == quant_bits1)
                quant_hufftab_best = quant_hufftab_no1;
            else 
                quant_hufftab_best = quant_hufftab_no2;
        }

        quant_hufftab_no[i] = quant_hufftab_best;
        quant_bits[i]       = quant_bits_best;

        total_bits += quant_bits_best;
    }


    return total_bits;
}



int fa_huffman_encode_mdctline(int *x_quant, int sfb_num, int *sfb_offset, int *quant_hufftab_no,
                               int *max_sfb, int *x_quant_code, int *x_quant_bits)
{
    int sfb;
    int sequence_esc;
    int len_esc;
    int index;
    int i, j;

    int offset, length;
    int counter;

    int bits;

    int hufftab_no;
    /*int hufftab_no_zero_cnt;*/

    counter = 0;
    bits    = 0;
    /*hufftab_no_zero_cnt = 0;*/

    for (sfb = 0; sfb < sfb_num; sfb++) {
        offset     = sfb_offset[sfb];
        length     = sfb_offset[sfb+1] - sfb_offset[sfb];
        hufftab_no = quant_hufftab_no[sfb];

#if 0
        if (hufftab_no) {
            *max_sfb = sfb+1;
            hufftab_no_zero_cnt = 0;
        } else {
            hufftab_no_zero_cnt++;
        }
#else 
        if (hufftab_no) {
            *max_sfb = sfb+1;
        }

#endif

        switch(hufftab_no) {
            case 0:
            case INTENSITY_HCB:
            case INTENSITY_HCB2:
                x_quant_bits[counter]   = 0;
                x_quant_code[counter++] = 0;
                break;

            case 1:
                for (i = offset; i < offset+length; i += 4) {
                    index =  27*x_quant[i] + 9*x_quant[i+1] + 3*x_quant[i+2] + x_quant[i+3] + 40;
                    bits  += fa_hufftab1[index][0];
                    x_quant_bits[counter]   = fa_hufftab1[index][0];
                    x_quant_code[counter++] = fa_hufftab1[index][1];
                }
                break;
            case 2:
                for (i = offset; i < offset+length; i += 4) {
                    index =  27*x_quant[i] + 9*x_quant[i+1] + 3*x_quant[i+2] + x_quant[i+3] + 40;
                    bits  += fa_hufftab2[index][0];
                    x_quant_bits[counter]   = fa_hufftab2[index][0];
                    x_quant_code[counter++] = fa_hufftab2[index][1];
                }
                break;
            case 3:
                for (i = offset; i < offset+length; i += 4){
                    index =  27*FA_ABS(x_quant[i]) + 9*FA_ABS(x_quant[i+1]) + 3*FA_ABS(x_quant[i+2]) + FA_ABS(x_quant[i+3]);
                    bits  += fa_hufftab3[index][0];
                    x_quant_bits[counter]   = fa_hufftab3[index][0];
                    x_quant_code[counter++] = fa_hufftab3[index][1];
                    for (j = 0; j < 4; j++) {
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 4:
                for (i = offset; i < offset+length; i += 4){
                    index =  27*FA_ABS(x_quant[i]) + 9*FA_ABS(x_quant[i+1]) + 3*FA_ABS(x_quant[i+2]) + FA_ABS(x_quant[i+3]);
                    bits  += fa_hufftab4[index][0];
                    x_quant_bits[counter]   = fa_hufftab4[index][0];
                    x_quant_code[counter++] = fa_hufftab4[index][1];
                    for (j = 0; j < 4; j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 5:
                for (i = offset; i < offset+length; i += 2){
                    index =  9*(x_quant[i]) + (x_quant[i+1]) + 40;
                    bits  += fa_hufftab5[index][0];
                    x_quant_bits[counter]   = fa_hufftab5[index][0];
                    x_quant_code[counter++] = fa_hufftab5[index][1];
                }
                break;
            case 6:
                for (i = offset; i < offset+length; i += 2){
                    index =  9*(x_quant[i]) + (x_quant[i+1]) + 40;
                    bits  += fa_hufftab6[index][0];
                    x_quant_bits[counter]   = fa_hufftab6[index][0];
                    x_quant_code[counter++] = fa_hufftab6[index][1];
                }
                break;
            case 7:
                for (i = offset; i < offset+length; i += 2){
                    index =  8*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                    bits  += fa_hufftab7[index][0];
                    x_quant_bits[counter]   = fa_hufftab7[index][0];
                    x_quant_code[counter++] = fa_hufftab7[index][1];
                    for (j = 0; j < 2; j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 8:
                for (i = offset; i < offset+length; i += 2){
                    index =  8*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                    bits  += fa_hufftab8[index][0];
                    x_quant_bits[counter]   = fa_hufftab8[index][0];
                    x_quant_code[counter++] = fa_hufftab8[index][1];
                    for (j = 0; j < 2; j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 9:
                for (i = offset; i < offset+length; i += 2){
                    index =  13*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                    bits  += fa_hufftab9[index][0];
                    x_quant_bits[counter]   = fa_hufftab9[index][0];
                    x_quant_code[counter++] = fa_hufftab9[index][1];
                    for (j = 0; j < 2; j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 10:
                for (i = offset; i < offset+length; i += 2){
                    index =  13*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                    bits  += fa_hufftab10[index][0];
                    x_quant_bits[counter]   = fa_hufftab10[index][0];
                    x_quant_code[counter++] = fa_hufftab10[index][1];
                    for (j = 0; j < 2; j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }
                }
                break;
            case 11:
                /* First, calculate the indecies into the huffman tables */
                for (i = offset; i < offset+length; i += 2){
                    if ((FA_ABS(x_quant[i]) >= 16) && (FA_ABS(x_quant[i+1]) >= 16)) {  /* both codewords were above 16 */
                        /* first, code the orignal pair, with the larger value saturated to +/- 16 */
                        index = 17*16 + 16;
                    } else if (FA_ABS(x_quant[i]) >= 16) {  /* the first codeword was above 16, not the second one */
                        /* first, code the orignal pair, with the larger value saturated to +/- 16 */
                        index = 17*16 + FA_ABS(x_quant[i+1]);
                    } else if (FA_ABS(x_quant[i+1]) >= 16) { /* the second codeword was above 16, not the first one */
                        index = 17*FA_ABS(x_quant[i]) + 16;
                    } else {  /* there were no values above 16, so no escape sequences */
                        index = 17*FA_ABS(x_quant[i]) + FA_ABS(x_quant[i+1]);
                    }

                    /* write out the codewords */
                    x_quant_bits[counter]   = fa_hufftab11[index][0];
                    x_quant_code[counter++] = fa_hufftab11[index][1];
                    bits += fa_hufftab11[index][0];

                    /* Take care of the sign bits */
                    for (j=0;j<2;j++){
                        if (x_quant[i+j] > 0) {
                            x_quant_code[counter]   = 0;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }else if (x_quant[i+j] < 0) {
                            x_quant_code[counter]   = 1;
                            x_quant_bits[counter++] = 1;
                            bits += 1; /* only for non-zero spectral coefficients */
                        }
                    }

                    /* write out the escape sequences */
                    if ((FA_ABS(x_quant[i]) >= 16) && (FA_ABS(x_quant[i+1]) >= 16)) {  /* both codewords were above 16 */
                        /* code and transmit the first escape_sequence */
                        sequence_esc = calculate_esc_sequence(x_quant[i],&len_esc);
                        x_quant_code[counter]   = sequence_esc;
                        x_quant_bits[counter++] = len_esc;
                        bits += len_esc;

                        /* then code and transmit the second escape_sequence */
                        sequence_esc = calculate_esc_sequence(x_quant[i+1],&len_esc);
                        x_quant_code[counter]   = sequence_esc;
                        x_quant_bits[counter++] = len_esc;
                        bits += len_esc;
                    } else if (FA_ABS(x_quant[i]) >= 16) {  /* the first codeword was above 16, not the second one */
                        /* code and transmit the escape_sequence */
                        sequence_esc = calculate_esc_sequence(x_quant[i],&len_esc);
                        x_quant_code[counter]   = sequence_esc;
                        x_quant_bits[counter++] = len_esc;
                        bits += len_esc;
                    } else if (FA_ABS(x_quant[i+1]) >= 16) { /* the second codeword was above 16, not the first one */
                        /* code and transmit the escape_sequence */
                        sequence_esc = calculate_esc_sequence(x_quant[i+1],&len_esc);
                        x_quant_code[counter]   = sequence_esc;
                        x_quant_bits[counter++] = len_esc;
                        bits += len_esc;
                    }
                }
        }
         
    }
#if 0
    if (hufftab_no_zero_cnt)
        counter = counter - hufftab_no_zero_cnt + 1 ;
#endif

/*
    I found that the faad can not support max_sfb=0, if max_sfb==0, the decoder will inform you some wrong thing happen,
    but it can also decode the remain ok frame,
    so if max_sfb=0, I increase it to 1
*/
    if (0 == *max_sfb)
        *max_sfb = 1;

    /*return bits;*/
    return counter;
}




