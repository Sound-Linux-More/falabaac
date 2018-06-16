/*
  falab - free algorithm lab 
  Copyright (C) 2012 luolongzhi ç½—é¾™æ™º (Chengdu, China)

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


  filename: main.c 
  version : v1.0.0
  time    : 2012/07/22 23:43
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/


/*
TODO:
   TASK                                STATUS                SUPPORT 
    ------------------------------------------------------------------
    mono                               done                   yes
    stereo(common_window=0)            done                   yes
    stereo(ms)                         done                   yes 
    lfe                                done                   yes(need more test) 
    high frequency optimize            done                   yes(bandwith limited now)
    bitrate control fixed              done                   yes(constant bitrate CBR is OK) 
    TNS                                done                   yes(not very important, little influence only for the strong hit audio point) 
    LTP                                no schecdule           no(very slow, no need support)
    add fast xmin/pe caculate method   done                 
    add new quantize fast method       done 
    optimize the speed performance     done 
     (maybe not use psy)
    bitrate cbr problem                doing 
    encode uinit                       done
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "fa_aacapi.h"
#include "fa_wavfmt.h"
#include "fa_parseopt.h"
#include "fa_timeprofile.h"

#define CHNMAX          64
#define FRAME_SIZE_MAX  (CHNMAX * 1024)//2048 
#define AAC_FRAME_SIZE  1024

int main(int argc, char *argv[])
{
    int ret;

    /*set your destination file and source file */
	FILE  * destfile;
	FILE  * sourcefile;

    /*file control variable*/
	fa_wavfmt_t fmt;
    int sample_rate;
    int chn_num;

    /*control the frame */
    int frame_index = 0;
    int is_last = 0;
    int read_len = 0;

    /*handle the aac encoder*/
    uintptr_t h_aacenc;

    /*sample buffer in and aac buffer out*/
	short wavsamples_in[FRAME_SIZE_MAX];
    unsigned char aac_buf[FRAME_SIZE_MAX];
    int aac_out_len;

    printf("************************************************************\n");
    printf("*                                                          *\n");
    printf("*              falabaac encoder v2.1.0.226                 *\n");
    printf("*                                                          *\n");
    printf("*   Copyright (C) 2012 luolongzhi ÂÞÁúÖÇ (Chengdu China)   *\n");
    printf("*                    Free Software                         *\n");
    printf("*                                                          *\n");
    printf("*             Email: luolongzhi@gmail.com                  *\n");
    printf("*                                                          *\n");
    printf("************************************************************\n\n");

    /*parse the argument*/
    ret = fa_parseopt(argc, argv);
    if(ret) return -1;

    /*open the dest and src file*/
    if ((destfile = fopen(opt_outputfile, "w+b")) == NULL) {
		printf("output file can not be opened\n");
		return 0; 
	}                         
	if ((sourcefile = fopen(opt_inputfile, "rb")) == NULL) {
		printf("input file can not be opened;\n");
		return 0; 
    }

    /*get the source wav file format such as sample rate , channel number...*/
    fmt = fa_wavfmt_readheader(sourcefile);
    printf("\n\nsamplerate = %lu\n", fmt.samplerate);
    sample_rate = fmt.samplerate;
    chn_num     = fmt.channels;

/*#define TEST_MEMLEAK */

#ifdef TEST_MEMLEAK
    while (1) {
        static int testcnt = 0;
#endif

    /*initial aac encoder, return the handle for the encoder*/
    h_aacenc = fa_aacenc_init(sample_rate, opt_bitrate, chn_num, opt_quality, opt_vbrflag,
                              FA_AACENC_MPEG_VER_DEF , FA_AACENC_OBJ_TYPE_DEF, opt_lfeenable,
                              opt_bandwidth,
                              opt_speedlevel, opt_time_resolution_first);
    if (!h_aacenc) {
        printf("initial failed, maybe configuration is not proper!\n");
        return -1;
    }

#ifdef TEST_MEMLEAK
    if (h_aacenc) 
        fa_aacenc_uninit(h_aacenc);

    #ifdef WIN32 
        Sleep(50);
    #else 
        usleep(50*1000);
    #endif
        testcnt++;
        printf("testcnt=%d\n", testcnt);

    }
#endif

    /*start time count*/
    FA_CLOCK_START(1);

    /*main loop of encode*/
    while(1)
    {
        if(is_last)
            break;

        /*read the raw sample from the wav file*/
        memset(wavsamples_in, 0, 2*AAC_FRAME_SIZE*chn_num);   //2 means the 2 bytes per real sample
        read_len = fread(wavsamples_in, 2, AAC_FRAME_SIZE*chn_num, sourcefile);
        if(read_len < (AAC_FRAME_SIZE*chn_num))
            is_last = 1;
       
        /*analysis and encode*/
        fa_aacenc_encode(h_aacenc, (unsigned char *)wavsamples_in, chn_num*2*read_len, aac_buf, &aac_out_len);

        /*write the aac ADTS stream into destfile*/
        fwrite(aac_buf, 1, aac_out_len, destfile);

        /*printf("the frame[%d]out length = %d\n", frame_index, aac_out_len);*/
        frame_index++;
        fprintf(stderr,"\rthe frame = [%d]", frame_index);
    }

    /*stop time count*/
    FA_CLOCK_END(1);
    FA_CLOCK_COST(1);

    /*close the files*/
    fclose(sourcefile);
    fclose(destfile);

    /*free the encode handle*/
    fa_aacenc_uninit(h_aacenc);

    printf("\n");

    /*printf the time cost*/
    FA_GET_TIME_COST(1);
    /*FA_GET_TIME_COST(2);*/
    /*FA_GET_TIME_COST(3);*/
    /*FA_GET_TIME_COST(4);*/
    /*FA_GET_TIME_COST(5);*/
    /*FA_GET_TIME_COST(6);*/

    return 0;
}
