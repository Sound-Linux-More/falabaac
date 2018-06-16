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


  filename: fa_getopt.c 
  version : v1.0.0
  time    : 2012/07/08 18:42 
  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
  code URL: http://code.google.com/p/falab/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__ 
#include <getopt.h>
#else
#include "getopt.h"
#endif

#include "fa_parseopt.h"

/*#define BW_MAXS  20*/
#define BW_MAXS  22.0

/*global option vaule*/
char  opt_inputfile[256]  = "";
char  opt_outputfile[256] = "";
int   opt_bitrate = 128;
float opt_quality = 0.7;
int   opt_vbrflag = 1;
int   opt_speedlevel = 1;
float opt_bandwidth = BW_MAXS;
int   opt_lfeenable = 0;
int   opt_time_resolution_first = 0;

int   opt_vbr_enable = 1;


const char *usage =
"\n\n"
"Usage: falabaac <-i> <inputfile> [options] ...\n"
"\n\n"
"See also:\n"
"    -h, --help               for help on ...\n"
/*"    --long-help          for a description of all options for ...\n"*/
"    -L, --license            for the license terms for falab.\n\n"
"Simple example:\n"
"       falabaac -i test.wav\n"
"       falabaac -i test.wav -o test_out.aac\n"
"       falabaac -i test.wav -q 1.0\n"
"       falabaac -i test.wav -b 128\n"
"       falabaac -i test.wav -b 128 -w 15\n"
"       falabaac -i test.wav -o test_out.aac -b 96\n"
"       falabaac -i test.wav -l 3\n"
"\n\n";

const char *default_set =
"\n\n"
"No argument input, run by default settings\n"
"    --bitrate    [128 kbps]\n"
"    --speedlevel [1]\n"
"    --bandwidth  [auto]\n"
"\n\n";

const char *short_help =
"\n\n"
"Usage: falabaac <-i> <inputfile> [options] ...\n"
"Options:\n"
"    -i <inputfile>             Set input filename                               [eg: test.wav]\n"
"    -o <outputfile>            Set output filename                              [eg: test_out.aac]\n"
"    -b <bitrate>               Set bitrate(kbps)                                [eg: -b 128]\n"
"    -q <quality>               Set quality(0.1~1.0)                             [eg: -q 0.5]\n"
"    -v <vbr>                   Set vbr mode(recommend)                          [eg: -v 1]\n"
"    -l <speedlevel>            Set speed level(1~6)                             [eg: -l 2]\n"
"    -w <bandwidth>             Set band width(kHz, 5~22kHz valid)               [eg: -w 15]\n"
"    -e <lfeenable>             Set the LFE encode enable(0 or 1)                [eg: -e 1]\n"
"    -t <time_resolution>       Set the encoder use time resolution first(0 or 1)[eg: -t 1]\n"
"    --help                     Show this abbreviated help.\n"
"    --long-help                Show complete help.\n"
"    --license                  For the license terms of falab.\n"
"\n\n";

const char *long_help =
"\n\n"
"Usage: falabaac <-i> <inputfile>  [options] ...\n"
"Options:\n"
"    -i <inputfile>             Set input filename                               [eg: test.wav]\n"
"    -o <outputfile>            Set output filename                              [eg: test_out.aac]\n"
"    -b <bitrate>               Set bitrate(kbps)                                [eg: -b 128]\n"
"    -q <quality>               Set quality(0.1~1.0)                             [eg: -q 0.7]\n"
"    -v <vbr>                   Set vbr mode(recommend)                          [eg: -v 1]\n"
"    -l <speedlevel>            Set speed level(1~6)                             [eg: -l 2]\n"
"    -w <bandwidth>             Set band width(kHz, 5~22kHz valid)               [eg: -w 15]\n"
"    -e <lfeenable>             Set the LFE encode enable(0 or 1)                [eg: -e 1]\n"
"    -t <time_resolution>       Set the encoder use time resolution first(0 or 1)[eg: -t 1]\n"
"    --help                     Show this abbreviated help.\n"
"    --long-help                Show complete help.\n"
"    --license                  for the license terms for falab.\n"
"    --input <inputfile>        Set input filename, must be wav file, now support 32kHz, 44.1kHz and 48kHz, 16bits/sample \n"
"    --output <outputfile>      Set output filename, aac file, format is MPEG2-ADTS\n"
"    --bitrate <bitrate>        Set average bitrate, 16~160kbps per channel, default is 128kbps. 96kbps is also good quality\n"
"    --quality <quality>        Set quality, 0.1~1.0\n"
"    --vbr <vbr>                Set vbr mode, strongly recommend, it has good quality and fast speed\n"
"    --speedlevel <l>           Set the speed level(1 is slow but good quality, 6 is fastest but less quality)\n"
"    --bandwidth  <w>           Set band width, only 5-20 (kHz) valid. 20kHz when bitrate >=96kbps\n"
"    --lfeenable  <e>           Set the LFE encode enable\n"
"    --time_resolution<t>       Set the encoding time resolution first, use short window\n"
"\n\n";


const char *all_help =
"\n\n"
"Usage: falabaac <-i> <inputfile>  [options] ...                                                            \n"
"                                                                                                           \n"
"Recommend command line:                                                                                    \n"
"       falabaac -i test.wav                                                                                \n"
"       falabaac -i test.wav -q 1                                                                           \n"
"                                                                                                           \n"
"Options:                                                                                                   \n"
"    -i <inputfile> , --input  <inputfile>                                                                  \n"          
"          Set input filename , must be wav file, now support 32kHz, 44.1kHz and 48kHz, 16bits/sample       \n"              
"          Example:  -i test.wav                                                                            \n" 
"                    --input test.wav                                                                       \n"
"                                                                                                           \n"
"    -o <outputfile> , --output <outputfile>                                                                \n"        
"          Set output filename , aac file, format is MPEG2-ADTS                                             \n"             
"          Example:  -o test_out.aac                                                                        \n"
"                    --output test_out.aac                                                                  \n"
"                                                                                                           \n"
"    -b <bitrate> , --bitrate <bitrate>                                                                     \n"
"          Set average bitrate, (16~160)kbps per channel                                                    \n"
"          Example:  -b 128                                                                                 \n"
"                    --bitrate 128                                                                          \n"
"                                                                                                           \n"
"    -q <quality> , --quality <quality>                                                                     \n"               
"          Set encoding quality level (0.1~1.0), default is 0.7, roughly bitrate is 110kbps for 44.1kHz     \n"
"          Example:  -q 0.7                                                                                 \n"
"                    --quality 0.7                                                                          \n"
"          WARN: when -b use, -q item is invalid                                                            \n"
"                                                                                                           \n"
"    -v <vbr> , --vbr <vbr>                                                                                 \n"                   
"          Set vbr mode (0,1), recommend use this mode, default is 1                                        \n"
"          Example:  -v 1                                                                                   \n"
"                    --vbr 1                                                                                \n"
"                                                                                                           \n"
"    -l <speedlevel> , --speedlevel <speedlevel>                                                            \n"           
"          Set speed level(1~6), default is 1, it has good quality                                          \n"
"          Want fast mode, -l 3 is a good choice                                                            \n"
"          Example:  -l 1                                                                                   \n"
"                    --speedlevel 1                                                                         \n"
"                                                                                                           \n"
"    -w <bandwidth> , --bandwidth <bandwidth>                                                               \n"
"          Set band width (5~22)kHz, float point value support, eg 16.5                                     \n"
"          Example:  -w 17                                                                                  \n"
"                    --bandwidth 17                                                                         \n"
"                                                                                                           \n"
"    -e <lfeenable> , --lfeenable <lfeenable>                                                               \n"            
"          Set the LFE encode enable(0 or 1)                                                                \n"
"          Example:  -e 1                                                                                   \n"
"                    --lfeenable 1                                                                          \n"
"                                                                                                           \n"
"    -t <time_resolution> , --time_resolution <time_resolution>                                             \n"      
"          Set the encoder time resolution mode (0 or 1), will use only short block when encodingh          \n"
"          Example:  -t 1                                                                                   \n"
"                    --time_resolution 1                                                                    \n"
"                                                                                                           \n"
"    --help                     Show this abbreviated help.                                                 \n"
"                                                                                                           \n"
"    --license                  for the license terms for falab.                                            \n"
"                                                                                                           \n"
"\n\n";




const char *license =
"\n\n"
"**************************************  WARN  *******************************************\n"
"*    Please note that the use of this software may require the payment of patent        *\n"
"*    royalties. You need to consider this issue before you start building derivative    *\n"
"*    works. We are not warranting or indemnifying you in any way for patent royalities! *\n"
"*                                                                                       *\n" 
"*                YOU ARE SOLELY RESPONSIBLE FOR YOUR OWN ACTIONS!                       *\n"
"*****************************************************************************************\n"
"\n"
"\n"
"falab - free algorithm lab \n"
"Copyright (C) 2012 luolongzhi (Chengdu, China)\n"
"\n"
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
"\n"
"\n"
"falab src code is based on the signal processing theroy, optimize theroy, ISO ref, etc.\n"
"The purpose of falab is building a free algorithm lab to help people to learn\n"
"and study algorithm, exchanging experience.    ---luolongzhi 2012.07.08\n"
"                                                                                    \n"
"code URL: http://code.google.com/p/falab/\n "
"\n";

static void fa_printopt()
{
    FA_PRINT("NOTE: configuration is below\n");
    FA_PRINT("NOTE: inputfile = %s\n", opt_inputfile);
    FA_PRINT("NOTE: outputfile= %s\n", opt_outputfile);
    FA_PRINT("NOTE: vbr switch= %d \n", opt_vbrflag);
    if (opt_vbrflag)
        FA_PRINT("NOTE: quality   = %f \n", opt_quality);
    else 
        FA_PRINT("NOTE: bitrate   = %d kbps\n", opt_bitrate);
    FA_PRINT("NOTE: speed lev = %d\n", opt_speedlevel);
    /*FA_PRINT("NOTE: timers    = %d\n", opt_time_resolution_first);*/
}

/**
 * @brief: check the input value valid, should check the valid scope
 *
 * @return: 0 if success, -1 if error
 */
static int fa_checkopt(int argc)
{

    if(argc < 3) {
        FA_PRINT_ERR("FAIL: input wav file should input, wav file should be 16bits per sample\n");
        return -1;
    }

    if (strlen(opt_outputfile) == 0) {
        char tmp[256];
        char *t = strrchr(opt_inputfile, '.');
        int l = t ? strlen(opt_inputfile) - strlen(t) : strlen(opt_inputfile);

        memset(tmp, 0, 256);
        memset(opt_outputfile, 0, 256);

        strncpy(tmp, opt_inputfile, l);
        sprintf(opt_outputfile, "%s.aac", tmp);
    }

    if (strlen(opt_inputfile) == 0 || strlen(opt_outputfile) == 0) {
        FA_PRINT_ERR("FAIL: input and output file should input\n");
        return -1;

    }
/*
    if(opt_bitrate > 256 || opt_bitrate < 32)  {
        FA_PRINT_ERR("FAIL: the bitrate is too large or too short, should be in [32000, 256000]\n");
        return -1;
    }
*/
    if (opt_speedlevel > 6 || opt_speedlevel < 1)  {
        FA_PRINT_ERR("FAIL: out of range, should be in [1,6]\n");
        return -1;
    }
 
    if (opt_bandwidth > (float)BW_MAXS || opt_bandwidth < 5.)  {
        /*FA_PRINT_ERR("FAIL: out of range, should be in [5,20] kHz\n");*/
        FA_PRINT_ERR("FAIL: out of range, should be in [5,22] kHz\n");
        return -1;
    }

    if (opt_quality > 1 || opt_quality < 0.1) {
        FA_PRINT_ERR("FAIL: out of range, should be in [0.1,1]\n");
        return -1;
    }

    FA_PRINT("SUCC: check option ok\n");
    return 0;
}


/**
 * @brief: parse the command line
 *         this is the simple template which will be used by falab projects
 *
 * @param:argc
 * @param:argv[]
 *
 * @return: 0 if success, -1 if error(input value maybe not right)
 */
int fa_parseopt(int argc, char *argv[])
{
    int ret;
    const char *die_msg = NULL;

    while (1) {
        static char * const     short_options = "hLi:o:b:q:v:l:w:e:t:";  
        static struct option    long_options[] = 
                                {
                                    { "help"       , 0, 0, 'h'}, 
                                    { "license"    , 0, 0, 'L'},
                                    { "input"      , 1, 0, 'i'},                 
                                    { "output"     , 1, 0, 'o'},                 
                                    { "bitrate"    , 1, 0, 'b'},        
                                    { "quality"    , 1, 0, 'q'},        
                                    { "vbr"        , 1, 0, 'v'},        
                                    { "speedlevel" , 1, 0, 'l'},        
                                    { "bandwidth"  , 1, 0, 'w'},        
                                    { "lfeenable"  , 1, 0, 'e'},        
                                    { "time_resolution"  , 1, 0, 't'},        
                                    {0             , 0, 0,  0},
                                };
        int c = -1;
        int option_index = 0;

        c = getopt_long(argc, argv, 
                        short_options, long_options, &option_index);

        if (c == -1) {
            break;
        }

        if (!c) {
            die_msg = usage;
            break;
        }

        switch (c) {
            case 'h': {
                          /*die_msg = short_help;*/
                          die_msg = all_help;
                          break;
                      }
/*
            case 'H': {
                          die_msg = long_help;
                          break;
                      }
                      */
            case 'L': {
                          die_msg = license;
                          break;
                      }

            case 'i': {
                          if (sscanf(optarg, "%s", opt_inputfile) > 0) {
                              FA_PRINT("SUCC: inputfile is %s\n", opt_inputfile);
                          }else {
                              FA_PRINT_ERR("FAIL: no inputfile\n");
                          }
                          break;
                      }

            case 'o': {
                          if (sscanf(optarg, "%s", opt_outputfile) > 0) {
                              FA_PRINT("SUCC: outputfile is %s\n", opt_outputfile);
                          }else {
                              FA_PRINT_ERR("FAIL: no outputfile\n");
                          }
                          break;
                      }

            case 'b': {
                          unsigned int i;
                          if (sscanf(optarg, "%u", &i) > 0) {
                              opt_bitrate = i;
                              opt_vbr_enable = 0;
                              FA_PRINT("SUCC: set bitrate = %u\n", opt_bitrate);

                          }
                          break;
                      }

            case 'q': {
                          float i;
                          if (sscanf(optarg, "%f", &i) > 0) {
                              opt_quality = i;
                              FA_PRINT("SUCC: set quality = %f\n", opt_quality);
                          }
                          break;
                      }

            case 'v': {
                          unsigned int i;
                          if (sscanf(optarg, "%u", &i) > 0) {
                              opt_vbrflag = i;
                              FA_PRINT("SUCC: set vbr mode = %u\n", opt_vbrflag);
                          }
                          break;
                      }

            case 'l': {
                          unsigned int i;
                          if (sscanf(optarg, "%u", &i) > 0) {
                              opt_speedlevel = i;
                              FA_PRINT("SUCC: set speedlevel = %u\n", opt_speedlevel);
                          }
                          break;
                      }

            case 'w': {
                          /*unsigned int i;*/
                          float i;
                          if (sscanf(optarg, "%f", &i) > 0) {
                              opt_bandwidth = i;
                              FA_PRINT("SUCC: set band_width = %f\n", opt_bandwidth);
                          }
                          break;
                      }

            case 'e': {
                          unsigned int i;
                          if (sscanf(optarg, "%u", &i) > 0) {
                              opt_lfeenable = i;
                              if (opt_lfeenable != 0 && opt_lfeenable != 1) {
                                  FA_PRINT("FAIL: lfe enable should be 0 or 1\n");
                                  exit(0);
                              }
                              FA_PRINT("SUCC: set lfe enable = %u\n", opt_lfeenable);
                          }
                          break;
                      }

            case 't': {
                          unsigned int i;
                          if (sscanf(optarg, "%u", &i) > 0) {
                              opt_time_resolution_first = i;
                              if (opt_time_resolution_first != 0 && opt_time_resolution_first!= 1) {
                                  FA_PRINT("FAIL: time_resolution enable should be 0 or 1\n");
                                  exit(0);
                              }
                              FA_PRINT("SUCC: set time_resolution enable = %u\n", opt_time_resolution_first);
                          }
                          break;
                      }


            case '?':
            default:
                      die_msg = usage;
                      break;
        }
    }

    if(die_msg) {
        FA_PRINT("%s\n", die_msg);
        goto fail;
    }

    /*check the input validity*/
    ret = fa_checkopt(argc);
    if(ret) {
        die_msg = usage;
        FA_PRINT("%s\n", die_msg);
        goto fail;
    }

    if (opt_vbr_enable == 0)
        opt_vbrflag = 0;
    /*print the settings*/
    fa_printopt();

    return 0;
fail:
    return -1;
}
