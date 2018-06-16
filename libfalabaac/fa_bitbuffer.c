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


  filename: fa_bitbuffer.c 
  version : v1.0.0
  time    : 2012/08/22 - 2012/10/05 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "fa_bitbuffer.h"


#define SIZE_OF_BYTE	    1
#define SIZE_OF_SHORT       2
#define SIZE_OF_LONG        4

#define BITS_OF_BYTE	    8


#ifndef FA_MAX
#define	FA_MAX(a,b)		((a) > (b) ? (a) : (b))
#endif 

#ifndef FA_MIN
#define	FA_MIN(a,b)		((a) < (b) ? (a) : (b))
#endif


void		fa_write_byte(unsigned char in , FILE *fp)
{
    unsigned char temp[SIZE_OF_BYTE];

    temp[0] = in & 0xff;
    fwrite(temp, sizeof(*temp), SIZE_OF_BYTE, fp);
}

void		fa_write_ushort(unsigned short in, FILE *fp)
{
    unsigned char temp[SIZE_OF_SHORT];

    temp[0] = in & 0xff;
    in >>= 8;
    temp[1] = in & 0xff;
    fwrite(temp,sizeof(*temp),SIZE_OF_SHORT,fp);
}

void		fa_write_ulong(unsigned long in, FILE *fp)
{
    unsigned char temp[SIZE_OF_LONG];

    temp[0] = in & 0xff;
    in >>= 8;
    temp[1] = in & 0xff;
    in >>= 8;
    temp[2] = in & 0xff;
    in >>= 8;
    temp[3] = in & 0xff;
    fwrite(temp,sizeof(*temp),SIZE_OF_LONG,fp);

}

unsigned char fa_read_byte(FILE *fp)
{
    unsigned char cx;
    unsigned char temp[SIZE_OF_BYTE];
    int ret;

    ret = fread(temp, sizeof(*temp), SIZE_OF_BYTE, fp);
    cx = temp[0];
    return cx;

}

unsigned short fa_read_ushort(FILE *fp)
{
    unsigned short cx;
    unsigned char temp[SIZE_OF_SHORT];
    int ret;

    ret = fread(temp, sizeof(*temp), SIZE_OF_SHORT, fp);
    cx = temp[0];
    cx |= (unsigned short)temp[1] << 8;
    return cx;

}

unsigned long fa_read_ulong(FILE *fp)
{
    unsigned long cx;
    unsigned char temp[SIZE_OF_LONG];
    int ret;

    ret = fread(temp,sizeof(*temp),SIZE_OF_LONG,fp);
    cx = temp[0];
    cx |= (unsigned long)temp[1] << 8;
    cx |= (unsigned long)temp[2] << 16;
    cx |= (unsigned long)temp[3] << 24;

    return cx;

}


void fa_bitbuffer_init(fa_bitbuffer_t *bitbuf, unsigned char *start, int sizeofbyte)
{
    assert(sizeofbyte*8 <= 32768);

    bitbuf->buf_size = sizeofbyte * 8;

    if (bitbuf)
        bitbuf->is_valid = 1;

    bitbuf->start =  start;		
    bitbuf->end = start + sizeofbyte - 1;		

    bitbuf->pNextRead = start;
    bitbuf->pNextWrite = start;

    bitbuf->rpos_of_byte = 7;			
    bitbuf->wpos_of_byte = 7;		

    bitbuf->nbits = 0;		  
}

void fa_bitbuffer_uninit(fa_bitbuffer_t * bitbuf)
{
    (bitbuf)->is_valid = 0;
    bitbuf = 0;
}

int  fa_bit2byte(int bit)
{
    return ((bit + 8 - 1)/8);
}

int  fa_getbits_num(fa_bitbuffer_t * bitbuf)
{
    return bitbuf->nbits;
}

int  fa_putbits(fa_bitbuffer_t * bitbuf, unsigned int wValue, int nbits)
{
    short bitsToWrite, bitsWritten;

    bitsWritten = nbits;                                                         

    bitbuf->nbits += nbits;

    while (nbits) {
        short          bits_to_shift;
        unsigned char  tmp, msk;

        bitsToWrite   = FA_MIN(bitbuf->wpos_of_byte + 1, nbits);
        bits_to_shift = bitbuf->wpos_of_byte+1 - bitsToWrite;

        tmp = (signed char)((unsigned int)wValue << (32-nbits) >> (32-bitsToWrite)<<bits_to_shift);
        msk = ~( ((1 << bitsToWrite) - 1) << bits_to_shift );    

        *bitbuf->pNextWrite &= msk;
        *bitbuf->pNextWrite |= tmp;

        bitbuf->wpos_of_byte = bitbuf->wpos_of_byte - bitsToWrite;
        nbits = nbits - bitsToWrite;

        if (bitbuf->wpos_of_byte<0) {
            bitbuf->wpos_of_byte = bitbuf->wpos_of_byte + 8;
            bitbuf->pNextWrite++;

            if (bitbuf->pNextWrite > bitbuf->end) {
                bitbuf->pNextWrite = bitbuf->start;
            }
        }
    }

    return bitsWritten;
}

int  fa_getbits(fa_bitbuffer_t * bitbuf, short noBitsToRead)
{
    int  returnValue;

    /* return value is of type Word32, it can hold up to 32 bits 
       this optimized code can read upto 25 Bits a time */
    assert(noBitsToRead <= 25);

    bitbuf->nbits        = bitbuf->nbits-noBitsToRead ;                              
    bitbuf->rpos_of_byte = bitbuf->rpos_of_byte-noBitsToRead ;                             

    /* 8-bit aligned read access: *pReadNext */
    returnValue = (unsigned int)*bitbuf->pNextRead;                                           

    while (bitbuf->rpos_of_byte < 0) {
        bitbuf->rpos_of_byte = (bitbuf->rpos_of_byte+ 8) ;                                
        bitbuf->pNextRead++;                                                               


        if (bitbuf->pNextRead > bitbuf->end) {
            bitbuf->pNextRead = bitbuf->start;                                        
        }

        returnValue <<= 8;                                                                  

        returnValue  |= (unsigned int)*bitbuf->pNextRead;                                       
    }

    returnValue = returnValue << ((31-noBitsToRead)- bitbuf->rpos_of_byte) >> (32-noBitsToRead);

    return returnValue;
}



/*below are the example for write and read*/
void  example_write_bitbuffer(FILE *f, int num_bytes, int sr_index)
{
    fa_bitbuffer_t bitbuf;
    unsigned char tmp[4];

    /*tmp: the buffer for the bitbuffer, here we need 4 bytes(32bits)*/
    memset(tmp,0,4);
    fa_bitbuffer_init(&bitbuf, tmp, 4);

    /*now write the bitbuffer vaule*/
    fa_putbits(&bitbuf, 0xfff    , 12);
    fa_putbits(&bitbuf, num_bytes, 13);
    fa_putbits(&bitbuf, sr_index , 4);
    fa_putbits(&bitbuf, 0        , 3);

    fa_bitbuffer_uninit(&bitbuf);

    fwrite(tmp,1,4,f);
}


void example_read_bitbuffer(FILE *f, unsigned int *num_bytes, int *sr_index)
{
    fa_bitbuffer_t bitbuf;
    unsigned char tmp[4];
    int ret;

    /*read the certain bytes into tmp buffer*/
    ret = fread(tmp,1,4,f);
    /*use the tmp buffer initial bitbuf*/
    fa_bitbuffer_init(&bitbuf, tmp, 4);

    /*now read value from the bitbuffer, bits order should obey with write*/
    fa_getbits(&bitbuf, 12);
    *num_bytes = fa_getbits(&bitbuf, 13);
    *sr_index  = fa_getbits(&bitbuf, 4);
    fa_getbits(&bitbuf, 3);

    fa_bitbuffer_uninit(&bitbuf);

}

        
