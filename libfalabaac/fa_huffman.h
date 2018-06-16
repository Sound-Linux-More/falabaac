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


  filename: fa_huffman.h 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#ifndef _FA_HUFFMAN_H
#define _FA_HUFFMAN_H 


#ifdef __cplusplus 
extern "C"
{ 
#endif  


#define INTENSITY_HCB 15
#define INTENSITY_HCB2 14

void fa_huffman_rom_init();

int fa_noiseless_huffman_bitcount(int *x_quant, int sfb_num,  int *sfb_offset,
                                  int *quant_hufftab_no, int *quant_bits);

int fa_huffman_encode_mdctline(int *x_quant, int sfb_num, int *sfb_offset, int *quant_hufftab_no,
                               int *max_sfb, int *x_quant_code, int *x_quant_bits);

#ifdef __cplusplus 
}
#endif  




#endif
